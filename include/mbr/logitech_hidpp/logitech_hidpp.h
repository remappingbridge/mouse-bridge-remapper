/* Adapted from accepted G06, MIT; see THIRD_PARTY.md. */
#ifndef MBR_LOGITECH_HIDPP_LOGITECH_HIDPP_H
#define MBR_LOGITECH_HIDPP_LOGITECH_HIDPP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MBR_HIDPP_LONG_PAYLOAD_SIZE 19u
#define MBR_HIDPP_REPORT_ID_SHORT 0x10u
#define MBR_HIDPP_REPORT_ID_LONG 0x11u
#define MBR_HIDPP_REPROG_CONTROLS_V4 0x1b04u
#define MBR_HIDPP_FORWARD_CID 0x0056u

typedef enum {
    MBR_HIDPP_OUTPUT_NONE = 0,
    MBR_HIDPP_OUTPUT_GET_FEATURE,
    MBR_HIDPP_OUTPUT_SET_FORWARD_DIVERT,
} mbr_hidpp_output_kind_t;

typedef struct {
    mbr_hidpp_output_kind_t kind;
    uint8_t report_id;
    uint8_t payload[MBR_HIDPP_LONG_PAYLOAD_SIZE];
    size_t payload_len;
} mbr_hidpp_output_t;

typedef struct {
    bool consumed;
    bool held_changed;
    bool forward_held;
} mbr_hidpp_input_result_t;

typedef struct {
    bool connected;
    bool forward_desired;
    bool forward_applied;
    bool forward_held;
    bool feature_failed;
    uint8_t feature_index;
    mbr_hidpp_output_kind_t waiting_for;
    bool pending_enable;
} mbr_logitech_hidpp_t;

void mbr_logitech_hidpp_init(mbr_logitech_hidpp_t *hidpp);
void mbr_logitech_hidpp_on_connect(mbr_logitech_hidpp_t *hidpp);
void mbr_logitech_hidpp_on_disconnect(mbr_logitech_hidpp_t *hidpp);
void mbr_logitech_hidpp_set_forward_desired(mbr_logitech_hidpp_t *hidpp, bool desired);
bool mbr_logitech_hidpp_next_output(mbr_logitech_hidpp_t *hidpp,
                                         mbr_hidpp_output_t *output);
void mbr_logitech_hidpp_output_result(mbr_logitech_hidpp_t *hidpp,
                                           mbr_hidpp_output_kind_t kind,
                                           bool accepted);
bool mbr_logitech_hidpp_process_input(mbr_logitech_hidpp_t *hidpp,
                                           uint8_t report_id,
                                           const uint8_t *payload,
                                           size_t payload_len,
                                           mbr_hidpp_input_result_t *result);
bool mbr_logitech_hidpp_claims_forward(const mbr_logitech_hidpp_t *hidpp);


#endif
