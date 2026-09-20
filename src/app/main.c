#include "pico/stdlib.h"

#include "mbr/app/app.h"
#include "mbr/hat/hat.h"
#include "mbr/interaction/interaction.h"
#include "mbr/renderer/renderer.h"

static void render_state(const mbr_display_hal_t *display, const mbr_ux_model_t *model)
{
    mbr_ui_frame_t frame;
    mbr_ui_project(model,&frame);
    (void)mbr_renderer_render(display,&frame);
}

int main(void)
{
    mbr_saved_mouse_t storage[4];
    mbr_ux_model_t model;
    mbr_ux_effects_t effects;
    mbr_display_hal_t display;

    mbr_ux_model_init(&model,storage,4u);
    mbr_ux_boot(&model,&effects);

    if (!mbr_renderer_init(&display)) for (;;) tight_loop_contents();
    mbr_hat_pico_init();
    render_state(&display,&model);
    mbr_renderer_set_backlight(true);

    for (;;) {
        mbr_hat_pico_task();
        mbr_hat_event_t event;
        while (mbr_hat_pico_poll_event(&event)) {
            const bool was_locked=mbr_interaction_is_locked(&model.interaction);
            mbr_ux_input(&model,event.control,event.pressed,&effects);
            const bool is_locked=mbr_interaction_is_locked(&model.interaction);
            if (!was_locked && is_locked) {
                mbr_renderer_set_backlight(false);
            } else if (was_locked && !is_locked) {
                render_state(&display,&model);
                mbr_renderer_set_backlight(true);
            } else if (!is_locked) {
                render_state(&display,&model);
            }
        }
        tight_loop_contents();
    }
}