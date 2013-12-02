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

volatile uint8_t spi_busy_semaphore = 0;
volatile uint8_t cs_is_restored = 1;
ISR(PCINT0_vect)
{
    if (PINB & (1<<PB2))
    {
        spi_busy_semaphore = 0;
        cs_is_restored = 1;
    }
    else 
    {
        spi_busy_semaphore = 1;
    }
}

void spi_wait(void)
{
    while(!(SPSR & (1<<SPIF))) {
        DDRD |= (1<<PD0);
        PORTD |= (1<<PD0);
    }
    PORTD &= ~(1<<PD0);
}

volatile uint8_t ipc_rcv_buf = 0;
volatile uint8_t rx_buf[IPC_RX_BUF_LEN] = {0};
volatile uint8_t rx_write_ptr = 0;
volatile uint8_t rx_read_ptr = 0;

volatile uint8_t tx_buf[IPC_RX_BUF_LEN] = {0};
volatile uint8_t tx_write_ptr = 0;
volatile uint8_t tx_read_ptr = 0;

/* Debug functions */
static uint16_t sent_pkts = 0;
static uint16_t fetched_pkts = 0;

uint16_t ipc_sent_packets(void)
{
    return sent_pkts;
}
uint16_t ipc_fetched_packets(void)
{
    return fetched_pkts;
}

void print_tx_read_ptr()
{
    char tmp[10];
    itoa(tx_read_ptr, tmp, 10);
    lcd_write_string("txr:");
    lcd_write_string(tmp);
}
void print_tx_buf_len()
{
    char tmp[10];
    itoa(tx_write_ptr, tmp, 10);
    lcd_write_string("txw:");
    lcd_write_string(tmp);
}
/* End debud functions */

static aaps_result_t put_packet_in_tx_buf(struct ipc_packet_t *pkt)
{
    char tmp[10];
    static uint8_t overflow = 0;

    if (pkt == NULL)
        return AAPS_RET_ERROR_BAD_PARAMETERS;
    aaps_result_t res = AAPS_RET_OK;

    uint8_t *pkt_ptr = (uint8_t *)pkt;
    uint8_t rollback = tx_write_ptr; 
    for (uint8_t i = 0; i < pkt->len; i++)
    {
        if (&(tx_buf[(tx_write_ptr + 1 % IPC_RX_BUF_LEN)]) == &(tx_buf[tx_read_ptr]))
        {
            itoa(++overflow, tmp, 10);
            lcd_set_cursor_pos(0);
            lcd_write_string("BO: ");
            lcd_write_string(tmp);
            tx_write_ptr = rollback ;
            res = AAPS_RET_ERROR_GENERAL;
            goto overflow;
             
        }
        tx_buf[tx_write_ptr] =  pkt_ptr[i];
        tx_write_ptr = (tx_write_ptr + 1) % IPC_RX_BUF_LEN;
    }
    sent_pkts++;
overflow:
    return res;
}

ipc_result_t ipc_get_packet_from_buf()
{
    //char tmp[10];
    uint8_t pkt_len = tx_buf[tx_read_ptr];
    uint8_t rx;
    uint8_t tmp;


    if (ipc_is_tx_buf_empty())
        return IPC_RET_TX_BUF_EMPTY;

    /* Decide if master is getting or putting */
    tmp = SPDR;

    spi_wait();
    if (((~tmp) & 0xff) == 0x55)
    {
        rx = 1;
    }

    SPDR = (uint8_t)(~0xFC) & 0xff;
    spi_wait();
    SPDR = ~pkt_len;
    spi_wait();
   
    while(pkt_len--)
    {
        SPDR = ~(tx_buf[tx_read_ptr]);
        tx_read_ptr = (tx_read_ptr + 1) % IPC_RX_BUF_LEN;
        spi_wait();
        //tmpd = SPDR;
    }

    fetched_pkts++;
    return IPC_RET_OK;
}
bool ipc_is_tx_buf_empty(void)
{
    if (tx_write_ptr ==
        tx_read_ptr )
    {
        return true;
    }
    else 
    {
        return false;
    }
}

/* Remove this debug function */
void ipc_set_tx_buf_empty(void)
{
    tx_write_ptr = 0;
}

void send_ipc_enc_new(uint16_t enc_value)
{
    struct ipc_packet_t pkt;
    pkt.len = 5;
    pkt.cmd = IPC_DATA_ENC;
    pkt.data[0] = (enc_value >> 8);
    pkt.data[1] = (enc_value & 0xff);
    pkt.crc = 0xff;
    if (put_packet_in_tx_buf(&pkt) != AAPS_RET_OK)
        return;
    IRQ_SET();
    IRQ_CLR();
}


