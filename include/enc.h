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
void btn_event(void);
void term_a_event(void);
void term_b_event(void);


extern uint8_t volatile enc_btn_event;
extern uint8_t volatile enc_term_a_event;
extern uint8_t volatile enc_term_b_event;
#endif
