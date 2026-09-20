#include <stdint.h>

#include "pico/stdlib.h"

#include "mbr/hat/hat.h"
#include "mbr/output_state/output_state.h"
#include "mbr/usb_hid/usb_hid.h"

enum {
    FIXTURE_INTERVAL_MS = 1000u,
    FIXTURE_STEP_COUNT = 10u,
};

static void send_mouse(const mbr_output_state_t *state)
{
    mbr_usb_hid_mouse_report_t report;
    mbr_usb_hid_build_mouse_report(&report, state);
    (void)mbr_usb_hid_pico_send_mouse(&report);
}

static void send_keyboard(const mbr_output_state_t *state)
{
    mbr_usb_hid_keyboard_report_t report;
    mbr_usb_hid_build_keyboard_report(&report, state);
    (void)mbr_usb_hid_pico_send_keyboard(&report);
}

static void apply_fixture_step(mbr_output_state_t *state, unsigned step)
{
    state->x = 0;
    state->y = 0;
    state->wheel = 0;
    state->pan = 0;

    switch (step) {
    case 1u: state->x = -20; break;
    case 2u: state->x = 20; break;
    case 3u: state->y = -20; break;
    case 4u: state->y = 20; break;
    case 5u: state->wheel = 1; break;
    case 6u: state->wheel = -1; break;
    case 7u: state->pan = 1; break;
    case 8u: state->pan = -1; break;
    default: break;
    }
    send_mouse(state);
}

static void apply_hat_event(mbr_output_state_t *state, const mbr_hat_event_t *event)
{
    uint8_t button = 0u;

    switch (event->control) {
    case MBR_CONTROL_JOY_UP: button = MBR_OUTPUT_BUTTON_LEFT; break;
    case MBR_CONTROL_JOY_DOWN: button = MBR_OUTPUT_BUTTON_RIGHT; break;
    case MBR_CONTROL_JOY_PRESS: button = MBR_OUTPUT_BUTTON_MIDDLE; break;
    case MBR_CONTROL_KEY_X: button = MBR_OUTPUT_BUTTON_FORWARD; break;
    case MBR_CONTROL_KEY_Y: button = MBR_OUTPUT_BUTTON_BACKWARD; break;
    default: break;
    }

    if (event->control == MBR_CONTROL_KEY_A) {
        state->escape_down = event->pressed;
        send_keyboard(state);
        return;
    }

    if (button == 0u) return;
    if (event->pressed) state->buttons |= button;
    else state->buttons &= (uint8_t)~button;
    send_mouse(state);
}

int main(void)
{
    mbr_output_state_t state;
    mbr_output_state_clear(&state);

    if (!mbr_usb_hid_pico_init()) for (;;) tight_loop_contents();
    mbr_hat_pico_init();

    uint32_t last_fixture = to_ms_since_boot(get_absolute_time());
    unsigned fixture_step = 0u;

    for (;;) {
        mbr_usb_hid_pico_task();
        mbr_hat_pico_task();

        mbr_hat_event_t event;
        while (mbr_hat_pico_poll_event(&event)) {
            apply_hat_event(&state, &event);
        }

        const uint32_t now = to_ms_since_boot(get_absolute_time());
        if (mbr_usb_hid_pico_mounted() &&
            (uint32_t)(now - last_fixture) >= FIXTURE_INTERVAL_MS) {
            fixture_step = (fixture_step + 1u) % FIXTURE_STEP_COUNT;
            apply_fixture_step(&state, fixture_step);
            last_fixture = now;
        }

        tight_loop_contents();
    }
}
