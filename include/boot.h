#ifndef BOOT_H__
#define BOOT_H__
#include <stdint.h>
typedef enum
{
    AAPS_RET_OK,
    AAPS_ERROR_OVERFLOW,
    AAPS_RET_ERROR_GENERAL,
    AAPS_RET_ERROR_BAD_PARAMETERS,
    AAPS_RET_ERROR_OUT_OF_MEMORY,
} aaps_result_t;

struct temperature_t
{
    uint8_t whole;
    uint8_t decimal;
};

aaps_result_t boot(void);
void boot_failed(void);
#endif
