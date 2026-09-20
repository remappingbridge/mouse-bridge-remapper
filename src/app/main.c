#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "pico/stdlib.h"

#include "mbr/app/app.h"
#include "mbr/ble_hogp/ble_hogp.h"
#include "mbr/hat/hat.h"
#include "mbr/interaction/interaction.h"
#include "mbr/output_state/output_state.h"
#include "mbr/renderer/renderer.h"
#include "mbr/usb_hid/usb_hid.h"

#define MBR_RUNTIME_MESSAGES_PER_TICK 32u

static void render_state(const mbr_display_hal_t *display,
                         const mbr_ux_model_t *model)
{
    mbr_ui_frame_t frame;
    mbr_ui_project(model, &frame);
    (void)mbr_renderer_render(display, &frame);
}

static bool copy_live_name(char *destination,
                           size_t capacity,
                           const char *source)
{
    if (destination == NULL || capacity == 0u) return false;
    if (source == NULL || source[0] == '\0') source = "UNKNOWN MOUSE";
    size_t length = strlen(source);
    if (length >= capacity) length = capacity - 1u;
    memcpy(destination, source, length);
    destination[length] = '\0';
    return true;
}

static void service_usb_mouse(mbr_output_state_t *output,
                              uint8_t *last_buttons,
                              bool *last_valid)
{
    mbr_usb_hid_mouse_report_t report;
    mbr_usb_hid_build_mouse_report(&report, output);

    const bool relative =
        report.x != 0 || report.y != 0 ||
        report.wheel != 0 || report.pan != 0;
    const bool buttons_changed =
        !*last_valid || report.buttons != *last_buttons;
    if (!relative && !buttons_changed) return;

    if (!mbr_usb_hid_pico_send_mouse(&report)) return;

    (void)mbr_output_state_consume_relative(
        output, report.x, report.y, report.wheel, report.pan);
    *last_buttons = report.buttons;
    *last_valid = true;
}

static bool service_ble_messages(mbr_ux_model_t *model,
                                 mbr_output_state_t *output,
                                 char *live_name,
                                 size_t live_name_capacity,
                                 mbr_mouse_session_id_t *live_session,
                                 bool *live_session_valid)
{
    bool ui_changed = false;

    for (unsigned count = 0u;
         count < MBR_RUNTIME_MESSAGES_PER_TICK;
         ++count) {
        mbr_bt_runtime_message_t message;
        if (!mbr_bt_runtime_poll(&message)) break;

        mbr_ble_hogp_event_t event;
        if (!mbr_ble_hogp_decode_runtime_message(&message, &event))
            continue;

        switch (event.type) {
        case MBR_BLE_HOGP_EVENT_CONNECTED: {
            if (!copy_live_name(live_name, live_name_capacity, event.name))
                break;

            mbr_saved_mouse_t record = {
                .mouse_id = event.session_id.mouse_id,
                .name = live_name,
                .profile = MBR_PROFILE_PASSTHROUGH,
            };

            bool accepted =
                mbr_ux_first_mouse_ready(
                    model, model->pairing.transaction_id,
                    record, event.session_id);

            if (!accepted) {
                accepted =
                    mbr_ux_saved_mouse_ready(
                        model, model->pairing.transaction_id,
                        event.session_id.mouse_id,
                        event.session_id);
            }

            if (!accepted) {
                accepted = mbr_ux_mouse_reconnected(
                    model, event.session_id.mouse_id,
                    event.session_id);
            }

            if (accepted) {
                *live_session = event.session_id;
                *live_session_valid = true;
                ui_changed = true;
            }
            break;
        }

        case MBR_BLE_HOGP_EVENT_DISCONNECTED:
            if (!*live_session_valid ||
                !mbr_mouse_session_id_equal(
                    *live_session, event.session_id))
                break;

            mbr_output_state_release_all(output);
            output->x = 0;
            output->y = 0;
            output->wheel = 0;
            output->pan = 0;
            *live_session_valid = false;
            (void)mbr_ux_mouse_disconnected(
                model, event.session_id, &(mbr_ux_effects_t){0});
            ui_changed = true;
            break;

        case MBR_BLE_HOGP_EVENT_MOUSE:
            if (!*live_session_valid ||
                !mbr_mouse_session_id_equal(
                    *live_session, event.session_id))
                break;
            if (!mbr_output_state_apply_mouse_event(
                    output, &event.mouse)) {
                mbr_output_state_clear(output);
                *live_session_valid = false;
            }
            break;

        default:
            break;
        }
    }

    if (mbr_bt_runtime_take_overflow()) {
        mbr_output_state_clear(output);
        *live_session_valid = false;
    }

    return ui_changed;
}

int main(void)
{
    mbr_saved_mouse_t storage[4];
    mbr_ux_model_t model;
    mbr_ux_effects_t effects;
    mbr_display_hal_t display;
    mbr_output_state_t output;
    char live_name[MBR_BLE_HOGP_NAME_CAPACITY] = "UNKNOWN MOUSE";
    mbr_mouse_session_id_t live_session = {0};
    bool live_session_valid = false;
    uint8_t last_mouse_buttons = 0u;
    bool last_mouse_valid = false;

    mbr_output_state_clear(&output);
    mbr_ux_model_init(&model, storage, 4u);
    mbr_ux_boot(&model, &effects);

    if (!mbr_usb_hid_pico_init())
        for (;;) tight_loop_contents();
    if (!mbr_renderer_init(&display))
        for (;;) {
            mbr_usb_hid_pico_task();
            tight_loop_contents();
        }

    mbr_hat_pico_init();
    render_state(&display, &model);
    mbr_renderer_set_backlight(true);

    (void)mbr_ble_hogp_start();

    for (;;) {
        mbr_usb_hid_pico_task();

        const bool ble_ui_changed =
            service_ble_messages(&model, &output,
                                 live_name, sizeof(live_name),
                                 &live_session, &live_session_valid);
        if (ble_ui_changed &&
            !mbr_interaction_is_locked(&model.interaction))
            render_state(&display, &model);

        service_usb_mouse(&output, &last_mouse_buttons,
                          &last_mouse_valid);

        mbr_hat_pico_task();
        mbr_hat_event_t event;
        while (mbr_hat_pico_poll_event(&event)) {
            const bool was_locked =
                mbr_interaction_is_locked(&model.interaction);
            mbr_ux_input(&model, event.control, event.pressed, &effects);
            const bool is_locked =
                mbr_interaction_is_locked(&model.interaction);

            if (!was_locked && is_locked) {
                mbr_renderer_set_backlight(false);
            } else if (was_locked && !is_locked) {
                render_state(&display, &model);
                mbr_renderer_set_backlight(true);
            } else if (!is_locked) {
                render_state(&display, &model);
            }
        }

        tight_loop_contents();
    }
}
