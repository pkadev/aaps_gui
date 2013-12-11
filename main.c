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

int main(void)
{
    uint16_t curr_enc_pos;

    if (boot() != AAPS_RET_OK)
        boot_failed();

    curr_enc_pos = get_enc_pos();

    //core_init();
    /*
     * Read channel if from eeprom? Or say hello with type
     * of peripheral?
     */
    //ipc_print_str("[G] Hi from GUI\n";
    struct ipc_packet_t ipc_pkt;
    uint16_t pkt_cnt = 0;
    char buf[10];
    while(1)
    {
        /* Handle IPC traffic */
        if (ipc_transfer(&ipc_pkt) == IPC_RET_OK)
        {
            if(packets_pending())
            {
                lcd_set_cursor_pos(99);
                utoa(pkt_cnt++, buf, 10);
                lcd_write_string(buf);

                switch(ipc_pkt.cmd)
                {
                    case IPC_CMD_DISPLAY_THERMO:
                    {
                        //struct temperature_t temp;
                        //temp.whole = ipc_pkt.data[1];
                        //temp.decimal = ipc_pkt.data[2];

                        //core_draw_temp(&temp, ipc_pkt.data[0]);
                        break;
                    }
                    case IPC_CMD_DISPLAY_ADC:
                    {
                        core_draw_adc(ipc_pkt.data[2], ipc_pkt.data[3],
                                      ipc_pkt.data[0], ipc_pkt.data[1]);
                        break;
                    }
                    case IPC_CMD_DISPLAY_DAC:
                    {
                        core_draw_dac(&ipc_pkt);
                        break;
                    }
                    case IPC_CMD_DISPLAY_VOLTAGE:
                    {
                        core_draw_voltage(&ipc_pkt);
                        break;
                    }
                    default:
                       lcd_write_int(ipc_pkt.cmd);
                       lcd_write_string(" Unsupported command!");
                }
                ipc_reduce_pkts_pending(&ipc_pkt);
            }
        }
        else
        {
            /*TODO: Add error handling */
            lcd_write_string("IPC Error");
            while(1);
        }

        /* Handle all other system events */
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

        if (enc_btn_event)
        {
            btn_event();
            ipc_send_enc(0xBEEF);
            enc_btn_event = 0;
        }
        uint16_t new_enc_pos = get_enc_pos();
        if (curr_enc_pos != new_enc_pos)
        {
            curr_enc_pos = new_enc_pos;
            ipc_send_enc(new_enc_pos);

        }
    }
    return 0; //Should never get here
}

