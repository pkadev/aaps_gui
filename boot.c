#include <avr/io.h>
#include <util/delay.h>
#include "m48_hal.h"
#include "boot.h"
#include "enc.h"
#include "lcd.h"

static void enable_pcint18();
static void enable_pcint2();
static void enable_pcint0();

/* These two should probably go somewhere else */
static void spi_init(void);
/***********************************************/
aaps_result_t boot(void)
{
    /* Enable global interrupts */
    STATUS_REGISTER |= (1<<STATUS_REGISTER_IT);

    IRQ_INIT();
    enable_pcint2();
    if(0)
    {
        enable_pcint18();
        enable_pcint0();
    }
    enc_init();
    spi_init();
    lcd_init();
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
    //DDRB |= (1<<PB3); //This is MOSI as output?? Remove??
    SPCR = (1<<SPE) | (1<<CPOL);
    /* MISO as output */
    DDRB |= (1<<PB4);
}


void boot_failed(void)
{
    while(1) {
        //LED_TOGGLE();
        _delay_ms(250);
    }
}
static void enable_pcint2()
{
    /* IRQ for SS on SPI to detect when SPI is busy */
    /* PIN change IRQ */
    PCICR |= (1<<PCIE0);      //Enable PCINT0 (PCINT0..7)
    PCMSK0 |= (1<<PCINT2);   //Enable PCINT2
    /* End PIN change IRQ */
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
