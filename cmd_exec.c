#include "cmd_exec.h"
#include "m48_hal.h"
#include "lcd.h"
#include "enc.h"

void core_init_page(uint8_t page)
{
    if (page == 1)
    {
        //core_draw_ilimit(0);
        lcd_set_cursor_pos(0);
        lcd_write_string("       A");
        lcd_set_cursor_pos(71);
        write_char('V');
        lcd_set_cursor_pos(27);
        write_char('V');
        lcd_set_cursor_pos(91);
        write_char('V');
    }
    else if (page == 2)
    {
        lcd_set_cursor_pos(0);
        lcd_write_string("Q1   C     W CB  AM");
        lcd_set_cursor_pos(64);
        lcd_write_string("Q2   C     W");
        lcd_set_cursor_pos(20);
        lcd_write_string("R2   C     W");
        lcd_set_cursor_pos(84);
        lcd_write_string("RS   C     W     W");
    }
}

void core_led_ctrl(struct ipc_packet_t *pkt)
{
    switch(pkt->data[0])
    {
        case IPC_LED_GREEN:
            enc_gled_ctrl(pkt->data[1]);
          break;
        case IPC_LED_YELLOW:
            enc_yled_ctrl(pkt->data[1]);
          break;
        case IPC_LED_RED:
            enc_rled_ctrl(pkt->data[1]);
          break;
    }
}

void core_draw_ilimit(struct ipc_packet_t *pkt)
{
    /* Add on/off */
    lcd_set_cursor_pos(13);
    write_char(0xff);
}

void core_draw_dac(struct ipc_packet_t *pkt)
{
    uint8_t lcd_pos = 0;
    uint32_t data = (uint32_t)pkt->data[4] << 24 | (uint32_t)pkt->data[3] << 16;
    uint16_t add = (pkt->data[2] << 8) | pkt->data[1];
    data += add;


    switch(pkt->data[0])
    {
        case IPC_DATA_CURRENT:
            if (data > 999999)
                data /= 10;
            lcd_pos = 14;
          break;
        case IPC_DATA_VOLTAGE:
            if (data > 9999999)
                data /= 10;
            if (data > 999999)
                data /= 10;
        lcd_pos = 78;
          break;
    }
    lcd_set_cursor_pos(lcd_pos);
    lcd_write_luint_r(data, lcd_pos, 6);
}

void core_draw_scale(uint16_t scale)
{
    lcd_write_luint_r(scale, 101, 3);
}

void core_draw_input(uint8_t input)
{
    char inp;
    inp = input ? 'V' : 'A';
    lcd_set_cursor_pos(38);
    write_char(inp);
}
void core_draw_temp(struct temperature_t *temp, uint8_t sensor)
{
    switch (sensor)
    {
        case 0:
            /* T3 for Q2 Power transistor*/
            lcd_set_cursor_pos(67);
            break;
        case 1:
            /* T2 for Q1 Power transistor */
            lcd_set_cursor_pos(3);
            break;
        case 2:
            /* T4 for R22 Power transistor */
            lcd_set_cursor_pos(23);
            break;
        case 3:
            /* T5 for RS current sense resistor */
            lcd_set_cursor_pos(87);
            break;
        case 4:
            /* T6 for CB current sense resistor */
            lcd_set_cursor_pos(77);
            break;
        case 5:
            /* T1 for ambient on AAPS_D */
            lcd_set_cursor_pos(81);
            break;
    }
    lcd_write_uint(temp->whole);
    //lcd_write_string(".");
    //lcd_write_uint(temp->decimal);
}

void core_draw_current(struct ipc_packet_t *pkt)
{
    uint32_t data = (uint32_t)pkt->data[4] << 16;
    uint16_t add = (pkt->data[2] << 8) | pkt->data[3];
    data += add;
    if (pkt->data[1] == 2)
    {
        lcd_write_luint_r(data, 0, 7);
    }
}

void core_draw_voltage(struct ipc_packet_t *pkt)
{
    uint8_t lcd_pos = 0;
    uint32_t data = (uint32_t)pkt->data[4] << 16;
    uint16_t add = (pkt->data[3] << 8) | pkt->data[2];
    data += add;

    switch(pkt->data[1])
    {
        case 0:
        lcd_pos = 20;
          break;
        case 1:
        lcd_pos = 64;
          break;
        case 3:
            lcd_pos = 84;
          break;
    }
    if (pkt->data[1] == 0 || pkt->data[1] == 1 ||
        pkt->data[1] == 3)
    {
        lcd_write_luint_r(data, lcd_pos, 7);
    }
}

/* TODO: Use ipc_packet as input parameter */
void core_draw_adc(uint8_t msb, uint8_t lsb, uint8_t type, uint8_t ch)
{
    uint8_t lcd_pos = 0;
    switch(ch)
    {
        case 0:
            lcd_pos = 28;
          break;
        case 1:
            lcd_pos = 72;
          break;
        case 2:
           lcd_pos = 8;
          break;
        case 3:
            lcd_pos = 92;
          break;
    }
    if (ch <= 3 && ch >= 0)
    {
        uint16_t data = msb << 8 | lsb;
        lcd_write_luint_r(data, lcd_pos, 5);
    }
}

void core_draw_power(struct ipc_packet_t *pkt)
{
    uint8_t lcd_pos = 0;
    uint8_t ch = pkt->data[0];
    uint32_t data = (uint32_t)pkt->data[3] << 16;
    uint16_t add = (pkt->data[2] << 8) | pkt->data[1];
    data += add;
    while (data > 99999)
        data /= 10;

    switch(ch)
    {
        case 0:
           lcd_pos = 6;
          break;
        case 1:
            lcd_pos = 70;
          break;
        case 2:
            lcd_pos = 26;
          break;
        case 3:
            lcd_pos = 90;
          break;
        case 4:
            lcd_pos = 96;
          break;
    }
    lcd_write_luint_r(data, lcd_pos, 5);
}
