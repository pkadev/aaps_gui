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

#define IPC_DUMMY_BYTE 0xff
#define IPC_SYNC_BYTE 0xfc
#define IPC_FINALIZE_BYTE 0xc0
#define IPC_GET_BYTE 0x55
#define IPC_PUT_BYTE 0x66

#define SPDR_INV(x) ((uint8_t)(~x)&0xff)

volatile uint8_t spi_busy_semaphore = 0;
volatile uint8_t cs_is_restored = 1;

volatile uint8_t rx_buf[IPC_RX_BUF_LEN] = {0};
volatile uint8_t rx_write_ptr = 0;
volatile uint8_t rx_read_ptr = 0;

volatile uint8_t tx_buf[IPC_RX_BUF_LEN] = {0};
volatile uint8_t tx_write_ptr = 0;
volatile uint8_t tx_read_ptr = 0;

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

static inline void spi_wait(void)
{
    while(!(SPSR & (1<<SPIF)));
}

static bool ipc_is_tx_buf_empty(void)
{
    if (tx_write_ptr == tx_read_ptr )
        return true;
    else
        return false;
}

static aaps_result_t put_packet_in_tx_buf(struct ipc_packet_t *pkt)
{
    aaps_result_t res = AAPS_RET_OK;
    uint8_t *pkt_ptr = (uint8_t *)pkt;
    uint8_t rollback = tx_write_ptr;

    if (pkt == NULL)
        return AAPS_RET_ERROR_BAD_PARAMETERS;
    for (uint8_t i = 0; i < pkt->len; i++)
    {
        if (&(tx_buf[(tx_write_ptr + 1 % IPC_RX_BUF_LEN)]) == &(tx_buf[tx_read_ptr]))
        {
            /* This is overflow in the internal transmit buffer */
            tx_write_ptr = rollback;
            res = AAPS_RET_ERROR_GENERAL;
            goto overflow;
        }
        tx_buf[tx_write_ptr] =  pkt_ptr[i];
        tx_write_ptr = (tx_write_ptr + 1) % IPC_RX_BUF_LEN;
    }
overflow:
    return res;
}

static ipc_result_t ipc_receive(char *rx_buf)
{
    /*
     * TODO: Handle errors. Sequence nbr, crc
     * Alse handle return value. Put pkt size in the
     * received data structure.
     */
    uint8_t data, rx_len;
    SPDR = SPDR_INV(IPC_SYNC_BYTE);
    spi_wait();
    data = SPDR; /* Useless data because the nature of SPI */
    spi_wait();
    data = SPDR; /* Packet length */
    rx_len = (~data) -1; /* Subtract one since size is already transmitted */
    spi_wait();

    while(rx_len--)
    {
        rx_buf[3 -rx_len] = (~SPDR) & 0xff;
        spi_wait(); 
    }

    /* Clear out SPDR so it's not sync/finalize byte */
    SPDR = SPDR_INV(0x00);
    return IPC_RET_OK;
}

static ipc_result_t ipc_transmit()
{
    uint8_t pkt_len = tx_buf[tx_read_ptr];
    uint8_t tmp;

    if (ipc_is_tx_buf_empty())
        return IPC_RET_TX_BUF_EMPTY;

    tmp = SPDR;
    spi_wait();

    SPDR = SPDR_INV(IPC_SYNC_BYTE);
    spi_wait();
    SPDR = ~pkt_len;

    spi_wait();

    while(pkt_len--)
    {
        SPDR = ~(tx_buf[tx_read_ptr]);
        tx_read_ptr = (tx_read_ptr + 1) % IPC_RX_BUF_LEN;
        spi_wait();
    }

    /* Clear out SPDR so it's not any sync/finalize byte */
    SPDR = (~0x00) & 0xff;
    return IPC_RET_OK;
}

void ipc_init(void)
{
    spi_busy_semaphore = 0;
    cs_is_restored = 1;
}

ipc_result_t ipc_transfer()
{
    /* TODO: Handle dynamic packet sizes */
    char rx_buf[4];
    uint8_t misc;

    if (spi_busy_semaphore)
    {
        if (cs_is_restored)
        {
            cs_is_restored = 0;
            spi_wait();
            misc = SPDR;
            spi_wait();
            misc = SPDR;

            if (misc == SPDR_INV(IPC_PUT_BYTE))
            {
                /* Master puts data */
                misc = ipc_receive(rx_buf);
                if (misc == IPC_RET_OK)
                {
                    /* TODO: handle the packet in generic way */
                    lcd_write_string(rx_buf);
                    goto end;
                }
                else
                    return misc;
            }
            else
            {
                /* Master gets data */
                if (ipc_transmit() != IPC_RET_OK)
                {
                    /* TODO: Handle error cases */
                }
                spi_wait();
            }
end:
            while(spi_busy_semaphore && !cs_is_restored)
                SPDR = SPDR_INV(IPC_FINALIZE_BYTE);
        }
    }
    return IPC_RET_OK;
}

/* TODO: Move to "business unit layer" ? */
void ipc_send_enc(uint16_t enc_value)
{
    /* TODO: Handle errors */
    struct ipc_packet_t pkt;
    pkt.len = 5;
    pkt.cmd = IPC_DATA_ENC;
    pkt.data[0] = (enc_value >> 8);
    pkt.data[1] = (enc_value & 0xff);
    pkt.crc = 0xff;
    if (put_packet_in_tx_buf(&pkt) != AAPS_RET_OK)
    {
        /* Handle error */
        return;
    }

    IRQ_SET();
    /* TODO: Find out if we need NOP here */
    IRQ_CLR();
}


