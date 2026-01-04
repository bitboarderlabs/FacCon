//// -- webserver.c --
//#include "webserver.h"
//#include "lwip/apps/httpd.h"
//#include "lwip/apps/fs.h"
//#include "io_core.h"
//#include "leds.h"
//#include <string.h>
//#include <stdio.h>
//
//static char page_buffer[1024];
//
//static const char html_template[] =
//"<html><head><title>Field IO Unit</title>"
//"<style>"
//".led { width: 30px; height: 30px; border-radius: 50%; display: inline-block; margin: 5px; cursor: pointer; }"
//".green { background-color: lime; }"
//".gray { background-color: gray; }"
//".red { background-color: red; }"
//".darkred { background-color: #440000; }"
//"</style>"
//"<script>"
//"function toggleOut(num) {"
//"  fetch('/toggle?num=' + num);"
//"}"
//"</script>"
//"</head><body>"
//"<h1>Field IO Unit - Snapshot</h1>"
//"<p>Page loaded at server time: %lu ms</p>"
//"<h2>Analog Inputs</h2>"
//"<p>A0: %u</p>"
//"<p>A1: %u</p>"
//"<p>A2: %u</p>"
//"<h2>Digital Input</h2>"
//"<div class='led %s'></div> DIN0 (%s)<br>"
//"<h2>Digital Output</h2>"
//"<div class='led %s' onclick='toggleOut(0)'></div> DOUT0 (%s) - Click to toggle<br>"
//"<p><a href='/'>Refresh</a> to see current values</p>"
//"<hr>"
//"<p>Modbus TCP working. I/O core working. Dynamic web snapshot with output toggle achieved.</p>"
//"</body></html>";
//
//// CGI handler for toggle
//const char * toggle_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
//{
//    for (int i = 0; i < iNumParams; i++) {
//        if (strcmp(pcParam[i], "num") == 0) {
//            uint8_t num = atoi(pcValue[i]);
//            if (num < DOUT_COUNT) {
//                bool current = IO_GetOutput(num);
//                IO_SetOutput(num, !current);
//            }
//        }
//    }
//    return "/";  // Redirect back
//}
//
//static tCGI toggle_cgi = {"/toggle", toggle_cgi_handler};
//
//static tCGI cgi_handlers[] = {
//    {"/toggle", toggle_cgi_handler}
//};
//
//void cgi_init(void)
//{
//    http_set_cgi_handlers(cgi_handlers, 1);
//}
//
//int fs_open_custom(struct fs_file *file, const char *name)
//{
//    if (strcmp(name, "/") == 0 || strcmp(name, "/index.html") == 0) {
//        uint16_t a0 = IO_GetAnalog(0);
//        uint16_t a1 = IO_GetAnalog(1);
//        uint16_t a2 = IO_GetAnalog(2);
//        bool din = IO_GetInput(0);
//        bool dout = IO_GetOutput(0);
//
//        const char *din_class = din ? "green" : "gray";
//        const char *din_text = din ? "On" : "Off";
//        const char *dout_class = dout ? "red" : "darkred";
//        const char *dout_text = dout ? "On" : "Off";
//
//        int len = snprintf(page_buffer, sizeof(page_buffer), html_template,
//                           HAL_GetTick(),
//                           a0, a1, a2,
//                           din_class, din_text,
//                           dout_class, dout_text);
//
//        file->data = page_buffer;
//        file->len = len;
//        file->index = len;
//        file->flags = FS_FILE_FLAGS_HEADER_INCLUDED;
//        return 1;
//    }
//
//    return 0;
//}
//
//void fs_close_custom(struct fs_file *file)
//{
//    // Nothing
//}
//
//void WebServer_Init(void)
//{
//    httpd_init();
//    cgi_init();
//}


#include "webserver.h"
#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "io_core.h"
#include <string.h>
#include <stdio.h>
#include <sysstatus.h>

static char page_buffer[1024];
static char json_buffer[256];

// Main page with polling JS and toggle
static const char main_html[] =
"<html><head><title>Field IO Unit</title>"
"<script>"
"function updateStatus() {"
"  fetch('/status.json')"
"  .then(response => response.json())"
"  .then(data => {"
"    document.getElementById('ain0').innerText = data.ain0;"
"    document.getElementById('ain1').innerText = data.ain1;"
"    document.getElementById('ain2').innerText = data.ain2;"
"    document.getElementById('din0').className = 'led ' + (data.din0 ? 'green' : 'gray');"
"    document.getElementById('dout0').className = 'led ' + (data.dout0 ? 'red' : 'darkred');"
"  });"
"}"
"setInterval(updateStatus, 500);  // 100 ms polling = <100 ms latency"
"updateStatus();"
"function toggleOut(num) {"
"  fetch('/toggle?num=' + num);"
"  updateStatus();  // Immediate refresh after toggle"
"}"
"</script>"
"<style>"
".led { width: 30px; height: 30px; border-radius: 50%; display: inline-block; margin: 5px; cursor: pointer; }"
".green { background-color: lime; }"
".gray { background-color: gray; }"
".red { background-color: red; }"
".darkred { background-color: #440000; }"
"</style>"
"</head><body>"
"<h1>Field IO Unit</h1>"
"<h2>Analog Inputs</h2>"
"<p>A0: <span id='ain0'>--</span></p>"
"<p>A1: <span id='ain1'>--</span></p>"
"<p>A2: <span id='ain2'>--</span></p>"
"<h2>Digital Input</h2>"
"<div class='led gray' id='din0'></div> DIN0<br>"
"<h2>Digital Output</h2>"
"<div class='led darkred' id='dout0' onclick='toggleOut(0)'></div> DOUT0 (click to toggle)<br>"
"<h2>App Sequencer (placeholder)</h2>"
"<button>Start</button> <button>Stop</button> <button>Pause</button> <button>Single Step</button><br>"
"<textarea rows='10' cols='80'>// Script will be editable here later</textarea>"
"</body></html>";

// Toggle CGI
const char * toggle_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "num") == 0) {
            uint8_t num = atoi(pcValue[i]);
            if (num < DOUT_COUNT) {
                bool current = IO_GetOutput(num);
                IO_SetOutput(num, !current);
            }
        }
    }
    return "/index.html";  // Redirect back
}

static tCGI toggle_cgi = {"/toggle", toggle_cgi_handler};
static tCGI cgi_handlers[] = {
    {"/toggle", toggle_cgi_handler}
};

void cgi_init(void)
{
    http_set_cgi_handlers(cgi_handlers, 1);
}

int fs_open_custom(struct fs_file *file, const char *name)
{
    if (strcmp(name, "/") == 0 || strcmp(name, "/index.html") == 0) {
        file->data = main_html;
        file->len = strlen(main_html);
        file->index = file->len;
        file->flags = FS_FILE_FLAGS_HEADER_INCLUDED;
        return 1;
    }

    if (strcmp(name, "/status.json") == 0) {
        int len = snprintf(json_buffer, sizeof(json_buffer),
            "{\"ain0\":%u,\"ain1\":%u,\"ain2\":%u,\"din0\":%s,\"dout0\":%s}",
            IO_GetAnalogIn(0),
            IO_GetAnalogIn(1),
            IO_GetAnalogIn(2),
            IO_GetInput(0) ? "true" : "false",
            IO_GetOutput(0) ? "true" : "false");

        file->data = json_buffer;
        file->len = len;
        file->index = len;
        file->flags = FS_FILE_FLAGS_HEADER_INCLUDED;
        return 1;
    }

    return 0;
}

void fs_close_custom(struct fs_file *file)
{
    // Nothing
}

void WebServer_Init(void)
{
    httpd_init();
    cgi_init();
}
