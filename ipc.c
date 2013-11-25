#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>

#include "cmd_exec.h"
#include "ipc.h"
#include "m48_hal.h"
#include "enc.h"
#include "lcd.h"

static void spi_wait(void)
{
    while(!(SPSR & (1<<SPIF))) {
        DDRD |= (1<<PD0);
        PORTD |= (1<<PD0);
    }
    PORTD &= ~(1<<PD0);
}

volatile uint8_t ipc_rcv_buf = 0;
volatile uint8_t rx_buf[IPC_RX_BUF_LEN] = {0};
volatile uint8_t write_ptr = 0;
volatile uint8_t read_ptr = 0;
volatile uint8_t packets_available = 0;
char sendbuffer[40] = {0};


ISR(SPI_STC_vect)
{
    static uint8_t internal_cnt = 0;
    ipc_rcv_buf = SPDR;
    rx_buf[write_ptr++] = SPDR;
    write_ptr %= IPC_RX_BUF_LEN;
    internal_cnt++;

    if (internal_cnt == IPC_PACKET_LEN) {
        packets_available++;
        internal_cnt = 0;
        //LED_TOGGLE();
    }
}

static void ipc_save_packet(struct ipc_packet_t *dst, size_t len, uint8_t read_ptr)
{
    dst->cmd = *(rx_buf + (read_ptr++ % IPC_RX_BUF_LEN));
    dst->len = *(rx_buf + (read_ptr++ % IPC_RX_BUF_LEN));
    dst->data[0] = *(rx_buf + (read_ptr++ % IPC_RX_BUF_LEN));
    dst->data[1] = *(rx_buf + (read_ptr++ % IPC_RX_BUF_LEN));
    dst->crc = *(rx_buf + (read_ptr++ % IPC_RX_BUF_LEN));

    packets_available--;
}

void send_ipc_enc(uint16_t enc_value)
{
    sendbuffer[0] = IPC_DATA_ENC;
    sendbuffer[1] = (enc_value >> 8);
    sendbuffer[2] = (enc_value & 0xff);
    sendbuffer[3] = '\0';
    print_ipc(sendbuffer, 4);
}

aaps_result_t ipc_handle_packet(struct ipc_packet_t *ipc_packet)
{
    /* TODO: Make exec functions return status */

    ipc_save_packet(ipc_packet, IPC_PACKET_LEN, read_ptr);
    read_ptr += IPC_PACKET_LEN;
    read_ptr %= IPC_RX_BUF_LEN;

    switch(ipc_packet->cmd)
    {
        case IPC_CMD_PERIPH_DETECT:
        {
            const char str[] = "[G] P detect\n";
            print_ipc(str, strlen(str));
          break;
        }
        case IPC_CMD_PUT_DATA:
            cmd_exec_display_voltage(ipc_packet);
            break;
        case IPC_CMD_SET_RELAY_D:
            cmd_exec_ctrl_relay(ipc_packet, RELAY_D_ID);
            break;
        case IPC_CMD_SET_RELAY:
            cmd_exec_ctrl_relay(ipc_packet, RELAY_ID);
            break;
        default:
            print_ipc_int("[G] Unsupported ipc command 0x", ipc_packet->cmd);
            read_ptr = 0;
            return AAPS_RET_ERROR_GENERAL;
    }
    return AAPS_RET_OK;
}

/*
 * TODO: Find out if we need both signed and unsigned version
 * of this function. Or better send raw data here and use one
 * byte to tell if it's signed or unsigned.
 */
void print_ipc_int(const char *str, unsigned int integer)
{
    char buf[17]; //(sizeof(int)*8+1) All integers fit his on radix=2 systems
    uint8_t str_len = strlen(str);
    uint8_t buf_len;
    sendbuffer[0] = IPC_DATA_ASCII;
    memcpy(sendbuffer+1, str, str_len);

    ltoa(integer, buf, 10);
    buf_len = strlen(buf);

    memcpy(sendbuffer + 1 + str_len, buf, buf_len);
    memset(sendbuffer + 1 + str_len + buf_len, '\n', 1);
    memset(sendbuffer + 1 + str_len + buf_len + 1, 0, 1);

    /*
     * TODO: Fix this temporary solution:
     * Add two bytes for the integer and one
     * byte for null termination.
     */
    print_ipc(sendbuffer, strlen(str) + 3);
}
void print_ipc(const char *str, size_t len)
{
    /* TODO: This can't be used standalone
     * if no data type is specified.
     */

    uint8_t i;
    SPCR &= ~(1<<SPIE);

    /* Put CMD in SPI data buffer */
    SPDR = ~IPC_CMD_DATA_AVAILABLE;

    /* Signal to master that CMD is available */
    IRQ_SET();
    IRQ_CLR();
    spi_wait();

    /* Tell master how many bytes to fetch */
    SPDR = ~len;
    spi_wait();
    for(i = 0; i < len; i++) {
        SPDR = ~(str[i]);
        spi_wait();
    }

    i = SPSR;
    i = SPDR;
    SPCR |= (1<<SPIE);
    /*
     * TODO: This must be here if you print two times in a row.
     * Need to figure out why this is!
     */
}
