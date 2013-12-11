#ifndef CMD_EXEC_H__
#define CMD_EXEC_H__


#include "boot.h"
#include "ipc.h"

aaps_result_t cmd_exec_get_temp(struct ipc_packet_t *packet);
aaps_result_t cmd_exec_get_adc(struct ipc_packet_t *packet);
aaps_result_t cmd_exec_ctrl_relay(struct ipc_packet_t *packet,
                                  uint8_t relay_id);

aaps_result_t cmd_exec_display_voltage(struct ipc_packet_t *packet);

void core_draw_adc(uint8_t lsb, uint8_t msb, uint8_t type, uint8_t ch);
void core_draw_temp(struct temperature_t *temp, uint8_t sensor);
void core_init();
void core_draw_current(struct ipc_packet_t *pkt);
void core_draw_voltage(struct ipc_packet_t *pkt);
void core_draw_dac(struct ipc_packet_t *pkt);

#endif
