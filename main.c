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

    /*
     * Read channel if from eeprom? Or say hello with type
     * of peripheral?
     */
    //ipc_print_str("[G] Hi from GUI\n";

    while(1)
    {
        /* Handle IPC traffic */
        if (ipc_transfer() != IPC_RET_OK)
        {
            /*TODO: Add error handling */
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

