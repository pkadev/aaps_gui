#ifndef IPC_H__
#define IPC_H__
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "boot.h"

#define IPC_RX_BUF_LEN 62
#define IPC_DATA_LEN 2
/* IPC Commands */
enum ipc_command_t
{
    IPC_CMD_SUPPORTED_CMDS,
    IPC_CMD_PERIPH_DETECT,
    IPC_CMD_NUM_OF_CMDS,
    IPC_CMD_GET_TEMP,
    IPC_CMD_GET_ADC,
    IPC_CMD_DATA_AVAILABLE,
    IPC_CMD_SET_VOLTAGE,
    IPC_CMD_SET_CURRENT_LIMIT,
    IPC_CMD_SET_RELAY_D,
    IPC_CMD_SET_RELAY,
    IPC_CMD_PUT_DATA,
};

enum ipc_data_type_t
{
    IPC_DATA_VOLTAGE = 0x7B,
    IPC_DATA_CURRENT = 0x7C,
    IPC_DATA_ASCII = 0x7D,
    IPC_DATA_THERMO = 0x7E,
    IPC_DATA_ENC = 0x7F,
};

struct ipc_packet_t
{
    uint8_t len;
    uint8_t cmd;
    uint8_t crc;
    uint8_t data[IPC_DATA_LEN];
};

typedef enum
{
    IPC_RET_OK,
    IPC_RET_ERROR_GENERAL,
    IPC_RET_TX_BUF_EMPTY,
} ipc_result_t;

/* IPC primitives functions */
void ipc_init(void);
ipc_result_t ipc_transfer();

/* TODO: Move to "business unit layer" ? */
void ipc_send_enc(uint16_t enc_value);
#endif
