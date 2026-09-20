#include "pico/stdlib.h"

#include "mbr/app/app.h"
#include "mbr/hat/hat.h"
#include "mbr/interaction/interaction.h"
#include "mbr/mouse_session/mouse_session.h"
#include "mbr/renderer/renderer.h"

#define QUALIFICATION_HOLD_MS 5000u

static const mbr_screen_id_t k_screens[MBR_SCREEN_COUNT] = {
    MBR_SCREEN_SEARCHING_FIRST,
    MBR_SCREEN_FIRST_MOUSE_CONNECTED,
    MBR_SCREEN_HOME_SEARCHING,
    MBR_SCREEN_HOME_SEARCHING_HELP,
    MBR_SCREEN_HOME_RETRY,
    MBR_SCREEN_HOME_RETRY_HELP,
    MBR_SCREEN_PAIR_NEW,
    MBR_SCREEN_HELP_PAIR_NEW,
    MBR_SCREEN_RETRY_PAIR_NEW,
    MBR_SCREEN_HELP_RETRY_PAIR_NEW,
    MBR_SCREEN_HOME_CONNECTED,
    MBR_SCREEN_HELP_HOME_CONNECTED,
    MBR_SCREEN_REMAPPER_OPTIONS,
    MBR_SCREEN_HELP_REMAPPER_OPTIONS,
    MBR_SCREEN_PASSTHROUGH_ACTIVE,
    MBR_SCREEN_PASSTHROUGH_NOT_ACTIVE,
    MBR_SCREEN_STANDARD_NOT_ACTIVE,
    MBR_SCREEN_STANDARD_ACTIVE,
    MBR_SCREEN_ESCAPE_NOT_ACTIVE,
    MBR_SCREEN_ESCAPE_ACTIVE,
    MBR_SCREEN_CUSTOM_EDIT,
    MBR_SCREEN_LEFT,
    MBR_SCREEN_RIGHT,
    MBR_SCREEN_MIDDLE,
    MBR_SCREEN_FORWARD,
    MBR_SCREEN_BACKWARD,
    MBR_SCREEN_SAVED_DEVICES,
    MBR_SCREEN_REMOVE_THIS,
    MBR_SCREEN_HELP_REMOVE_THIS,
    MBR_SCREEN_LEARN_THE_KEYS
};

static void configure_fixture_model(mbr_ux_model_t *model)
{
    mbr_mouse_session_slot_clear(&model->live);
    model->screen = MBR_SCREEN_SEARCHING_FIRST;
    model->help_return = MBR_SCREEN_SEARCHING_FIRST;
    model->selection = 0u;
    model->saved_page = 0u;
    model->custom_source = MBR_MOUSE_SOURCE_LEFT;
    model->custom_target_selection = MBR_MOUSE_TARGET_LEFT;
    model->remove_mouse_id = (mbr_mouse_id_t){1u};
}

static void prepare_screen_state(mbr_ux_model_t *model, mbr_screen_id_t screen)
{
    const bool needs_live = !(screen == MBR_SCREEN_SEARCHING_FIRST ||
                              screen == MBR_SCREEN_HOME_SEARCHING ||
                              screen == MBR_SCREEN_HOME_SEARCHING_HELP ||
                              screen == MBR_SCREEN_HOME_RETRY ||
                              screen == MBR_SCREEN_HOME_RETRY_HELP);
    const mbr_mouse_session_id_t session = {{1u},1u};
    if (needs_live) (void)mbr_mouse_session_slot_promote(&model->live,session);
    else mbr_mouse_session_slot_clear(&model->live);

    model->screen=screen;
    model->selection=0u;
    model->saved_page=0u;
    model->help_return=MBR_SCREEN_SEARCHING_FIRST;
    switch (screen) {
    case MBR_SCREEN_HOME_SEARCHING_HELP: model->help_return=MBR_SCREEN_HOME_SEARCHING; break;
    case MBR_SCREEN_HOME_RETRY_HELP: model->help_return=MBR_SCREEN_HOME_RETRY; break;
    case MBR_SCREEN_HELP_PAIR_NEW: model->help_return=MBR_SCREEN_PAIR_NEW; break;
    case MBR_SCREEN_HELP_RETRY_PAIR_NEW: model->help_return=MBR_SCREEN_RETRY_PAIR_NEW; break;
    case MBR_SCREEN_HELP_HOME_CONNECTED: model->help_return=MBR_SCREEN_HOME_CONNECTED; break;
    case MBR_SCREEN_HELP_REMAPPER_OPTIONS: model->help_return=MBR_SCREEN_REMAPPER_OPTIONS; break;
    case MBR_SCREEN_HELP_REMOVE_THIS: model->help_return=MBR_SCREEN_REMOVE_THIS; break;
    default: break;
    }
}

static void render_state(const mbr_display_hal_t *display,const mbr_ux_model_t *model)
{
    mbr_ui_frame_t frame;
    mbr_ui_project(model,&frame);
    (void)mbr_renderer_render(display,&frame);
}

int main(void)
{
    mbr_saved_mouse_t storage[1];
    mbr_ux_model_t model;
    mbr_ux_effects_t effects;
    mbr_display_hal_t display;
    mbr_ux_model_init(&model,storage,1u);
    (void)mbr_mouse_registry_add(&model.registry,(mbr_saved_mouse_t){{1u},"LONG MOUSE NAME 123456789",MBR_PROFILE_ESCAPE});
    configure_fixture_model(&model);

    if (!mbr_renderer_init(&display)) for (;;) tight_loop_contents();
    mbr_hat_pico_init();
    mbr_renderer_set_backlight(true);

    size_t index=0u;
    uint32_t last_switch=to_ms_since_boot(get_absolute_time());
    prepare_screen_state(&model,k_screens[index]);
    render_state(&display,&model);

    for (;;) {
        mbr_hat_pico_task();
        mbr_hat_event_t event;
        while (mbr_hat_pico_poll_event(&event)) {
            const bool was_locked=mbr_interaction_is_locked(&model.interaction);
            mbr_ux_input(&model,event.control,event.pressed,&effects);
            const bool is_locked=mbr_interaction_is_locked(&model.interaction);
            if (!was_locked && is_locked) mbr_renderer_set_backlight(false);
            else if (was_locked && !is_locked) {
                mbr_renderer_set_backlight(true);
                render_state(&display,&model);
            } else if (!is_locked) render_state(&display,&model);
        }

        const uint32_t now=to_ms_since_boot(get_absolute_time());
        if (!mbr_interaction_is_locked(&model.interaction) && (uint32_t)(now-last_switch)>=QUALIFICATION_HOLD_MS) {
            index=(index+1u)%MBR_SCREEN_COUNT;
            prepare_screen_state(&model,k_screens[index]);
            render_state(&display,&model);
            last_switch=now;
        }
        tight_loop_contents();
    }
}