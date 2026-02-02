//-- webserver.c --//
#include "webserver.h"
#include "lwip/apps/httpd.h"
#include "lwip/apps/fs.h"
#include "io_core.h"  // Assumes your IO functions: IO_GetAnalogIn(uint8_t), IO_GetInput(uint8_t), IO_GetOutput(uint8_t), IO_SetOutput(uint8_t, bool)
#include <string.h>
#include <stdio.h>
#include <stdlib.h>  // For atoi

static char json_buffer[512];  // Increased size for more IO data

// Main HTML page with JS for polling and toggles
static const char main_html[] =
"<html><head><title>Remote IO Controller</title>"
"<script>"
"function updateStatus() {"
" fetch('/status.json')"
" .then(response => response.json())"
" .then(data => {"
"   document.getElementById('ain0_raw').innerText = data.ain0_raw;"
"   document.getElementById('ain0_c').innerText = data.ain0_c;"
"   document.getElementById('ain0_f').innerText = data.ain0_f;"
"   document.getElementById('ain1_raw').innerText = data.ain1_raw;"
"   document.getElementById('ain1_c').innerText = data.ain1_c;"
"   document.getElementById('ain1_f').innerText = data.ain1_f;"
"   document.getElementById('ain2_raw').innerText = data.ain2_raw;"
"   document.getElementById('ain2_c').innerText = data.ain2_c;"
"   document.getElementById('ain2_f').innerText = data.ain2_f;"
"   document.getElementById('ain3_raw').innerText = data.ain3_raw;"
"   document.getElementById('ain3_c').innerText = data.ain3_c;"
"   document.getElementById('ain3_f').innerText = data.ain3_f;"
"   document.getElementById('ain4_raw').innerText = data.ain4_raw;"
"   document.getElementById('ain4_c').innerText = data.ain4_c;"
"   document.getElementById('ain4_f').innerText = data.ain4_f;"
"   document.getElementById('din0').className = 'led ' + (data.din0 ? 'green' : 'gray');"
"   document.getElementById('din1').className = 'led ' + (data.din1 ? 'green' : 'gray');"
"   document.getElementById('dout0').className = 'led ' + (data.dout0 ? 'red' : 'darkred');"
"   document.getElementById('dout1').className = 'led ' + (data.dout1 ? 'red' : 'darkred');"
"   document.getElementById('dout2').className = 'led ' + (data.dout2 ? 'red' : 'darkred');"
"   document.getElementById('dout3').className = 'led ' + (data.dout3 ? 'red' : 'darkred');"
"   document.getElementById('dout4').className = 'led ' + (data.dout4 ? 'red' : 'darkred');"
"   document.getElementById('dout5').className = 'led ' + (data.dout5 ? 'red' : 'darkred');"
"   document.getElementById('dout6').className = 'led ' + (data.dout6 ? 'red' : 'darkred');"
" });"
"}"
"setInterval(updateStatus, 500);"
"updateStatus();"
"function toggleOut(num) {"
" fetch('/toggle?num=' + num);"
" updateStatus();"
"}"
"</script>"
"<style>"
"  .led { width: 20px; height: 20px; border-radius: 50%; display: inline-block; margin: 5px; cursor: pointer; }"
"  .green { background-color: lime; }"
"  .gray { background-color: gray; }"
"  .red { background-color: red; }"
"  .darkred { background-color: #440000; }"
"  table { border-collapse: collapse; margin: 20px 0; font-size: 14px; }"
"  th, td { border: 1px solid #ccc; padding: 8px; text-align: center; }"
"  th { background-color: #f2f2f2; }"
"  .name-col { text-align: left; width: 160px; }"
"</style>"
"</head><body>"
"<h1>Remote IO Controller</h1>"

"<h2>Analog Inputs</h2>"
"<table>"
"  <tr>"
"    <th class='name-col'>Name</th>"
"    <th>Raw A2D</th>"
"    <th>Temp C / Amps</th>"
"    <th>Temp F</th>"
"  </tr>"
"  <tr><td>CPU Temp</td><td id='ain0_raw'>--</td><td id='ain0_c'>--</td><td id='ain0_f'>--</td></tr>"
"  <tr><td>Board Temp</td><td id='ain1_raw'>--</td><td id='ain1_c'>--</td><td id='ain1_f'>--</td></tr>"
"  <tr><td>Remote Temp 1</td><td id='ain2_raw'>--</td><td id='ain2_c'>--</td><td id='ain2_f'>--</td></tr>"
"  <tr><td>Remote Temp 2</td><td id='ain3_raw'>--</td><td id='ain3_c'>--</td><td id='ain3_f'>--</td></tr>"
"  <tr><td>Current Transformer</td><td id='ain4_raw'>--</td><td id='ain4_c'>--</td><td id='ain4_f'>--</td></tr>"
"</table>"

"<h2>Digital Inputs</h2>"
"<div class='led gray' id='din0'></div> DIN 1<br>"
"<div class='led gray' id='din1'></div> DIN 2<br>"

"<h2>Digital Outputs (click to toggle)</h2>"
"<div class='led darkred' id='dout0' onclick='toggleOut(0)'></div> W<br>"
"<div class='led darkred' id='dout1' onclick='toggleOut(1)'></div> Y<br>"
"<div class='led darkred' id='dout2' onclick='toggleOut(2)'></div> G<br>"
"<div class='led darkred' id='dout3' onclick='toggleOut(3)'></div> W2<br>"
"<div class='led darkred' id='dout4' onclick='toggleOut(4)'></div> Y2<br>"
"<div class='led darkred' id='dout5' onclick='toggleOut(5)'></div> Aux 1<br>"
"<div class='led darkred' id='dout6' onclick='toggleOut(6)'></div> Relay<br>"
"</body></html>";

// CGI handler for toggle (old style: returns filename to serve next)
const char * toggle_cgi_handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    for (int i = 0; i < iNumParams; i++) {
        if (strcmp(pcParam[i], "num") == 0) {
            uint8_t num = (uint8_t)atoi(pcValue[i]);
            if (num < 7) {  // 7 outputs
                bool current = IO_GetOutput(num);
                IO_SetOutput(num, !current);
            }
        }
    }
    return "/index.html";  // Redirect to main page (browser reloads with updated states)
}

// CGI table
static const tCGI cgi_handlers[] = {
    { "/toggle", toggle_cgi_handler }
};

static void cgi_init(void)
{
    http_set_cgi_handlers(cgi_handlers, sizeof(cgi_handlers) / sizeof(tCGI));
}

// Custom file open for dynamic content
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
            "{"
            "\"ain0_raw\":%u,\"ain0_c\":%.1f,\"ain0_f\":%.1f,"
            "\"ain1_raw\":%u,\"ain1_c\":%.1f,\"ain1_f\":%.1f,"
            "\"ain2_raw\":%u,\"ain2_c\":%.1f,\"ain2_f\":%.1f,"
            "\"ain3_raw\":%u,\"ain3_c\":%.1f,\"ain3_f\":%.1f,"
            "\"ain4_raw\":%u,\"ain4_c\":%.1f,\"ain4_f\":%.1f,"
            "\"din0\":%s,\"din1\":%s,"
            "\"dout0\":%s,\"dout1\":%s,\"dout2\":%s,\"dout3\":%s,\"dout4\":%s,\"dout5\":%s,\"dout6\":%s"
            "}",
            IO_GetAnalogInRaw(0), IO_GetAnalogIn(0,0), IO_GetAnalogIn(0,1),
            IO_GetAnalogInRaw(1), IO_GetAnalogIn(1,0), IO_GetAnalogIn(1,1),
            IO_GetAnalogInRaw(2), IO_GetAnalogIn(2,0), IO_GetAnalogIn(2,1),
            IO_GetAnalogInRaw(3), IO_GetAnalogIn(3,0), IO_GetAnalogIn(3,1),
            IO_GetAnalogInRaw(4), IO_GetAnalogIn(4,0), IO_GetAnalogIn(4,1),
            IO_GetInput(0) ? "true" : "false", IO_GetInput(1) ? "true" : "false",
            IO_GetOutput(0) ? "true" : "false", IO_GetOutput(1) ? "true" : "false",
            IO_GetOutput(2) ? "true" : "false", IO_GetOutput(3) ? "true" : "false",
            IO_GetOutput(4) ? "true" : "false", IO_GetOutput(5) ? "true" : "false",
            IO_GetOutput(6) ? "true" : "false"
        );

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
    // No-op
}

void WebServer_Init(void)
{
    httpd_init();
    cgi_init();
}
