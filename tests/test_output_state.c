#include <stdio.h>
#include <stdlib.h>

#include "mbr/output_state/output_state.h"

#define CHECK(expr) do {     if (!(expr)) {         fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr);         return 1;     } } while (0)

int main(void)
{
    mbr_output_state_t state;
    mbr_output_state_clear(&state);

    mbr_mouse_event_t event = {0};
    event.type = MBR_MOUSE_EVENT_BUTTON;
    event.data.button.button = MBR_MOUSE_BUTTON_LEFT;
    event.data.button.pressed = true;
    CHECK(mbr_output_state_apply_mouse_event(&state, &event));
    CHECK(state.buttons == MBR_OUTPUT_BUTTON_LEFT);

    CHECK(mbr_output_state_apply_mouse_event(&state, &event));
    CHECK(state.buttons == MBR_OUTPUT_BUTTON_LEFT);

    event.data.button.pressed = false;
    CHECK(mbr_output_state_apply_mouse_event(&state, &event));
    CHECK(state.buttons == 0u);

    event.type = MBR_MOUSE_EVENT_MOVE;
    event.data.move.dx = 300;
    event.data.move.dy = -260;
    CHECK(mbr_output_state_apply_mouse_event(&state, &event));
    CHECK(state.x == 300 && state.y == -260);

    event.type = MBR_MOUSE_EVENT_WHEEL;
    event.data.wheel.vertical = 130;
    event.data.wheel.horizontal = -129;
    CHECK(mbr_output_state_apply_mouse_event(&state, &event));
    CHECK(state.wheel == 130 && state.pan == -129);

    CHECK(mbr_output_state_consume_relative(&state, 127, -128, 127, -128));
    CHECK(state.x == 173 && state.y == -132);
    CHECK(state.wheel == 3 && state.pan == -1);

    CHECK(!mbr_output_state_consume_relative(&state, -1, 0, 0, 0));

    event.type = MBR_MOUSE_EVENT_BUTTON;
    event.data.button.button = MBR_MOUSE_BUTTON_FORWARD;
    event.data.button.pressed = true;
    CHECK(mbr_output_state_apply_mouse_event(&state, &event));
    event.data.button.button = MBR_MOUSE_BUTTON_BACKWARD;
    CHECK(mbr_output_state_apply_mouse_event(&state, &event));
    mbr_output_state_release_all(&state);
    CHECK(state.buttons == 0u && !state.escape_down);
    CHECK(state.x == 173 && state.y == -132);

    puts("MBR-05 output state: OK");
    return 0;
}
