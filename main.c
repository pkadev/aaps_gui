#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "core.h"
#include "m48_hal.h"
#include "boot.h"
#include "ipc.h"
#include "cmd_exec.h"
#include "enc.h"
#include "lcd.h"
#include "core.h"

//static void debug_led_init()
//{
//    DDRD |= (1<<PD0);
//}
//
//static void debug_led_on()
//{
//    PORTD |= (1<<PD0);
//}
//static void debug_led_off()
//{
//    PORTD &= ~(1<<PD0);
//}
int main(void)
{
    uint16_t curr_enc_pos;

    if (boot() != AAPS_RET_OK)
        boot_failed();

    curr_enc_pos = get_enc_pos();

    core_init();
    /*
     * Read channel if from eeprom? Or say hello with type
     * of peripheral?
     */
    //ipc_print_str("[G] Hi from GUI\n";
    struct ipc_packet_t ipc_pkt;
    while(1)
    {
        /* Handle IPC traffic */
        if (ipc_transfer(&ipc_pkt) == IPC_RET_OK)
        {
            if(packets_pending())
            {
                if (ipc_pkt.cmd == IPC_CMD_PUT_DATA)
                {
                    //struct temperature_t temp;
                    //temp.whole = ipc_pkt.data[1];
                    //temp.decimal = ipc_pkt.data[2];

                    //core_draw_temp(&temp, ipc_pkt.data[0]);
                    core_draw_adc(ipc_pkt.data[2], ipc_pkt.data[3],
                                  ipc_pkt.data[0], ipc_pkt.data[1]);
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

