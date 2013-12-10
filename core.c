#include "core.h"
#include <string.h>

void core_init()
{
    lcd_set_cursor_pos(0);
    lcd_write_string("Tc: ");
    lcd_set_cursor_pos(64);
    lcd_write_string("T0: ");
    lcd_set_cursor_pos(20);
    lcd_write_string("T1: ");
}


void core_draw_temp(struct temperature_t *temp, uint8_t sensor)
{
    char buf[3];
    itoa(temp->whole, buf, 10);

    switch (sensor)
    {
        case 0:
            lcd_set_cursor_pos(3);
            break;
        case 1:
            lcd_set_cursor_pos(67);
            break;
        case 2:
            lcd_set_cursor_pos(23);
            break;
    }
    lcd_write_string(buf);
    lcd_write_string(".");
    itoa(temp->decimal, buf, 10);
    lcd_write_string(buf);
}
void core_draw_adc(uint8_t msb, uint8_t lsb, uint8_t type, uint8_t ch)
{
    char buf[6];
    switch(ch)
    {
        case 0:
        lcd_set_cursor_pos(10);
          break;
        case 1:
        lcd_set_cursor_pos(74);
          break;
        case 2:
        lcd_set_cursor_pos(30);
          break;
        case 3:
        lcd_set_cursor_pos(10);
          break;
        case 4:
        lcd_set_cursor_pos(10);
          break;
        case 5:
        lcd_set_cursor_pos(10);
          break;
        case 6:
        lcd_set_cursor_pos(10);
          break;
        case 7:
        lcd_set_cursor_pos(10);
          break;
    }
    if (ch < 3 && ch >= 0)
    {
        uint16_t data = msb << 8 | lsb;
        utoa(data, buf, 10);
        lcd_write_string(buf);
        if (data < 10000)
            write_char(' ');
        if (data < 1000)
            write_char(' ');
        if (data < 100)
            write_char(' ');
        if (data < 10)
            write_char(' ');
    }
}
