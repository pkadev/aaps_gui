#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "enc.h"
#include "ipc.h"

uint8_t volatile enc_btn_event = 0;
uint8_t volatile enc_longpress_event = 0;
uint8_t volatile enc_term_a_event = 0;
uint8_t volatile enc_term_b_event = 0;
volatile uint8_t irq_event = 0;
static volatile uint8_t longpress_cnt = 0;
static volatile uint8_t btn_cnt = 0;
static volatile uint8_t btn_up = 1;
ISR(PCINT2_vect)
{
    if ((PIND & (1 << PD1)) == 0)
            enc_btn_event = 1;
}

ISR(INT0_vect) /* Encoder TERM A */
{
    if (((PIND & (1<<PD3)) == (1 << PD3)))
        enc_term_a_event = 1;
}
ISR(INT1_vect) /* Encoder TERM B */
{
    if (((PIND & (1<<PD2)) == (1 << PD2)))
        enc_term_b_event = 1;
}

ISR(TIMER0_OVF_vect)
{
    if ((PIND & (1 << PD1)) == 0)
    {
        if (longpress_cnt++ > 60)
        {
            enc_longpress_event = 1;
            longpress_cnt = 0;
        }
    }
    else
        longpress_cnt = 0;
}

void timer_init(void)
{
    TCCR0B |= (1 << CS02) | (1 << CS00);
    TIMSK0 = (1<<TOIE0);
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
    timer_init();
}

void enc_rled_ctrl(bool on)
{
    if (on)
        PORTB |= (1 << PB7);
    else
        PORTB &= ~(1 << PB7);
}

void enc_gled_ctrl(bool on)
{
    if (on)
        PORTB |= (1 << PB6);
    else
        PORTB &= ~(1 << PB6);
}

void enc_yled_ctrl(bool on)
{
    enc_gled_ctrl(on);
    enc_rled_ctrl(on);
}
