#ifndef MBR_BLE_HOGP_BLE_HOGP_H
#define MBR_BLE_HOGP_BLE_HOGP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mbr/bt_runtime/bt_runtime.h"
#include "mbr/domain/domain.h"

#define MBR_BLE_HOGP_MAX_FIELDS 48u
#define MBR_BLE_HOGP_MAX_REPORTS 16u
#define MBR_BLE_HOGP_RUNTIME_CHANNEL UINT16_C(0x0501)
#define MBR_BLE_HOGP_NAME_CAPACITY 22u

typedef enum {
    MBR_BLE_HOGP_MESSAGE_CONNECTED = 1,
    MBR_BLE_HOGP_MESSAGE_DISCONNECTED = 2,
    MBR_BLE_HOGP_MESSAGE_MOUSE = 3
} mbr_ble_hogp_message_type_t;

typedef enum {
    MBR_BLE_HOGP_FIELD_BUTTON = 0,
    MBR_BLE_HOGP_FIELD_X,
    MBR_BLE_HOGP_FIELD_Y,
    MBR_BLE_HOGP_FIELD_WHEEL,
    MBR_BLE_HOGP_FIELD_PAN
} mbr_ble_hogp_field_kind_t;

typedef struct {
    uint8_t report_id;
    mbr_ble_hogp_field_kind_t kind;
    uint16_t bit_offset;
    uint8_t bit_size;
    uint8_t button_index;
    bool signed_value;
} mbr_ble_hogp_field_t;

typedef struct {
    uint8_t report_id;
    uint16_t input_bits;
    uint8_t button_mask;
} mbr_ble_hogp_report_state_t;

typedef struct {
    mbr_ble_hogp_field_t fields[MBR_BLE_HOGP_MAX_FIELDS];
    size_t field_count;
    mbr_ble_hogp_report_state_t reports[MBR_BLE_HOGP_MAX_REPORTS];
    size_t report_count;
    uint8_t aggregate_buttons;
    bool configured;
} mbr_ble_hogp_parser_t;

typedef bool (*mbr_ble_hogp_emit_fn)(void *context,
                                     const mbr_mouse_event_t *event);

typedef struct {
    uint8_t address_type;
    uint8_t address[6];
} mbr_ble_hogp_peer_t;

typedef enum {
    MBR_BLE_HOGP_EVENT_CONNECTED = 0,
    MBR_BLE_HOGP_EVENT_DISCONNECTED,
    MBR_BLE_HOGP_EVENT_MOUSE
} mbr_ble_hogp_event_type_t;

typedef struct {
    mbr_ble_hogp_event_type_t type;
    mbr_ble_hogp_peer_t peer;
    mbr_mouse_session_id_t session_id;
    char name[MBR_BLE_HOGP_NAME_CAPACITY];
    mbr_mouse_event_t mouse;
} mbr_ble_hogp_event_t;

_Static_assert(sizeof(mbr_ble_hogp_event_t) <=
               MBR_BT_RUNTIME_MESSAGE_PAYLOAD_SIZE,
               "BLE HOGP event must fit runtime queue payload");

bool mbr_ble_hogp_parser_configure(mbr_ble_hogp_parser_t *parser,
                                    const uint8_t *descriptor,
                                    size_t descriptor_len);
bool mbr_ble_hogp_parser_has_mouse(const mbr_ble_hogp_parser_t *parser);
bool mbr_ble_hogp_parser_normalize_report(const mbr_ble_hogp_parser_t *parser,
                                           uint8_t report_id,
                                           const uint8_t *report,
                                           size_t report_len,
                                           const uint8_t **payload,
                                           size_t *payload_len);
bool mbr_ble_hogp_parser_parse_report(mbr_ble_hogp_parser_t *parser,
                                       uint8_t report_id,
                                       const uint8_t *report,
                                       size_t report_len,
                                       mbr_ble_hogp_emit_fn emit,
                                       void *context);
bool mbr_ble_hogp_decode_runtime_message(
    const mbr_bt_runtime_message_t *message,
    mbr_ble_hogp_event_t *event);
bool mbr_ble_hogp_start(void);

#endif
