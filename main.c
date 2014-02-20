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

static uint8_t page = 2;
static void display_chg_page()
{
    lcd_clr_disp();
    if (page == 1) {
        core_init_page(2);
        page = 2;
    } else {
        core_init_page(1);
        page = 1;
    }
}

int main(void)
{
    if (boot() != AAPS_RET_OK)
        boot_failed();

    //DDRD &= ~(1<<PD0); /* PD0 Input */
    //PORTD |= (1<<PD0); /* Enable pull-up */

    DDRC &= ~(1<<PC5);
    PORTC |= (1<<PC5);

    DDRC &= ~(1<<PC4);
    PORTC |= (1<<PC4);

    core_init_page(page);
    /*
     * Read channel id from eeprom? Or say hello with type
     * of peripheral?
     */
    //ipc_print_str("[G] Hi from GUI\n";
    struct ipc_packet_t ipc_pkt;
    ipc_ret_t result;
    while(1)
    {
        /* Handle IPC traffic */
        result = ipc_transfer(&ipc_pkt);
        if (result == IPC_RET_OK)
        {
            if(packets_pending())
            {
                switch(ipc_pkt.cmd)
                {
                    case IPC_CMD_PERIPH_DETECT:
                    {
                        core_send_periph_info();
                        break;
                    }
                    case IPC_CMD_DISPLAY_THERMO:
                    {
                        if (page == 2)
                        {
                            struct temperature_t temp;
                            temp.whole = ipc_pkt.data[1];
                            temp.decimal = ipc_pkt.data[2];

                            core_draw_temp(&temp, ipc_pkt.data[0]);
                        }
                        break;
                    }
                    case IPC_CMD_DISPLAY_ADC:
                    {
                        if (page == 1)
                        core_draw_adc(ipc_pkt.data[2], ipc_pkt.data[3],
                                      ipc_pkt.data[0], ipc_pkt.data[1]);
                        break;
                    }
                    case IPC_CMD_DISPLAY_DAC:
                    {
                        if (page == 1)
                            core_draw_dac(&ipc_pkt);
                        break;
                    }
                    case IPC_CMD_DISPLAY_VOLTAGE:
                    {
                        if (page == 1)
                            core_draw_voltage(&ipc_pkt);
                        break;
                    }
                    case IPC_CMD_DISPLAY_CURRENT:
                    {
                        if (page == 1)
                            core_draw_current(&ipc_pkt);
                        break;
                    }
                    case IPC_CMD_DISPLAY_POWER:
                    {
                        if (page == 1)
                            core_draw_current(&ipc_pkt);
                        break;
                    }
                    case IPC_CMD_SET_LED:
                    {
                        core_led_ctrl(&ipc_pkt);
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
            lcd_set_cursor_pos(98);
            lcd_write_string("IE ");
            lcd_write_uint(result);
        }
        /* SW0 */
	    static bool sw0_lp_active = false;
	    static bool sw1_lp_active = false;
        if (enc_sw0_event)
        {
	        if ((PINC & (1<<PC5)) == 0)
            {
                _delay_ms(1);
	            if (enc_sw0_event && (PINC & (1<<PC5)))
	            {
		            if (!sw0_lp_active)
	                {
                        ipc_send_enc(IPC_DATA_ENC_SW0);
                        display_chg_page();
		                enc_sw0_event = 0;
		            }
		            sw0_lp_active = false;
                }
            }
            if (enc_sw0_event > 50)
	        {
                ipc_send_enc(IPC_DATA_ENC_SW0_LONGPRESS);
                enc_sw0_event = 0;
		        sw0_lp_active = true;
	        }
        }
        if (enc_sw1_event)
        {
	        if ((PINC & (1<<PC4)) == 0)
            {
                _delay_ms(1);
	            if (enc_sw1_event && (PINC & (1<<PC4)))
	            {
		            if (!sw1_lp_active)
	                {
                        ipc_send_enc(IPC_DATA_ENC_SW1);
		                enc_sw0_event = 0;
		            }
		            sw1_lp_active = false;
                }
            }
            if (enc_sw1_event > 50)
	        {
                ipc_send_enc(IPC_DATA_ENC_SW1_LONGPRESS);
                enc_sw1_event = 0;
		        sw1_lp_active = true;
	        }
        }

        /* Handle all other system events */
        if (enc_term_b_event)
        {
            ipc_send_enc(IPC_DATA_ENC_CCW);
            enc_term_b_event = 0;
        }

        if (enc_term_a_event)
        {
            ipc_send_enc(IPC_DATA_ENC_CW);
            enc_term_a_event = 0;
        }

        if (enc_longpress_event)
        {
            ipc_send_enc(IPC_DATA_ENC_LONGPRESS);
            enc_longpress_event = 0;
            enc_btn_event = 0;
        }
        bool double_click = 0;
        if (enc_btn_event && ((PIND & (1 << PD1)) == (1<<PD1)))
        {
            if (enc_db_click == 1)
            {
                ipc_send_enc(IPC_DATA_ENC_DB_BTN);
                
                enc_db_click = 2;
                double_click = 1;
                stop_db_click_timer();
            }
            if (!double_click)
                start_db_click_timer();
            enc_btn_event = 0;
        }
        if (enc_db_click == 2)
        {
            if (!double_click)
            {
                ipc_send_enc(IPC_DATA_ENC_BTN);
            }
            stop_db_click_timer();
            enc_db_click = 0;
        }
    }
    return 0; //Should never get here
}

