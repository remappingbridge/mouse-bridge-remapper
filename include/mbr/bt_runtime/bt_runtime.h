#ifndef MBR_BT_RUNTIME_BT_RUNTIME_H
#define MBR_BT_RUNTIME_BT_RUNTIME_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBR_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE 80u
#define MBR_BT_RUNTIME_QUEUE_CAPACITY 128u

typedef struct {
    uint16_t channel;
    uint16_t type;
    uint16_t length;
    uint8_t payload[MBR_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE];
} mbr_bt_runtime_message_t;

typedef void (*mbr_bt_runtime_session_setup_fn)(void);

void mbr_bt_runtime_reset(void);
bool mbr_bt_runtime_publish(uint16_t channel,
                            uint16_t type,
                            const void *payload,
                            uint16_t length);
bool mbr_bt_runtime_poll(mbr_bt_runtime_message_t *message);
bool mbr_bt_runtime_take_overflow(void);
bool mbr_bt_runtime_start(mbr_bt_runtime_session_setup_fn session_setup);

#ifdef __cplusplus
}
#endif

#endif
