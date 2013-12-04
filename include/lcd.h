#ifndef LCD_H__
#define LCD_H__

int lcd_init(void);
void write_char(char c);
void lcd_write_string(char *str);
void lcd_set_cursor_pos(uint8_t pos);
void lcd_clr_disp(void);

#endif /* LCD_H__ */
