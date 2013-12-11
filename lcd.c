#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/atomic.h>
#include "lcd.h"
/*
 * aaps_g.c
 *
 * Created: 2013-08-01 15:08:04
 *  Author: Ke
 */

//LCD interface definitions
#define LCD_DDR         (DDRD)
#define LCD_PORT        (PORTD)
#define LCD_PIN         (PIND)

#define LCD_REG         (PB0)
#define LCD_RW          (PC1)
#define LCD_E           (PC2)
#define LCD_BACK        (PB1)

#define LCD_DB4         (PD4)
#define LCD_DB5         (PD5)
#define LCD_DB6         (PD6)
#define LCD_DB7         (PD7)


//GUI interface definitions
#define LIGHT_SENS      (PC0)
#define LEDR_G          (PB6)   //Green LED
#define LEDR_R          (PB7)   //Red LED
#define SW0             (PC5)
#define SW1             (PC4)
#define SW2_IRQ_LOCAL   (PC3)
#define SWR             (PD1)
#define TERM_A          (PD2)
#define TERM_B          (PD3)

// Macros LCD
#define LCD_BACK_ON()       DDRB |= (1<<LCD_BACK); (PORTB|=(1<<LCD_BACK))
#define LCD_BACK_OFF()      DDRB |= (1<<LCD_BACK); (PORTB&=~(1<<LCD_BACK))
#define LCD_E_SET()         DDRC |= (1<<LCD_E); (PORTC|=(1<<LCD_E))
#define LCD_E_CLR()         DDRC |= (1<<LCD_E); (PORTC&=~(1<<LCD_E))
#define LCD_REG_SET()       DDRB |= (1<<LCD_REG); (PORTB|=(1<<LCD_REG))
#define LCD_REG_CLR()       DDRB |= (1<<LCD_REG); (PORTB&=~(1<<LCD_REG))
#define LCD_RW_READ()       DDRC |= (1<<LCD_RW); (PORTC|=(1<<LCD_RW))
#define LCD_RW_WRITE()      DDRC |= (1<<LCD_RW); (PORTC&=~(1<<LCD_RW))

#define LCD_CMD_SET_DRAM_ADDR 0x80
#define LCD_CMD_CLD_DISP 0x01

#define TIMING_PORT PORTC
#define TIMING_DDR DDRC
#define TIMING_BIT PC5
#define TIMING_HIGH() TIMING_PORT|=(1<<TIMING_BIT)
#define TIMING_LOW() TIMING_PORT&=~(1<<TIMING_BIT)
static void bit_write(uint8_t _data)
{
    if (_data & 0x80)
        LCD_PORT |= (0x80);
    else 
        LCD_PORT &= ~(0x80);

    if (_data & 0x40)
        LCD_PORT |= (0x40);
    else
        LCD_PORT &= ~(0x40);

    if (_data & 0x20)
        LCD_PORT |= (0x20);
    else
        LCD_PORT &= ~(0x20);

    if (_data & 0x10)
        LCD_PORT |= (0x10);
    else
        LCD_PORT &= ~(0x10);

}
static void lcd_write4(uint8_t data)
{

    /* High Nibble */
    bit_write(data);
    LCD_E_SET();
    _delay_us(1);
    LCD_E_CLR();
    /* Low Nibble */
    bit_write(data << 4);
    LCD_E_SET();
    _delay_us(1);
    LCD_E_CLR();
}

void lcd_set_cursor_pos(uint8_t pos)
{
    /*
     * TODO: Set cursor does not work well
     * with other lines tha the first
     */
    LCD_REG_CLR();
    pos |= LCD_CMD_SET_DRAM_ADDR;
    lcd_write4(pos);
    _delay_ms(5);
}

void lcd_clr_disp(void) //Clear all charachters on the LCD. Cursor will be moved to origo.
{
    LCD_REG_CLR();
    lcd_write4(LCD_CMD_CLD_DISP);
    _delay_ms(5);
}

void write_char(char c)  //Write character to LCD (ASCII)
{
    LCD_REG_SET(); //RS=1
    lcd_write4(c);
    _delay_us(100);
}


void lcd_write_string(char *str)
{
    while(*str)
    {
        write_char(*str);
        str++;
    }
}
void lcd_write_int(int16_t integer)
{
    char tmp[10];
    itoa(integer, tmp, 10);
    lcd_write_string(tmp);
}

void lcd_write_uint(uint16_t uinteger)
{
    char tmp[10];
    utoa(uinteger, tmp, 10);
    lcd_write_string(tmp);
}
void lcd_init(void)
{
    TIMING_DDR |= (1<<TIMING_BIT);
    TIMING_HIGH();
    //init LCD
    LCD_DDR |= (1<<LCD_DB7) | (1<<LCD_DB6) | (1<<LCD_DB5) | (1<<LCD_DB4);
    LCD_RW_WRITE(); //Write to LCD
    LCD_REG_CLR(); //RS=0 gives instructions

    /* First byte */
    LCD_PORT = 0x30;
    LCD_E_SET();
    _delay_us(1);
    LCD_E_CLR();
    _delay_ms(5);

    /* Second byte */
    LCD_PORT = 0x30;
    LCD_E_SET();
    _delay_us(1);
    LCD_E_CLR();
    _delay_ms(1);

    /* Third byte */
    LCD_PORT = 0x30;
    LCD_E_SET();
    _delay_us(1);
    LCD_E_CLR();
    _delay_ms(1);

    //4th byte (after this byte the data must be sent as nibble pairs)
    LCD_PORT =0x20; //4-bit interface
    LCD_E_SET();
    _delay_us(1);
    LCD_E_CLR();
    _delay_ms(1);

    /*
    RS R/W DB7 DB6 DB5 DB4
    0 0 0 0 1 0
    0 0 N F x x

    N=Number of `lines'
    0 for 1/8 duty cycle -- 1 `line'
    1 for 1/16 duty cycle -- 2 `lines'
    F=font
    1 for 5x11 dot matrix
    0 for 5x8 dot matrix
    */

    //5th byte
    lcd_write4(0x48);
    _delay_ms(1);

    //6th byte (Display off, cursor off, blink off)
    lcd_write4(0x0c);
    _delay_ms(1);

    //7th byte (Clear screen, cursor home)
    lcd_write4(0x01);
    _delay_ms(1);

    //8th byte (Increment cursor to the right)
    //when writing, don't shift screen
    lcd_write4(0x06);
    _delay_ms(1);
    //End Init LCD

    lcd_clr_disp();
}




