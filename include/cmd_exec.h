#ifndef CMD_EXEC_H__
#define CMD_EXEC_H__


#include "boot.h"
#include "ipc.h"

void core_draw_adc(uint8_t lsb, uint8_t msb, uint8_t type, uint8_t ch);
void core_draw_temp(struct temperature_t *temp, uint8_t sensor);
void core_init_page(uint8_t page);
void core_draw_current(struct ipc_packet_t *pkt);
void core_draw_voltage(struct ipc_packet_t *pkt);
void core_draw_dac(struct ipc_packet_t *pkt);
void core_draw_power(struct ipc_packet_t *pkt);
void core_draw_ilimit(struct ipc_packet_t *pkt);
void core_led_ctrl(struct ipc_packet_t *pkt);
aaps_result_t core_send_periph_info(void);

#endif
