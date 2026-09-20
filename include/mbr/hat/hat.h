#ifndef MBR_HAT_HAT_H
#define MBR_HAT_HAT_H

#include <stdbool.h>
#include <stdint.h>

#include "mbr/domain/domain.h"

#define MBR_HAT_PIN_JOY_UP 2u
#define MBR_HAT_PIN_JOY_PRESS 3u
#define MBR_HAT_PIN_KEY_A 15u
#define MBR_HAT_PIN_JOY_LEFT 16u
#define MBR_HAT_PIN_KEY_B 17u
#define MBR_HAT_PIN_JOY_DOWN 18u
#define MBR_HAT_PIN_KEY_X 19u
#define MBR_HAT_PIN_JOY_RIGHT 20u
#define MBR_HAT_PIN_KEY_Y 21u

typedef struct {
    mbr_control_t control;
    bool pressed;
} mbr_hat_event_t;

bool mbr_hat_control_for_pin(uint8_t pin, mbr_control_t *control);
uint8_t mbr_hat_pin_for_control(mbr_control_t control);
void mbr_hat_pico_init(void);
void mbr_hat_pico_task(void);
bool mbr_hat_pico_poll_event(mbr_hat_event_t *event);

#endif