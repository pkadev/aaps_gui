#include "cmd_exec.h"
#include "m48_hal.h"
#include "lcd.h"
#include "enc.h"
#define THERMO_SENSOR_0     0x00
#define THERMO_SENSOR_1     0x01

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
        lcd_write_string("RS   C     W");
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
        lcd_set_cursor_pos(13);
        write_char(0xff);
}

void core_draw_dac(struct ipc_packet_t *pkt)
{
    uint16_t data = (pkt->data[1] << 8) | pkt->data[2];
    switch(pkt->data[0])
    {
        case IPC_DATA_CURRENT:
        lcd_set_cursor_pos(14);
          break;
        case IPC_DATA_VOLTAGE:
        lcd_set_cursor_pos(78);
          break;
    }
    lcd_write_uint(data);
    if (data < 10000)
        write_char(' ');
    else if (data < 1000)
        lcd_write_string(" ");
    else if (data < 100)
        lcd_write_string(" ");
    else if (data < 10)
        lcd_write_string(" ");
}


void core_draw_temp(struct temperature_t *temp, uint8_t sensor)
{
    switch (sensor)
    {
        case 0:
            /* T1 for Ambient */
            lcd_set_cursor_pos(81);
            break;
        case 1:
            /* T2 for Q1 Power transistor */
            lcd_set_cursor_pos(67);
            break;
        case 2:
            /* T3 for Q2 Power transistor */
            lcd_set_cursor_pos(23);
            break;
        case 3:
            /* T4 for R22 emitter resistor */
            lcd_set_cursor_pos(87);
            break;
        case 4:
            /* T5 for Rs current sense resistor */
            lcd_set_cursor_pos(95);
            break;
        case 5:
            /* T6 for circuit board */
            lcd_set_cursor_pos(77);
            break;
    }
    lcd_write_uint(temp->whole);
    //lcd_write_string(".");
    //lcd_write_uint(temp->decimal);
}

void core_draw_current(struct ipc_packet_t *pkt)
{
    uint16_t data = (pkt->data[2] << 8) | pkt->data[3];
    lcd_set_cursor_pos(0);
    if (pkt->data[1] == 2)
    {
        lcd_write_uint(data);
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
void core_draw_voltage(struct ipc_packet_t *pkt)
{
    uint16_t data = (pkt->data[2] << 8) | pkt->data[3];
    switch(pkt->data[1])
    {
        case 0:
        lcd_set_cursor_pos(20);
          break;
        case 1:
        lcd_set_cursor_pos(64);
          break;
       // case 2:
       // //lcd_set_cursor_pos(20);
       //   break;
       // case 3:
       // lcd_set_cursor_pos(10);
       //   break;
       // case 4:
       // lcd_set_cursor_pos(10);
       //   break;
       // case 5:
       // lcd_set_cursor_pos(10);
       //   break;
       // case 6:
       // lcd_set_cursor_pos(10);
       //   break;
       // case 7:
       // lcd_set_cursor_pos(10);
       //   break;
    }
    if (pkt->data[1] < 2 && pkt->data[1] >= 0)
    {
        lcd_write_uint(data);
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
/* TODO: Use ipc_packet as input parameter */
void core_draw_adc(uint8_t msb, uint8_t lsb, uint8_t type, uint8_t ch)
{
    char buf[10];
    switch(ch)
    {
        case 0:
        lcd_set_cursor_pos(28);
          break;
        case 1:
        lcd_set_cursor_pos(72);
          break;
        case 2:
        lcd_set_cursor_pos(8);
          break;
        //case 3:
        //lcd_set_cursor_pos(10);
        //  break;
        //case 4:
        //lcd_set_cursor_pos(10);
        //  break;
        //case 5:
        //lcd_set_cursor_pos(10);
        //  break;
        //case 6:
        //lcd_set_cursor_pos(10);
        //  break;
        //case 7:
        //lcd_set_cursor_pos(10);
        //  break;
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
