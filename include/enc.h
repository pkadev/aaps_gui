#ifndef ENC_H__
#define ENC_H__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

void enc_init(void);
uint16_t get_enc_pos(void);
void btn_event(void);
void term_a_event(void);
void term_b_event(void);

void enc_gled_ctrl(bool on);
void enc_yled_ctrl(bool on);
void enc_rled_ctrl(bool on);

void start_db_click_timer(void);
void stop_db_click_timer(void);

extern uint8_t volatile enc_sw0_event;
extern uint8_t volatile enc_sw1_event;
extern uint8_t volatile enc_btn_event;
extern uint8_t volatile enc_term_a_event;
extern uint8_t volatile enc_term_b_event;
extern uint8_t volatile enc_longpress_event;
extern uint8_t volatile enc_db_click;
#endif
