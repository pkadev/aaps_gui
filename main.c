#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "m48_hal.h"
#include "boot.h"
#include "ipc.h"
#include "cmd_exec.h"
#include "enc.h"
#include "lcd.h"
#include <util/atomic.h>

int main(void)
{
    uint16_t curr_enc_pos;
    
    if (boot() != AAPS_RET_OK)
        boot_failed();

    curr_enc_pos = get_enc_pos();

    /*
     * Read channel if from eeprom? Or say hello with type
     * of peripheral?
     */
    //ipc_print_str("[G] Hi from GUI\n";
    char tmp[10];
    spi_busy_semaphore = 0;
    while(1)
    {
        if (spi_busy_semaphore)
        {
            if (!ipc_is_tx_buf_empty() && cs_is_restored)
            {
                cs_is_restored = 0;

                if (ipc_get_packet_from_buf() != IPC_RET_OK) 
                {
                    /* TODO: Remove this crap */
                    static uint8_t empty = 0;
                    lcd_set_cursor_pos(20);
                    lcd_write_string("Empty: ");
                    itoa(empty++, tmp, 10);
                    lcd_write_string(tmp);
                }
                SPDR = (~0xC0) & 0xff;
                spi_wait();
                /* Not sure if this is needed */
                while(spi_busy_semaphore);
            }
        }
        else
        {

            if (enc_term_b_event)
            {
                term_b_event();
                enc_term_b_event = 0;
            }
            if (enc_term_a_event)
            {
                term_a_event();
                enc_term_a_event = 0;
            }

            //lcd_set_cursor_pos(60);
            //itoa(ipc_sent_packets(), tmp, 10); 
            //lcd_write_string(tmp);
            //lcd_set_cursor_pos(0);
            //itoa(ipc_fetched_packets(), tmp, 10); 
            //lcd_write_string(tmp);
            if (enc_btn_event)
            {
                btn_event();
                for (uint8_t j=0; j < 2; j++)
                    send_ipc_enc_new(j);
                enc_btn_event = 0;
            }
            uint16_t new_enc_pos = get_enc_pos();
            if (curr_enc_pos != new_enc_pos)
            {
                static uint8_t enc = 0;
                curr_enc_pos = new_enc_pos;
                send_ipc_enc_new(enc++);

            }
        }
    }
    return 0; //Should never get here
}

