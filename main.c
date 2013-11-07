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

struct ipc_packet_t ipc_packet = {0};

    /*
     * TODO: Changes ISRs for AAPS_G
     * Current ISR context is copy paste from
     * AAPS_A
     */


int main(void)
{
    uint16_t curr_enc_pos;

    aaps_result_t result = AAPS_RET_OK;
    if (boot() != AAPS_RET_OK)
        boot_failed();

    curr_enc_pos = get_enc_pos();
    /*
     * Read channel if from eeprom? Or say hello with type
     * of peripheral?
     */
    print_ipc_int("[G] Hi from GUI: ", 1);

    while(1)
    {
        if (result == AAPS_RET_OK)
        {
            if (packets_available)
            {
                ipc_handle_packet(&ipc_packet);
            }
        } else {
            const char str[] = "[G]Critical error\n";
            print_ipc(str, strlen(str));
            //LED_CLR();
            while(1);
        }
        uint16_t new_enc_pos = get_enc_pos();
        if (curr_enc_pos != new_enc_pos)
        {
            curr_enc_pos = new_enc_pos;
            send_ipc_enc(new_enc_pos);
        }
    }
    return 0; //Should never get here
}

