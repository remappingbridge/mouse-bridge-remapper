#include "mbr/output_state/output_state.h"

#include <limits.h>
#include <string.h>

static int16_t saturating_add(int16_t current, int16_t delta)
{
    const int32_t sum = (int32_t)current + (int32_t)delta;
    if (sum > INT16_MAX) return INT16_MAX;
    if (sum < INT16_MIN) return INT16_MIN;
    return (int16_t)sum;
}

static bool consume_component(int16_t *pending, int16_t value)
{
    if (pending == NULL || value == 0) return true;
    if (*pending == 0) return false;
    if ((*pending > 0 && value < 0) || (*pending < 0 && value > 0)) return false;
    if ((uint16_t)(*pending > 0 ? *pending : -*pending) <
        (uint16_t)(value > 0 ? value : -value)) return false;
    *pending = (int16_t)(*pending - value);
    return true;
}

void mbr_output_state_clear(mbr_output_state_t *state)
{
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
}

bool mbr_output_state_apply_mouse_event(mbr_output_state_t *state,
                                        const mbr_mouse_event_t *event)
{
    if (state == NULL || event == NULL) return false;

    switch (event->type) {
    case MBR_MOUSE_EVENT_BUTTON: {
        uint8_t mask = 0u;
        switch (event->data.button.button) {
        case MBR_MOUSE_BUTTON_LEFT: mask = MBR_OUTPUT_BUTTON_LEFT; break;
        case MBR_MOUSE_BUTTON_RIGHT: mask = MBR_OUTPUT_BUTTON_RIGHT; break;
        case MBR_MOUSE_BUTTON_MIDDLE: mask = MBR_OUTPUT_BUTTON_MIDDLE; break;
        case MBR_MOUSE_BUTTON_FORWARD: mask = MBR_OUTPUT_BUTTON_FORWARD; break;
        case MBR_MOUSE_BUTTON_BACKWARD: mask = MBR_OUTPUT_BUTTON_BACKWARD; break;
        default: return false;
        }
        if (event->data.button.pressed) state->buttons = (uint8_t)(state->buttons | mask);
        else state->buttons = (uint8_t)(state->buttons & (uint8_t)~mask);
        return true;
    }
    case MBR_MOUSE_EVENT_MOVE:
        state->x = saturating_add(state->x, event->data.move.dx);
        state->y = saturating_add(state->y, event->data.move.dy);
        return true;
    case MBR_MOUSE_EVENT_WHEEL:
        state->wheel = saturating_add(state->wheel, event->data.wheel.vertical);
        state->pan = saturating_add(state->pan, event->data.wheel.horizontal);
        return true;
    default:
        return false;
    }
}

void mbr_output_state_release_all(mbr_output_state_t *state)
{
    if (state == NULL) return;
    state->buttons = 0u;
    state->escape_down = false;
}

bool mbr_output_state_consume_relative(mbr_output_state_t *state,
                                       int16_t x,
                                       int16_t y,
                                       int16_t wheel,
                                       int16_t pan)
{
    if (state == NULL) return false;
    if (!consume_component(&state->x, x) ||
        !consume_component(&state->y, y) ||
        !consume_component(&state->wheel, wheel) ||
        !consume_component(&state->pan, pan)) return false;
    return true;
}
