#include "cmd_exec.h"
#include "m48_hal.h"
#include "lcd.h"

#define THERMO_SENSOR_0     0x00
#define THERMO_SENSOR_1     0x01

void core_init()
{
    lcd_set_cursor_pos(0);
    lcd_write_string("Tc: ");
    lcd_set_cursor_pos(64);
    lcd_write_string("T0: ");
    lcd_set_cursor_pos(20);
    lcd_write_string("T1: ");
}
aaps_result_t cmd_exec_ctrl_relay(struct ipc_packet_t *packet,
                                  uint8_t relay_id)
{
    if (relay_id == RELAY_D_ID)
    {
        if (packet->data[1])
            ;//RELAY_D_SET();
        else
            ;//RELAY_D_CLR();
    }
    else if (relay_id == RELAY_ID)
    {
        if (packet->data[1])
            ;//RELAY_SET();
        else
            ;//RELAY_CLR();
    }

    return AAPS_RET_OK;
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
    if (data < 1000)
        lcd_write_string("  ");
    if (data < 100)
        lcd_write_string("    ");
    if (data < 10)
        lcd_write_string("     ");
}


void core_draw_temp(struct temperature_t *temp, uint8_t sensor)
{
    char buf[10];
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
void core_draw_voltage(struct ipc_packet_t *pkt)
{
    uint16_t data = (pkt->data[2] << 8) | pkt->data[3];
    switch(pkt->data[1])
    {
        case 0:
        lcd_set_cursor_pos(0);
          break;
        case 1:
        lcd_set_cursor_pos(64);
          break;
        case 2:
        lcd_set_cursor_pos(20);
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
    if (pkt->data[1] < 3 && pkt->data[1] >= 0)
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
        lcd_set_cursor_pos(7);
          break;
        case 1:
        lcd_set_cursor_pos(71);
          break;
        case 2:
        lcd_set_cursor_pos(27);
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
