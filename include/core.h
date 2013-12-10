#ifndef CORE_H__
#define CORE_H__

#include <stdint.h>
#include <stdlib.h>
#include "lcd.h"

typedef enum
{
    AAPS_RET_OK,
    AAPS_ERROR_OVERFLOW,
    AAPS_RET_ERROR_GENERAL,
    AAPS_RET_ERROR_BAD_PARAMETERS,
} aaps_result_t;

struct temperature_t
{
    uint8_t whole;
    uint8_t decimal;
};

void core_draw_adc(uint8_t lsb, uint8_t msb, uint8_t type, uint8_t ch);
void core_draw_temp(struct temperature_t *temp, uint8_t sensor);
void core_init();

#endif

