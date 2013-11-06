#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "enc.h"
#include "ipc.h"

volatile uint8_t irq_event = 0;
volatile uint16_t enc_pos = 0;
static uint8_t btn_status = 0;
uint16_t get_enc_pos(void)
{
    return enc_pos;
}

ISR(PCINT2_vect)
{
    if ((PIND & (1 << PD1)) == 0)
    {
       /*
        * TODO: Remove this and use timers to detect
        * when the button is pushed. Also att electric
        * debouncing on the button.
        */
        _delay_ms(20);

        if ((PIND & (1 << PD1)) == 0)
        {
            print_ipc_int("[G] btn: ", btn_status);
            switch(btn_status)
            {
                case 0:
                    enc_gled_on();
                    btn_status = 1;
                break;
                case 1:
                    enc_rled_on();
                    btn_status = 2;
                break;
                case 2:
                    enc_gled_off();
                    btn_status = 3;
                break;
                case 3:
                    enc_gled_off();
                    enc_rled_off();
                    btn_status = 0;
                break;
            }
        }
    }
}

ISR(INT0_vect) /* Encoder TERM A */
{
    if (((PIND & (1<<PD3)) == (1 << PD3)))
    {
        if (btn_status == 1)
            enc_pos += 10;
        else if (btn_status == 2)
            enc_pos += 50;
        else if (btn_status == 3)
            enc_pos += 100;
        else
            enc_pos++;
    }
}

ISR(INT1_vect) /* Encoder TERM B */
{
    if (((PIND & (1<<PD2)) == (1 << PD2)))
    {
        if (btn_status == 1)
           enc_pos-=10;
        else if (btn_status == 2)
            enc_pos -= 50;
        else if (btn_status == 3)
            enc_pos -= 100;
        else
           enc_pos--;
    }

}

static void enable_ext_irq()
{
    EICRA |= (1<<ISC01) | (1<<ISC11);        //IRQ on falling edge
    EIMSK |= (1<<INT0) | (1<<INT1);
}

void enc_init(void)
{
    DDRB |= (1 << PB6);
    DDRB |= (1 << PB7);

    /* Enable PCINT17 for encoder button */
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT17);

    /* Enable external IRQ for encoder TERM-A and TERM-B. */
    enable_ext_irq();

}

void enc_rled_off(void)
{
    PORTB &= ~(1 << PB6);
}
void enc_rled_on(void)
{
    PORTB |= (1 << PB6);
}
void enc_gled_toggle(void)
{
    PORTB ^= (1 << PB7);
}
void enc_gled_off(void)
{
    PORTB &= ~(1 << PB7);
}
void enc_gled_on(void)
{
    PORTB |= (1 << PB7);
}
