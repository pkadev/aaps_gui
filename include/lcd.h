#ifndef LCD_H__
#define LCD_H__

void lcd_init(void);
void write_char(char c);
void lcd_write_int(int16_t integer);
void lcd_write_uint(uint16_t uinteger);
void lcd_write_luint(uint32_t uinteger);
void lcd_write_luint_r(uint32_t uinteger, uint8_t pos, uint8_t width);
void lcd_write_string(char *str);
void lcd_set_cursor_pos(uint8_t pos);
void lcd_clr_disp(void);

#endif /* LCD_H__ */
