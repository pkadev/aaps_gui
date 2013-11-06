#ifndef ENC_H__
#define ENC_H__

#include <stdio.h>
#include <stdint.h>

void enc_init(void);
void enc_rled_off(void);
void enc_rled_on(void);
void enc_gled_toggle(void);
void enc_gled_off(void);
void enc_gled_on(void);
uint16_t get_enc_pos(void);
#endif
