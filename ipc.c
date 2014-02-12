#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "cmd_exec.h"
#include "ipc.h"
#include "m48_hal.h"
#include "enc.h"
#include "lcd.h"
#include "crc8.h"

#define IPC_DUMMY_BYTE 0xff
#define IPC_SYNC_BYTE 0xfc
#define IPC_FINALIZE_BYTE 0xc0
#define IPC_GET_BYTE 0x55
#define IPC_PUT_BYTE 0x66
#define SPDR_INV(x) ((uint8_t)(~x)&0xff)

#define IPC_PKT_OVERHEAD 3  /* Len, cmd, crc */

volatile uint8_t spi_busy_semaphore = 0;
volatile uint8_t cs_is_restored = 1;

volatile uint8_t rx_buf[IPC_RX_BUF_LEN] = {0};
volatile uint8_t rx_write_ptr = 0;
volatile uint8_t rx_read_ptr = 0;

volatile uint8_t tx_buf[IPC_RX_BUF_LEN] = {0};
volatile uint8_t tx_write_ptr = 0;
volatile uint8_t tx_read_ptr = 0;

static uint8_t pkts_pending = 0;

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
    if (tx_write_ptr == tx_read_ptr)
        return true;
    else
        return false;
}

static aaps_result_t put_packet_in_tx_buf(struct ipc_packet_t *pkt)
{
    aaps_result_t res = AAPS_RET_OK;
    uint8_t *pkt_ptr = (uint8_t *)pkt;
    uint8_t rollback = tx_write_ptr;
    uint8_t i;

    if (pkt == NULL)
        return AAPS_RET_ERROR_BAD_PARAMETERS;



    /* First add overhead bytes */
    for (i = 0; i < IPC_PKT_OVERHEAD; i++)
    {
        if (&(tx_buf[(tx_write_ptr + 1 % IPC_RX_BUF_LEN)]) == &(tx_buf[tx_read_ptr]))
        {
            /* This is overflow in the internal transmit buffer */
            tx_write_ptr = rollback;
            res = AAPS_RET_ERROR_GENERAL;
            goto overflow;
        }
        tx_buf[tx_write_ptr] = pkt_ptr[i];
        tx_write_ptr = (tx_write_ptr + 1) % IPC_RX_BUF_LEN;
    }

    /* Then data from from heap */
    for (i = 0; i < pkt->len - IPC_PKT_OVERHEAD; i++)
    {
        if (&(tx_buf[(tx_write_ptr + 1 % IPC_RX_BUF_LEN)]) == &(tx_buf[tx_read_ptr]))
        {
            /* This is overflow in the internal transmit buffer */
            tx_write_ptr = rollback;
            res = AAPS_RET_ERROR_GENERAL;
            goto overflow;
        }
        tx_buf[tx_write_ptr] = pkt->data[pkt->len - IPC_PKT_OVERHEAD - i - 1];
        tx_write_ptr = (tx_write_ptr + 1) % IPC_RX_BUF_LEN;
    }
overflow:
    return res;
}

static ipc_ret_t ipc_receive(struct ipc_packet_t *rx_pkt)
{
    /*
     * TODO: Handle errors. Sequence nbr, crc
     * Alse handle return value. Put pkt size in the
     * received data structure.
     */
    uint8_t data, data_len;
    ipc_ret_t ret = IPC_RET_OK;
    SPDR = SPDR_INV(IPC_SYNC_BYTE);
    spi_wait();
    data = SPDR; /* Useless data because the nature of SPI */
    spi_wait();

    /* Total packet length */
    rx_pkt->len = SPDR_INV(SPDR);

    rx_pkt->data = malloc(rx_pkt->len - IPC_PKT_OVERHEAD);
    if (rx_pkt->data == NULL)
    {
        ret = IPC_RET_ERROR_OUT_OF_MEMORY;
        goto exit;
    }

    /* Payload length */
    data_len = rx_pkt->len - IPC_PKT_OVERHEAD;
    spi_wait();

    /* Cmd */
    rx_pkt->cmd = SPDR_INV(SPDR);
    spi_wait();

    /* CRC */
    rx_pkt->crc = SPDR_INV(SPDR);
    spi_wait();

    uint8_t cnt = 0;
    while(data_len--)
    {
        rx_pkt->data[cnt++] = SPDR_INV(SPDR);
        spi_wait();
    }

    if (crc8(rx_pkt->data, rx_pkt->len - IPC_PKT_OVERHEAD) != rx_pkt->crc)
       ret = IPC_RET_ERROR_CRC_FAIL;
exit:
    /* Clear SPDR so it's not sync/finalize byte */
    SPDR = SPDR_INV(0x00);
    return ret;
}

static ipc_ret_t ipc_transmit()
{
    uint8_t pkt_len = tx_buf[tx_read_ptr];
    uint8_t tmp;

    if (ipc_is_tx_buf_empty())
        return IPC_RET_ERROR_TX_BUF_EMPTY;

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
    SPDR = SPDR_INV(0x00);
    return IPC_RET_OK;
}

void ipc_init(void)
{
    spi_busy_semaphore = 0;
    cs_is_restored = 1;
}

uint8_t packets_pending()
{
    return pkts_pending;
}
void ipc_reduce_pkts_pending(struct ipc_packet_t *pkt)
{
    free(pkt->data);
    pkt->data = NULL;
    pkts_pending--;
}
ipc_ret_t ipc_transfer(struct ipc_packet_t *pkt)
{
    /* TODO: Handle dynamic packet sizes */
    uint8_t misc;
    ipc_ret_t res = IPC_RET_OK;

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
                ATOMIC_BLOCK(ATOMIC_FORCEON)
                {
                    /* Master puts data */
                    misc = ipc_receive(pkt);
                }
                if (misc == IPC_RET_OK)
                {
                    pkts_pending++;
                    res = misc;
                    /*
                     * TODO: handle the packet in generic way.
                     * Probably add it to a list of packets for
                     * the system to handle in a timely fashion
                     */
                    goto end;
                }
                else
                {
                    res = misc;
                    goto end;
                }
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
    return res;
}

/* TODO: Move to "business unit layer" ? */
void ipc_send_enc(enum ipc_data_type_t type)
{
    /* TODO: Handle errors */
    struct ipc_packet_t pkt;
    pkt.len = 1 + IPC_PKT_OVERHEAD;
    pkt.cmd = type;
    pkt.data = malloc(pkt.len - IPC_PKT_OVERHEAD);

    if (pkt.data == NULL)
    {
        /* Handle error */
        return;
    }

    pkt.data[0] = 0; /* Not used */

    pkt.crc = crc8(pkt.data, 1);

    if (put_packet_in_tx_buf(&pkt) != AAPS_RET_OK)
    {
        /* Handle error */
        return;
    }

    free(pkt.data);

    IRQ_SET();
    /* TODO: Find out if we need NOP here */
    IRQ_CLR();
}


aaps_result_t core_send_periph_info(void)
{
    /* TODO: Handle errors */
    struct ipc_packet_t pkt;
    uint8_t payload_len = 3;
    uint8_t total_len = payload_len + IPC_PKT_OVERHEAD;
    uint16_t device_id = 0; /* TODO: Implement read_device_id(); */

    pkt.len = total_len;
    pkt.cmd = IPC_DATA_PERIPH_DETECT;
    pkt.data = malloc(payload_len);

    if (pkt.data == NULL)
        return AAPS_RET_ERROR_OUT_OF_MEMORY;

    pkt.data[0] = device_id << 8;
    pkt.data[1] = device_id & 0xFF;
    pkt.data[2] = 0;/* TODO:  Sort out differences ow_get_num_sensors(); */

    pkt.crc = crc8(pkt.data, payload_len);

    if (put_packet_in_tx_buf(&pkt) != AAPS_RET_OK)
    {
        return AAPS_RET_ERROR_GENERAL;
    }

    free(pkt.data);

    IRQ_SET();
    /* TODO: Find out if we need NOP here */
    IRQ_CLR();
    return AAPS_RET_OK;
}
