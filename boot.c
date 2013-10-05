#include <avr/io.h>
#include <util/delay.h>
#include "m48_hal.h"
#include "boot.h"

static void enable_ext_irq();
static void enable_pcint18();
static void enable_pcint0();

/* These two should probably go somewhere else */
static void spi_init(void);
/***********************************************/
aaps_result_t boot(void)
{
    /* Enable global interrupts */
    STATUS_REGISTER |= (1<<STATUS_REGISTER_IT);

    IRQ_INIT();

    enable_ext_irq();
    if(0)
    {
        enable_pcint18();
        enable_pcint0();
    }
    spi_init();
    aaps_result_t ret = AAPS_RET_OK;
    /*
     * TODO: Change this to read settings. Or check if Settings
     * should be applied on startup. This is still copy paste
     * from AAPS_A.
     */

    return ret;
}
static void spi_init(void)
{
    DDRB |= (1<<PB3); //This is MOSI as output?? Remove??
    SPCR = (1<<SPIE) | (1<<SPE) | (1<<CPOL);
    SPSR = (1<<SPI2X);
    /* Do we need to set MISO as output? */
    DDRB |= (1<<PB4);
}


void boot_failed(void)
{
    while(1) {
        //LED_TOGGLE();
        _delay_ms(250);
    }
}
static void enable_ext_irq()
{
    EICRA |= (1<<ISC01) | (1<<ISC10);        //IRQ on falling edge
    EIMSK |= (1<<INT0) | (1<<INT1);
}

static void enable_pcint18()
{
    /* PIN change IRQ */
    //PCICR |= (1<<PCIE2);      //Enable PCINT2 (PCINT23..16)
    //PCMSK2 |= (1<<PCINT18);   //Enable PCINT18
    /* End PIN change IRQ */
}

static void enable_pcint0()
{
    /* PIN change IRQ */
    PCICR |= (1<<PCIE0);      //Enable PCINT0 (PCINT7..0)
    PCMSK0 |= (1<<PCINT0);   //Enable PCINT0 - Don't run this when pin is configured as IRQ
    /* End PIN change IRQ */
}
