//-- modbus_tcp.c --//
#include "modbus_tcp.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "io_core.h"
#include "sysstatus.h"
#include <string.h>

#define MODBUS_TCP_PORT    502
#define MAX_MODBUS_PDU     253

static struct tcp_pcb *modbus_pcb = NULL;

#define EX_ILLEGAL_FUNCTION   0x01
#define EX_ILLEGAL_DATA_ADDR  0x02
#define EX_ILLEGAL_DATA_VALUE 0x03

static uint8_t unit_id = 1;

// Forward declaration to be safe
static err_t modbus_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

static err_t modbus_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);

    tcp_accepted(modbus_pcb);
    tcp_recv(newpcb, modbus_recv);   // Now recognized
    tcp_err(newpcb, NULL);
    tcp_poll(newpcb, NULL, 4);

    return ERR_OK;
}

static err_t modbus_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    LWIP_UNUSED_ARG(arg);

    if (p == NULL) {
        tcp_close(tpcb);
        return ERR_OK;
    }

    if (err != ERR_OK || p->tot_len < 8) {
        pbuf_free(p);
        return ERR_OK;
    }

    uint8_t *data = (uint8_t *)p->payload;

    if (data[6] != unit_id && data[6] != 0xFF) {
        pbuf_free(p);
        return ERR_OK;
    }

    uint8_t fc = data[7];
    uint16_t addr, qty;
    uint8_t resp[260];
    uint16_t resp_len = 0;

    memcpy(resp, data, 6);
    resp[6] = unit_id;

    SysStatus_Comms_Trigger();

    switch (fc) {
        case 0x01:  // Read Coils
        case 0x02:  // Read Discrete Inputs
		{
			addr = (data[8] << 8) | data[9];
			qty = (data[10] << 8) | data[11];

			bool is_coil = (fc == 0x01);
			uint16_t max_count = is_coil ? DOUT_COUNT : DIN_COUNT;

			// Only error if starting address invalid or qty=0
			if (addr >= max_count || qty == 0) {
				resp[7] = fc | 0x80;
				resp[8] = EX_ILLEGAL_DATA_ADDR;
				resp_len = 9;
				break;
			}

			// Clamp qty to available points (no exception)
			if (addr + qty > max_count) {
				qty = max_count - addr;
			}

			uint8_t byte_count = (qty + 7) / 8;
			resp[7] = fc;
			resp[8] = byte_count;
			resp_len = 9 + byte_count;

			memset(&resp[9], 0, byte_count);

			for (uint16_t i = 0; i < qty; i++) {
				bool val = is_coil ? IO_GetOutput(addr + i) : IO_GetInput(addr + i);
				if (val) {
					resp[9 + i/8] |= (1 << (i % 8));
				}
			}
			break;
		}

        case 0x04:  // Read Input Registers
		{
			addr = (data[8] << 8) | data[9];
			qty = (data[10] << 8) | data[11];

			if (addr >= AIN_COUNT || qty == 0) {
				resp[7] = fc | 0x80;
				resp[8] = EX_ILLEGAL_DATA_ADDR;
				resp_len = 9;
				break;
			}

			// Clamp qty to available registers
			if (addr + qty > AIN_COUNT) {
				qty = AIN_COUNT - addr;
			}

			uint8_t byte_count = qty * 2;
			resp[7] = fc;
			resp[8] = byte_count;
			resp_len = 9 + byte_count;

			for (uint16_t i = 0; i < qty; i++) {
				uint16_t val = IO_GetAnalogIn(addr + i, celcius);
				resp[9 + i*2] = val >> 8;
				resp[9 + i*2 + 1] = val & 0xFF;
			}
			break;
		}

        case 0x05:  // Write Single Coil
        {
            addr = (data[8] << 8) | data[9];
            uint16_t value = (data[10] << 8) | data[11];

            if (addr >= DOUT_COUNT) {
                resp[7] = fc | 0x80;
                resp[8] = EX_ILLEGAL_DATA_ADDR;
                resp_len = 9;
                break;
            }

            bool val = (value == 0xFF00);
            IO_SetOutput(addr, val);

            memcpy(&resp[7], &data[7], 5);
            resp_len = 12;
            break;
        }

        case 0x0F:  // Write Multiple Coils
		{
			addr = (data[8] << 8) | data[9];
			qty = (data[10] << 8) | data[11];
			uint8_t byte_count = data[12];

			if (addr >= DOUT_COUNT || qty == 0 || byte_count != (qty + 7)/8) {
				resp[7] = fc | 0x80;
				resp[8] = EX_ILLEGAL_DATA_ADDR;
				resp_len = 9;
				break;
			}

			// Clamp qty to available coils
			if (addr + qty > DOUT_COUNT) {
				qty = DOUT_COUNT - addr;
			}

			for (uint16_t i = 0; i < qty; i++) {
				bool val = (data[13 + i/8] & (1 << (i % 8))) != 0;
				IO_SetOutput(addr + i, val);
			}

			// Response: echo addr + qty (clamped qty is fine — spec allows)
			resp[7] = fc;
			resp[8] = addr >> 8;
			resp[9] = addr & 0xFF;
			resp[10] = qty >> 8;
			resp[11] = qty & 0xFF;
			resp_len = 12;
			break;
		}

        default:
            resp[7] = fc | 0x80;
            resp[8] = EX_ILLEGAL_FUNCTION;
            resp_len = 9;
            break;
    }

    uint16_t pdu_len = resp_len - 6;
    resp[4] = pdu_len >> 8;
    resp[5] = pdu_len & 0xFF;

    tcp_write(tpcb, resp, resp_len, TCP_WRITE_FLAG_COPY);
    tcp_output(tpcb);

    pbuf_free(p);
    return ERR_OK;
}

void ModbusTCP_Init(uint8_t NodeId)
{
    modbus_pcb = tcp_new();
    if (modbus_pcb != NULL) {
        err_t err = tcp_bind(modbus_pcb, IP_ADDR_ANY, MODBUS_TCP_PORT);
        if (err == ERR_OK) {
            modbus_pcb = tcp_listen(modbus_pcb);
            tcp_accept(modbus_pcb, modbus_accept);
        }
    }
}
