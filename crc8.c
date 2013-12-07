#include "crc8.h"
#include "lcd.h"

#define CRC8INIT    0x00
#define CRC8POLY    0x18    //0X18 = X^8+X^5+X^4+X^0

uint8_t crc8(uint8_t *data_in, uint8_t size)
{
    uint8_t  crc, bit_counter, data, feedback_bit;
    crc = CRC8INIT;
    for (uint8_t i = 0; i != size; i++) {
        data = data_in[i];

        bit_counter = 8;
        do {
            feedback_bit = (crc ^ data) & 0x01;
            if ( feedback_bit == 0x01 ) {
                crc = crc ^ CRC8POLY;
            }
            crc = (crc >> 1) & 0x7F;
            if ( feedback_bit == 0x01 ) {
                crc = crc | 0x80;
            }
            data = data >> 1;
            bit_counter--;
        } while (bit_counter > 0);
    }
    return crc;
}
