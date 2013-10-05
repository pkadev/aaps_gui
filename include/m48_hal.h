#ifndef M48_HAL_H__
#define M48_HAL_H__
#include <avr/io.h>

/*
 * TODO: Adapt and cleanup of  macros for
 * AAPS_G - Currently it's copy paste
 * from AAPS_A.
 */

/* AVR specific defines */

/* System defines */
#define STATUS_REGISTER         SREG
#define STATUS_REGISTER_IT      SREG_I

#define RELAY_D_ID              0x00
#define RELAY_ID                0x01

/* Defines for Watchdog */
#define WD_CTRL_REG             WDTCSR
#define WD_CHANGE_ENABLE        WDCE
#define WD_IT_ENABLE_MASK       WDIE
#define WD_ENABLE               WDE
#define WD_PRESCALER2           WDP2
#define WD_PRESCALER1           WDP1
#define WD_PRESCALER0           WDP0

/* UART0 defines */
#define UART_DATA_REG           UDR0
#define UART_BAUD_RATE_REG_LOW  UBRR0L

/* SW1 Defines */
#define SW1_PORT PORTD
#define SW1_DDR DDRD
#define SW1_IN PIND
#define SW1_PIN 2
#define SW1_WAIT_UNTILL_PRESSED() while((SW1_IN & (1<<SW1_PIN))==(1<<SW1_PIN)){}

/* SW2 Defines */
#define SW2_PORT PORTB
#define SW2_DDR DDRB
#define SW2_IN PINB
#define SW2_PIN 0
#define SW2_WAIT_UNTILL_PRESSED() while((SW2_IN & (1<<SW2_PIN))==0){}


/* IRQ Defines */
#define IRQ_PORT PORTB
#define IRQ_DDR DDRB
#define IRQ_IN PINB
#define IRQ_PIN 0
#define IRQ_INIT() (IRQ_DDR|=(1<<IRQ_PIN))
#define IRQ_SET()  (IRQ_PORT|=(1<<IRQ_PIN))
#define IRQ_CLR()  (IRQ_PORT&=~(1<<IRQ_PIN))
/* End IRQ Defines */
#endif
