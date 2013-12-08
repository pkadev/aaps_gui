#include "core.h"

void core_init_temp()
{
    lcd_set_cursor_pos(0);
    lcd_write_string("T: ");
}
void core_draw_temp(struct temperature_t *temp)
{
    char buf[3];
    itoa(temp->whole, buf, 10);
    lcd_set_cursor_pos(3);
    lcd_write_string(buf);
    lcd_write_string(".");
    itoa(temp->decimal, buf, 10);
    lcd_write_string(buf);
}
