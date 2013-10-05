#include "cmd_exec.h"
#include "m48_hal.h"

#define THERMO_SENSOR_0     0x00
#define THERMO_SENSOR_1     0x01

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
