#ifndef MBR_OUTPUT_STATE_OUTPUT_STATE_H
#define MBR_OUTPUT_STATE_OUTPUT_STATE_H

#include <stdbool.h>
#include <stdint.h>

enum {
    MBR_OUTPUT_BUTTON_LEFT = 1u << 0,
    MBR_OUTPUT_BUTTON_RIGHT = 1u << 1,
    MBR_OUTPUT_BUTTON_MIDDLE = 1u << 2,
    MBR_OUTPUT_BUTTON_FORWARD = 1u << 3,
    MBR_OUTPUT_BUTTON_BACKWARD = 1u << 4,
};

typedef struct {
    uint8_t buttons;
    int16_t x;
    int16_t y;
    int16_t wheel;
    int16_t pan;
    bool escape_down;
} mbr_output_state_t;

void mbr_output_state_clear(mbr_output_state_t *state);

#endif
