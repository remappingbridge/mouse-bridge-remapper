#include "mbr/app/app.h"

#include <string.h>

static void push_effect(mbr_ux_effects_t *effects, mbr_ux_effect_t effect)
{
    if (effects == NULL || effects->count >= MBR_UX_MAX_EFFECTS) return;
    effects->items[effects->count++] = effect;
}

void mbr_ux_effects_clear(mbr_ux_effects_t *effects)
{
    if (effects == NULL) return;
    memset(effects, 0, sizeof(*effects));
}

static void set_screen(mbr_ux_model_t *model, mbr_screen_id_t screen)
{
    model->screen = screen;
    model->selection = 0u;
}

static uint32_t duration_for(mbr_search_purpose_t purpose)
{
    switch (purpose) {
    case MBR_SEARCH_FIRST_MOUSE: return MBR_FIRST_MOUSE_DURATION_MS;
    case MBR_SEARCH_SAVED: return MBR_SEARCH_SAVED_DURATION_MS;
    case MBR_SEARCH_PAIR_NEW: return MBR_PAIR_NEW_DURATION_MS;
    default: return 0u;
    }
}

static uint32_t start_search(mbr_ux_model_t *model,
                             mbr_search_purpose_t purpose,
                             mbr_ux_effects_t *effects)
{
    const uint32_t duration = duration_for(purpose);
    const uint32_t transaction_id =
        mbr_pairing_coordinator_start(&model->pairing, purpose, duration);
    mbr_ux_effect_t effect = {0};
    effect.kind = MBR_UX_EFFECT_START_SEARCH;
    effect.purpose = purpose;
    effect.transaction_id = transaction_id;
    effect.duration_ms = duration;
    push_effect(effects, effect);
    return transaction_id;
}

static void cancel_active_search(mbr_ux_model_t *model,
                                 mbr_ux_effects_t *effects)
{
    if (!model->pairing.active) return;
    mbr_ux_effect_t effect = {0};
    effect.kind = MBR_UX_EFFECT_CANCEL_SEARCH;
    effect.purpose = model->pairing.purpose;
    effect.transaction_id = model->pairing.transaction_id;
    push_effect(effects, effect);
    mbr_pairing_coordinator_stop(&model->pairing);
}

static bool is_help_screen(mbr_screen_id_t screen)
{
    switch (screen) {
    case MBR_SCREEN_HOME_SEARCHING_HELP:
    case MBR_SCREEN_HOME_RETRY_HELP:
    case MBR_SCREEN_HELP_PAIR_NEW:
    case MBR_SCREEN_HELP_RETRY_PAIR_NEW:
    case MBR_SCREEN_HELP_HOME_CONNECTED:
    case MBR_SCREEN_HELP_REMAPPER_OPTIONS:
    case MBR_SCREEN_HELP_REMOVE_THIS:
        return true;
    default:
        return false;
    }
}

static bool is_instruction_screen(mbr_screen_id_t screen)
{
    return screen == MBR_SCREEN_FIRST_MOUSE_CONNECTED ||
           screen == MBR_SCREEN_LEARN_THE_KEYS;
}

static bool ordinary_lock_allowed(mbr_screen_id_t screen)
{
    switch (screen) {
    case MBR_SCREEN_PAIR_NEW:
    case MBR_SCREEN_RETRY_PAIR_NEW:
    case MBR_SCREEN_PASSTHROUGH_ACTIVE:
    case MBR_SCREEN_PASSTHROUGH_NOT_ACTIVE:
    case MBR_SCREEN_STANDARD_NOT_ACTIVE:
    case MBR_SCREEN_STANDARD_ACTIVE:
        return true;
    default:
        return false;
    }
}

static unsigned option_count(const mbr_ux_model_t *model)
{
    switch (model->screen) {
    case MBR_SCREEN_HOME_SEARCHING:
    case MBR_SCREEN_HOME_RETRY:
        return 3u;
    case MBR_SCREEN_HOME_CONNECTED:
        return 4u;
    case MBR_SCREEN_REMAPPER_OPTIONS:
        return 4u;
    case MBR_SCREEN_CUSTOM_EDIT:
        return MBR_MOUSE_SOURCE_COUNT;
    case MBR_SCREEN_LEFT:
    case MBR_SCREEN_RIGHT:
    case MBR_SCREEN_MIDDLE:
    case MBR_SCREEN_FORWARD:
    case MBR_SCREEN_BACKWARD:
        return MBR_MOUSE_TARGET_COUNT;
    default:
        return 0u;
    }
}

static unsigned wrap_prev(unsigned value, unsigned count)
{
    return count == 0u ? 0u : (value + count - 1u) % count;
}

static unsigned wrap_next(unsigned value, unsigned count)
{
    return count == 0u ? 0u : (value + 1u) % count;
}

const mbr_saved_mouse_t *mbr_ux_live_mouse(const mbr_ux_model_t *model)
{
    if (model == NULL || !model->live.occupied || !model->live.ready) return NULL;
    return mbr_mouse_registry_find(&model->registry, model->live.session_id.mouse_id);
}

const mbr_saved_mouse_t *mbr_ux_saved_page_mouse(const mbr_ux_model_t *model)
{
    if (model == NULL || model->registry.count == 0u) return NULL;
    size_t page = model->saved_page;
    if (page >= model->registry.count) page = model->registry.count - 1u;
    return mbr_mouse_registry_get(&model->registry, page);
}

void mbr_ux_model_init(mbr_ux_model_t *model,
                       mbr_saved_mouse_t *storage,
                       size_t storage_capacity)
{
    if (model == NULL) return;
    memset(model, 0, sizeof(*model));
    mbr_mouse_registry_init(&model->registry, storage, storage_capacity);
    mbr_mouse_session_slot_init(&model->live);
    mbr_custom_draft_init(&model->custom_draft);
    mbr_pairing_coordinator_init(&model->pairing);
    mbr_interaction_init(&model->interaction);
    model->screen = MBR_SCREEN_SEARCHING_FIRST;
    model->help_return = MBR_SCREEN_SEARCHING_FIRST;
    model->custom_source = MBR_MOUSE_SOURCE_LEFT;
}

void mbr_ux_go_home(mbr_ux_model_t *model, mbr_ux_effects_t *effects)
{
    if (model == NULL) return;
    if (model->registry.count == 0u) {
        set_screen(model, MBR_SCREEN_SEARCHING_FIRST);
        if (!mbr_pairing_coordinator_matches(&model->pairing,
                                             model->pairing.transaction_id,
                                             MBR_SEARCH_FIRST_MOUSE)) {
            cancel_active_search(model, effects);
            (void)start_search(model, MBR_SEARCH_FIRST_MOUSE, effects);
        }
        return;
    }

    if (mbr_mouse_session_ready_count(&model->live) == 1u) {
        if (model->pairing.active && model->pairing.purpose != MBR_SEARCH_PAIR_NEW)
            mbr_pairing_coordinator_stop(&model->pairing);
        set_screen(model, MBR_SCREEN_HOME_CONNECTED);
        return;
    }

    set_screen(model, MBR_SCREEN_HOME_SEARCHING);
    if (!mbr_pairing_coordinator_matches(&model->pairing,
                                         model->pairing.transaction_id,
                                         MBR_SEARCH_SAVED)) {
        cancel_active_search(model, effects);
        (void)start_search(model, MBR_SEARCH_SAVED, effects);
    }
}

void mbr_ux_boot(mbr_ux_model_t *model, mbr_ux_effects_t *effects)
{
    mbr_ux_effects_clear(effects);
    mbr_ux_go_home(model, effects);
}

static void enter_help(mbr_ux_model_t *model, mbr_screen_id_t help)
{
    model->help_return = model->screen;
    set_screen(model, help);
}

static void start_pair_new(mbr_ux_model_t *model, mbr_ux_effects_t *effects)
{
    cancel_active_search(model, effects);
    set_screen(model, MBR_SCREEN_PAIR_NEW);
    (void)start_search(model, MBR_SEARCH_PAIR_NEW, effects);
}

static void enter_saved_devices(mbr_ux_model_t *model)
{
    model->saved_page = 0u;
    set_screen(model, MBR_SCREEN_SAVED_DEVICES);
}

static void enter_source_editor(mbr_ux_model_t *model, mbr_mouse_source_t source)
{
    static const mbr_screen_id_t screens[MBR_MOUSE_SOURCE_COUNT] = {
        MBR_SCREEN_LEFT,
        MBR_SCREEN_RIGHT,
        MBR_SCREEN_MIDDLE,
        MBR_SCREEN_FORWARD,
        MBR_SCREEN_BACKWARD,
    };
    model->custom_source = source;
    model->custom_target_selection = model->custom_draft.targets[source];
    model->screen = screens[source];
    model->selection = model->custom_target_selection;
}

static void request_profile(mbr_ux_model_t *model,
                            mbr_profile_kind_t profile,
                            mbr_ux_effects_t *effects)
{
    const mbr_saved_mouse_t *live = mbr_ux_live_mouse(model);
    if (live == NULL) return;
    model->pending_profile_apply = true;
    model->pending_profile = profile;
    mbr_ux_effect_t effect = {0};
    effect.kind = MBR_UX_EFFECT_APPLY_PROFILE;
    effect.mouse_id = live->mouse_id;
    effect.profile = profile;
    push_effect(effects, effect);
}

static void access_home_option(mbr_ux_model_t *model, mbr_ux_effects_t *effects)
{
    switch (model->selection) {
    case 0u: start_pair_new(model, effects); break;
    case 1u: enter_saved_devices(model); break;
    case 2u: set_screen(model, MBR_SCREEN_LEARN_THE_KEYS); break;
    default: break;
    }
}

static void access_connected_home_option(mbr_ux_model_t *model,
                                         mbr_ux_effects_t *effects)
{
    switch (model->selection) {
    case 0u: start_pair_new(model, effects); break;
    case 1u: set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS); break;
    case 2u: enter_saved_devices(model); break;
    case 3u: set_screen(model, MBR_SCREEN_LEARN_THE_KEYS); break;
    default: break;
    }
}

static void access_remapper_option(mbr_ux_model_t *model)
{
    const mbr_saved_mouse_t *live = mbr_ux_live_mouse(model);
    const mbr_profile_kind_t active = live == NULL ? MBR_PROFILE_PASSTHROUGH : live->profile;
    switch (model->selection) {
    case 0u: set_screen(model, active == MBR_PROFILE_PASSTHROUGH ? MBR_SCREEN_PASSTHROUGH_ACTIVE : MBR_SCREEN_PASSTHROUGH_NOT_ACTIVE); break;
    case 1u: set_screen(model, active == MBR_PROFILE_STANDARD ? MBR_SCREEN_STANDARD_ACTIVE : MBR_SCREEN_STANDARD_NOT_ACTIVE); break;
    case 2u: set_screen(model, active == MBR_PROFILE_ESCAPE ? MBR_SCREEN_ESCAPE_ACTIVE : MBR_SCREEN_ESCAPE_NOT_ACTIVE); break;
    case 3u: set_screen(model, MBR_SCREEN_CUSTOM_EDIT); break;
    default: break;
    }
}

static bool handle_instruction_release(mbr_ux_model_t *model,
                                       mbr_control_t control,
                                       mbr_ux_effects_t *effects)
{
    if (!is_instruction_screen(model->screen)) return false;
    if (model->interaction.locked) {
        if (control == MBR_CONTROL_KEY_X)
            mbr_interaction_set_locked(&model->interaction, false);
        return true;
    }
    if (control == MBR_CONTROL_KEY_B) {
        mbr_interaction_set_locked(&model->interaction, true);
        return true;
    }
    if (control == MBR_CONTROL_KEY_Y) {
        mbr_ux_go_home(model, effects);
        return true;
    }
    return true;
}

void mbr_ux_input(mbr_ux_model_t *model,
                  mbr_control_t control,
                  bool pressed,
                  mbr_ux_effects_t *effects)
{
    if (model == NULL || (unsigned)control >= MBR_CONTROL_COUNT) return;
    mbr_ux_effects_clear(effects);
    if (pressed) {
        (void)mbr_interaction_press(&model->interaction, control);
        return;
    }
    if (!mbr_interaction_release(&model->interaction, control)) return;
    if (is_help_screen(model->screen)) {
        set_screen(model, model->help_return);
        return;
    }
    if (is_instruction_screen(model->screen)) {
        (void)handle_instruction_release(model, control, effects);
        return;
    }
    if (model->interaction.locked) {
        mbr_interaction_set_locked(&model->interaction, false);
        return;
    }
    if (model->screen == MBR_SCREEN_SEARCHING_FIRST) return;
    if (ordinary_lock_allowed(model->screen) && control == MBR_CONTROL_KEY_Y) {
        mbr_interaction_set_locked(&model->interaction, true);
        return;
    }

    const unsigned count = option_count(model);
    if (count != 0u && control == MBR_CONTROL_JOY_UP) {
        model->selection = wrap_prev(model->selection, count);
        if (model->screen >= MBR_SCREEN_LEFT && model->screen <= MBR_SCREEN_BACKWARD)
            model->custom_target_selection = model->selection;
        return;
    }
    if (count != 0u && control == MBR_CONTROL_JOY_DOWN) {
        model->selection = wrap_next(model->selection, count);
        if (model->screen >= MBR_SCREEN_LEFT && model->screen <= MBR_SCREEN_BACKWARD)
            model->custom_target_selection = model->selection;
        return;
    }

    switch (model->screen) {
    case MBR_SCREEN_HOME_SEARCHING:
        if (control == MBR_CONTROL_JOY_PRESS) access_home_option(model, effects);
        else if (control == MBR_CONTROL_KEY_B) { cancel_active_search(model, effects); set_screen(model, MBR_SCREEN_HOME_RETRY); }
        else if (control == MBR_CONTROL_KEY_X) enter_help(model, MBR_SCREEN_HOME_SEARCHING_HELP);
        break;
    case MBR_SCREEN_HOME_RETRY:
        if (control == MBR_CONTROL_JOY_PRESS) access_home_option(model, effects);
        else if (control == MBR_CONTROL_KEY_A) { set_screen(model, MBR_SCREEN_HOME_SEARCHING); (void)start_search(model, MBR_SEARCH_SAVED, effects); }
        else if (control == MBR_CONTROL_KEY_X) enter_help(model, MBR_SCREEN_HOME_RETRY_HELP);
        break;
    case MBR_SCREEN_PAIR_NEW:
        if (control == MBR_CONTROL_KEY_B) { cancel_active_search(model, effects); mbr_ux_go_home(model, effects); }
        else if (control == MBR_CONTROL_KEY_X) enter_help(model, MBR_SCREEN_HELP_PAIR_NEW);
        break;
    case MBR_SCREEN_RETRY_PAIR_NEW:
        if (control == MBR_CONTROL_KEY_A) start_pair_new(model, effects);
        else if (control == MBR_CONTROL_KEY_B) mbr_ux_go_home(model, effects);
        else if (control == MBR_CONTROL_KEY_X) enter_help(model, MBR_SCREEN_HELP_RETRY_PAIR_NEW);
        break;
    case MBR_SCREEN_HOME_CONNECTED:
        if (control == MBR_CONTROL_JOY_PRESS) access_connected_home_option(model, effects);
        else if (control == MBR_CONTROL_KEY_X) enter_help(model, MBR_SCREEN_HELP_HOME_CONNECTED);
        break;
    case MBR_SCREEN_REMAPPER_OPTIONS:
        if (control == MBR_CONTROL_JOY_PRESS) access_remapper_option(model);
        else if (control == MBR_CONTROL_JOY_LEFT) mbr_ux_go_home(model, effects);
        else if (control == MBR_CONTROL_KEY_X) enter_help(model, MBR_SCREEN_HELP_REMAPPER_OPTIONS);
        break;
    case MBR_SCREEN_PASSTHROUGH_ACTIVE:
        if (control == MBR_CONTROL_KEY_B) set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS);
        break;
    case MBR_SCREEN_PASSTHROUGH_NOT_ACTIVE:
        if (control == MBR_CONTROL_KEY_A) request_profile(model, MBR_PROFILE_PASSTHROUGH, effects);
        else if (control == MBR_CONTROL_KEY_B) set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS);
        break;
    case MBR_SCREEN_STANDARD_NOT_ACTIVE:
        if (control == MBR_CONTROL_KEY_A) request_profile(model, MBR_PROFILE_STANDARD, effects);
        else if (control == MBR_CONTROL_KEY_B) set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS);
        break;
    case MBR_SCREEN_STANDARD_ACTIVE:
        if (control == MBR_CONTROL_KEY_B) set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS);
        break;
    case MBR_SCREEN_ESCAPE_NOT_ACTIVE:
        if (control == MBR_CONTROL_KEY_A) request_profile(model, MBR_PROFILE_ESCAPE, effects);
        else if (control == MBR_CONTROL_KEY_B) set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS);
        break;
    case MBR_SCREEN_ESCAPE_ACTIVE:
        if (control == MBR_CONTROL_KEY_B) set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS);
        else if (control == MBR_CONTROL_JOY_LEFT) mbr_ux_go_home(model, effects);
        break;
    case MBR_SCREEN_CUSTOM_EDIT:
        if (control == MBR_CONTROL_JOY_PRESS) enter_source_editor(model, (mbr_mouse_source_t)model->selection);
        else if (control == MBR_CONTROL_KEY_A) request_profile(model, MBR_PROFILE_CUSTOM, effects);
        break;
    case MBR_SCREEN_LEFT:
    case MBR_SCREEN_RIGHT:
    case MBR_SCREEN_MIDDLE:
    case MBR_SCREEN_FORWARD:
    case MBR_SCREEN_BACKWARD:
        if (control == MBR_CONTROL_KEY_A) {
            const mbr_mouse_target_t target = (mbr_mouse_target_t)model->selection;
            if (mbr_custom_draft_set(&model->custom_draft, model->custom_source, target)) {
                mbr_ux_effect_t effect = {0};
                effect.kind = MBR_UX_EFFECT_SAVE_CUSTOM_DRAFT;
                effect.source = model->custom_source;
                effect.target = target;
                push_effect(effects, effect);
            }
            set_screen(model, MBR_SCREEN_CUSTOM_EDIT);
            model->selection = model->custom_source;
        }
        break;
    case MBR_SCREEN_SAVED_DEVICES:
        if (control == MBR_CONTROL_JOY_RIGHT && model->registry.count != 0u) model->saved_page = (model->saved_page + 1u) % model->registry.count;
        else if (control == MBR_CONTROL_JOY_LEFT && model->registry.count != 0u) model->saved_page = (model->saved_page + model->registry.count - 1u) % model->registry.count;
        else if (control == MBR_CONTROL_JOY_PRESS) {
            const mbr_saved_mouse_t *page = mbr_ux_saved_page_mouse(model);
            if (page != NULL) { model->remove_mouse_id = page->mouse_id; set_screen(model, MBR_SCREEN_REMOVE_THIS); }
        } else if (control == MBR_CONTROL_KEY_B) mbr_ux_go_home(model, effects);
        break;
    case MBR_SCREEN_REMOVE_THIS:
        if (control == MBR_CONTROL_KEY_A) {
            model->pending_remove = true;
            mbr_ux_effect_t effect = {0};
            effect.kind = MBR_UX_EFFECT_REMOVE_MOUSE;
            effect.mouse_id = model->remove_mouse_id;
            push_effect(effects, effect);
        } else if (control == MBR_CONTROL_KEY_B) set_screen(model, MBR_SCREEN_SAVED_DEVICES);
        else if (control == MBR_CONTROL_KEY_X) enter_help(model, MBR_SCREEN_HELP_REMOVE_THIS);
        break;
    default:
        break;
    }
}

bool mbr_ux_first_mouse_ready(mbr_ux_model_t *model,
                              uint32_t transaction_id,
                              mbr_saved_mouse_t record,
                              mbr_mouse_session_id_t session_id)
{
    if (model == NULL || !mbr_pairing_coordinator_matches(&model->pairing, transaction_id, MBR_SEARCH_FIRST_MOUSE) ||
        model->registry.count != 0u || !mbr_mouse_id_equal(record.mouse_id, session_id.mouse_id) ||
        !mbr_mouse_registry_add(&model->registry, record)) return false;
    mbr_mouse_session_slot_clear(&model->live);
    if (!mbr_mouse_session_slot_promote(&model->live, session_id)) return false;
    mbr_pairing_coordinator_stop(&model->pairing);
    set_screen(model, MBR_SCREEN_FIRST_MOUSE_CONNECTED);
    return true;
}

bool mbr_ux_saved_mouse_ready(mbr_ux_model_t *model,
                              uint32_t transaction_id,
                              mbr_mouse_id_t mouse_id,
                              mbr_mouse_session_id_t session_id)
{
    if (model == NULL || !mbr_pairing_coordinator_matches(&model->pairing, transaction_id, MBR_SEARCH_SAVED) ||
        mbr_mouse_registry_find(&model->registry, mouse_id) == NULL || !mbr_mouse_id_equal(mouse_id, session_id.mouse_id)) return false;
    mbr_mouse_session_slot_clear(&model->live);
    if (!mbr_mouse_session_slot_promote(&model->live, session_id)) return false;
    mbr_pairing_coordinator_stop(&model->pairing);
    set_screen(model, MBR_SCREEN_HOME_CONNECTED);
    return true;
}

bool mbr_ux_mouse_reconnected(mbr_ux_model_t *model,
                              mbr_mouse_id_t mouse_id,
                              mbr_mouse_session_id_t session_id)
{
    if (model == NULL || model->live.occupied ||
        mbr_mouse_registry_find(&model->registry, mouse_id) == NULL ||
        !mbr_mouse_id_equal(mouse_id, session_id.mouse_id)) return false;
    if (model->pairing.active && model->pairing.purpose != MBR_SEARCH_SAVED)
        return false;
    mbr_mouse_session_slot_clear(&model->live);
    if (!mbr_mouse_session_slot_promote(&model->live, session_id)) return false;
    if (model->pairing.active)
        mbr_pairing_coordinator_stop(&model->pairing);
    if (model->screen == MBR_SCREEN_HOME_SEARCHING ||
        model->screen == MBR_SCREEN_HOME_RETRY)
        set_screen(model, MBR_SCREEN_HOME_CONNECTED);
    return true;
}

bool mbr_ux_pair_new_candidate_ready(mbr_ux_model_t *model,
                                     uint32_t transaction_id,
                                     mbr_mouse_id_t mouse_id,
                                     mbr_mouse_session_id_t session_id)
{
    if (model == NULL || !mbr_pairing_coordinator_matches(&model->pairing, transaction_id, MBR_SEARCH_PAIR_NEW) ||
        mbr_mouse_registry_find(&model->registry, mouse_id) != NULL || !mbr_mouse_id_equal(mouse_id, session_id.mouse_id)) return false;
    mbr_pairing_candidate_set_ready_for(&model->pairing.candidate, transaction_id, mouse_id, session_id);
    return true;
}

bool mbr_ux_pair_new_handoff_committed(mbr_ux_model_t *model,
                                       uint32_t transaction_id,
                                       mbr_saved_mouse_t new_record,
                                       mbr_mouse_session_id_t new_session_id)
{
    if (model == NULL || !mbr_pairing_coordinator_matches(&model->pairing, transaction_id, MBR_SEARCH_PAIR_NEW) ||
        !model->pairing.candidate.present || !model->pairing.candidate.replacement_ready ||
        model->pairing.candidate.transaction_id != transaction_id ||
        !mbr_mouse_id_equal(model->pairing.candidate.mouse_id, new_record.mouse_id) ||
        !mbr_mouse_session_id_equal(model->pairing.candidate.session_id, new_session_id) ||
        !mbr_mouse_registry_add(&model->registry, new_record)) return false;
    mbr_mouse_session_slot_clear(&model->live);
    if (!mbr_mouse_session_slot_promote(&model->live, new_session_id)) return false;
    mbr_pairing_coordinator_stop(&model->pairing);
    set_screen(model, MBR_SCREEN_HOME_CONNECTED);
    return true;
}

void mbr_ux_search_expired(mbr_ux_model_t *model,
                           uint32_t transaction_id,
                           mbr_ux_effects_t *effects)
{
    if (model == NULL || !model->pairing.active || model->pairing.transaction_id != transaction_id) return;
    const mbr_search_purpose_t purpose = model->pairing.purpose;
    mbr_pairing_coordinator_stop(&model->pairing);
    if (purpose == MBR_SEARCH_FIRST_MOUSE) {
        if (model->registry.count == 0u) { set_screen(model, MBR_SCREEN_SEARCHING_FIRST); (void)start_search(model, MBR_SEARCH_FIRST_MOUSE, effects); }
    } else if (purpose == MBR_SEARCH_SAVED) {
        if (model->screen == MBR_SCREEN_HOME_SEARCHING_HELP) model->help_return = MBR_SCREEN_HOME_RETRY;
        else set_screen(model, MBR_SCREEN_HOME_RETRY);
    } else if (purpose == MBR_SEARCH_PAIR_NEW) {
        if (model->screen == MBR_SCREEN_HELP_PAIR_NEW) model->help_return = MBR_SCREEN_RETRY_PAIR_NEW;
        else set_screen(model, MBR_SCREEN_RETRY_PAIR_NEW);
    }
}

void mbr_ux_mouse_disconnected(mbr_ux_model_t *model,
                               mbr_mouse_session_id_t session_id,
                               mbr_ux_effects_t *effects)
{
    if (model == NULL || !mbr_mouse_session_matches(&model->live, session_id)) return;
    mbr_mouse_session_slot_clear(&model->live);
    if (model->screen == MBR_SCREEN_HOME_CONNECTED) mbr_ux_go_home(model, effects);
}

bool mbr_ux_profile_apply_confirmed(mbr_ux_model_t *model,
                                    mbr_mouse_id_t mouse_id,
                                    mbr_profile_kind_t profile)
{
    if (model == NULL || !model->pending_profile_apply || model->pending_profile != profile) return false;
    const mbr_saved_mouse_t *live = mbr_ux_live_mouse(model);
    if (live == NULL || !mbr_mouse_id_equal(live->mouse_id, mouse_id) || !mbr_mouse_registry_set_profile(&model->registry, mouse_id, profile)) return false;
    model->pending_profile_apply = false;
    switch (profile) {
    case MBR_PROFILE_PASSTHROUGH: set_screen(model, MBR_SCREEN_PASSTHROUGH_ACTIVE); break;
    case MBR_PROFILE_STANDARD: set_screen(model, MBR_SCREEN_STANDARD_ACTIVE); break;
    case MBR_PROFILE_ESCAPE: set_screen(model, MBR_SCREEN_ESCAPE_ACTIVE); break;
    case MBR_PROFILE_CUSTOM: set_screen(model, MBR_SCREEN_REMAPPER_OPTIONS); model->selection = 3u; break;
    default: return false;
    }
    return true;
}

void mbr_ux_profile_apply_failed(mbr_ux_model_t *model,
                                 mbr_mouse_id_t mouse_id,
                                 mbr_profile_kind_t profile)
{
    if (model == NULL || !model->pending_profile_apply || model->pending_profile != profile) return;
    const mbr_saved_mouse_t *live = mbr_ux_live_mouse(model);
    if (live == NULL || !mbr_mouse_id_equal(live->mouse_id, mouse_id)) return;
    model->pending_profile_apply = false;
}

bool mbr_ux_remove_confirmed(mbr_ux_model_t *model,
                             mbr_mouse_id_t mouse_id,
                             mbr_ux_effects_t *effects)
{
    if (model == NULL || !model->pending_remove || !mbr_mouse_id_equal(model->remove_mouse_id, mouse_id)) return false;
    if (model->live.occupied && mbr_mouse_id_equal(model->live.session_id.mouse_id, mouse_id)) mbr_mouse_session_slot_clear(&model->live);
    if (!mbr_mouse_registry_remove(&model->registry, mouse_id)) return false;
    model->pending_remove = false;
    if (model->registry.count == 0u) { model->saved_page = 0u; mbr_ux_go_home(model, effects); }
    else {
        if (model->saved_page >= model->registry.count) model->saved_page = model->registry.count - 1u;
        set_screen(model, MBR_SCREEN_SAVED_DEVICES);
    }
    return true;
}

void mbr_ux_remove_failed(mbr_ux_model_t *model, mbr_mouse_id_t mouse_id)
{
    if (model == NULL || !model->pending_remove || !mbr_mouse_id_equal(model->remove_mouse_id, mouse_id)) return;
    model->pending_remove = false;
}
