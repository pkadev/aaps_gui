#include <avr/io.h>
#include <avr/interrupt.h>
#include "m48_hal.h"
#include "boot.h"
#include "ipc.h"
#include "cmd_exec.h"

struct ipc_packet_t ipc_packet = {0};

    /*
     * TODO: Changes ISRs for AAPS_G
     * Current ISR context is copy paste from
     * AAPS_A
     */
ISR(PCINT2_vect) { /*If SW1 is configured as PCINT18 */ }
ISR(INT1_vect)
{
    if (PIND & (1<<PD3))
        ;//LED_SET();
    else
        ;//LED_CLR();
}


int main(void)
{
    aaps_result_t result = AAPS_RET_OK;
    if (boot() != AAPS_RET_OK)
        boot_failed();

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
            print_ipc("[G]Critical error\n");
            //LED_CLR();
            while(1);
        }
    }
    return 0; //Should never get here
}

