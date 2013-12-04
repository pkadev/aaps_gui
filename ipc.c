#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


#include "cmd_exec.h"
#include "ipc.h"
#include "m48_hal.h"
#include "enc.h"
#include "lcd.h"
volatile uint8_t spi_busy_semaphore = 0;
volatile uint8_t cs_is_restored = 1;



#define IPC_DUMMY_BYTE 0xff
#define IPC_SYNC_BYTE 0xfc
#define IPC_FINALIZE_BYTE 0xc0
#define IPC_GET_BYTE 0x55
#define IPC_PUT_BYTE 0x66

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


static aaps_result_t put_packet_in_tx_buf(struct ipc_packet_t *pkt)
{
    if (pkt == NULL)
        return AAPS_RET_ERROR_BAD_PARAMETERS;
    aaps_result_t res = AAPS_RET_OK;

    uint8_t *pkt_ptr = (uint8_t *)pkt;
    uint8_t rollback = tx_write_ptr; 
    for (uint8_t i = 0; i < pkt->len; i++)
    {
        if (&(tx_buf[(tx_write_ptr + 1 % IPC_RX_BUF_LEN)]) == &(tx_buf[tx_read_ptr]))
        {
            /* This is overflow in the internal transmit buffer */
            enc_gled_on();
            enc_rled_on();
            tx_write_ptr = rollback ;
            res = AAPS_RET_ERROR_GENERAL;
            goto overflow;
             
        }
        tx_buf[tx_write_ptr] =  pkt_ptr[i];
        tx_write_ptr = (tx_write_ptr + 1) % IPC_RX_BUF_LEN;
    }
overflow:
    return res;
}
ipc_result_t ipc_receive(char *rx_buf)
{
    uint8_t data, rx_len;
    SPDR = (uint8_t)(~IPC_SYNC_BYTE) & 0xff;
    spi_wait();
    data = SPDR; /* Useless data */
    spi_wait();
    data = SPDR; /* Packet length */
    rx_len = (~data) -1; /* Subtract one since size is already transmitted */
    spi_wait();

    while(rx_len--)
    {
        rx_buf[3 -rx_len] = (~SPDR) & 0xff;
        spi_wait(); 
    }
    SPDR = (~0xC0) & 0xff;
    //spi_wait();
    return ~rx_len;
}
ipc_result_t ipc_transfer()
{
    uint8_t pkt_len = tx_buf[tx_read_ptr];
    uint8_t tmp;
    
    if (ipc_is_tx_buf_empty())
        return IPC_RET_TX_BUF_EMPTY;

    tmp = SPDR;
    spi_wait();
  
    enc_gled_on();
    
    SPDR = (uint8_t)(~IPC_SYNC_BYTE) & 0xff;
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

    return IPC_RET_OK;
}
bool ipc_is_tx_buf_empty(void)
{
    if (tx_write_ptr == tx_read_ptr )
        return true;
    else 
        return false;
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


