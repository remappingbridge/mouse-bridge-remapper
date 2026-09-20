#ifndef MBR_APP_APP_H
#define MBR_APP_APP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mbr/domain/domain.h"
#include "mbr/interaction/interaction.h"
#include "mbr/mouse_registry/mouse_registry.h"
#include "mbr/mouse_session/mouse_session.h"
#include "mbr/pairing_coordinator/pairing_coordinator.h"
#include "mbr/profiles/profiles.h"

typedef enum {
    MBR_UX_EFFECT_NONE = 0,
    MBR_UX_EFFECT_START_SEARCH,
    MBR_UX_EFFECT_CANCEL_SEARCH,
    MBR_UX_EFFECT_APPLY_PROFILE,
    MBR_UX_EFFECT_SAVE_CUSTOM_DRAFT,
    MBR_UX_EFFECT_REMOVE_MOUSE
} mbr_ux_effect_kind_t;

typedef struct {
    mbr_ux_effect_kind_t kind;
    mbr_search_purpose_t purpose;
    uint32_t transaction_id;
    uint32_t duration_ms;
    mbr_mouse_id_t mouse_id;
    mbr_profile_kind_t profile;
    mbr_mouse_source_t source;
    mbr_mouse_target_t target;
} mbr_ux_effect_t;

typedef struct {
    mbr_ux_effect_t items[MBR_UX_MAX_EFFECTS];
    size_t count;
} mbr_ux_effects_t;

typedef struct {
    mbr_mouse_registry_t registry;
    mbr_authoritative_mouse_slot_t live;
    mbr_custom_draft_t custom_draft;
    mbr_pairing_coordinator_t pairing;
    mbr_interaction_t interaction;

    mbr_screen_id_t screen;
    mbr_screen_id_t help_return;
    unsigned selection;
    size_t saved_page;
    mbr_mouse_source_t custom_source;
    unsigned custom_target_selection;

    bool pending_profile_apply;
    mbr_profile_kind_t pending_profile;
    bool pending_remove;
    mbr_mouse_id_t remove_mouse_id;
} mbr_ux_model_t;

void mbr_ux_effects_clear(mbr_ux_effects_t *effects);
void mbr_ux_model_init(mbr_ux_model_t *model,
                       mbr_saved_mouse_t *storage,
                       size_t storage_capacity);
void mbr_ux_boot(mbr_ux_model_t *model, mbr_ux_effects_t *effects);
void mbr_ux_go_home(mbr_ux_model_t *model, mbr_ux_effects_t *effects);
void mbr_ux_input(mbr_ux_model_t *model,
                  mbr_control_t control,
                  bool pressed,
                  mbr_ux_effects_t *effects);

bool mbr_ux_first_mouse_ready(mbr_ux_model_t *model,
                              uint32_t transaction_id,
                              mbr_saved_mouse_t record,
                              mbr_mouse_session_id_t session_id);
bool mbr_ux_saved_mouse_ready(mbr_ux_model_t *model,
                              uint32_t transaction_id,
                              mbr_mouse_id_t mouse_id,
                              mbr_mouse_session_id_t session_id);
bool mbr_ux_pair_new_candidate_ready(mbr_ux_model_t *model,
                                     uint32_t transaction_id,
                                     mbr_mouse_id_t mouse_id,
                                     mbr_mouse_session_id_t session_id);
bool mbr_ux_pair_new_handoff_committed(mbr_ux_model_t *model,
                                       uint32_t transaction_id,
                                       mbr_saved_mouse_t new_record,
                                       mbr_mouse_session_id_t new_session_id);
void mbr_ux_search_expired(mbr_ux_model_t *model,
                           uint32_t transaction_id,
                           mbr_ux_effects_t *effects);
void mbr_ux_mouse_disconnected(mbr_ux_model_t *model,
                               mbr_mouse_session_id_t session_id,
                               mbr_ux_effects_t *effects);
bool mbr_ux_profile_apply_confirmed(mbr_ux_model_t *model,
                                    mbr_mouse_id_t mouse_id,
                                    mbr_profile_kind_t profile);
void mbr_ux_profile_apply_failed(mbr_ux_model_t *model,
                                 mbr_mouse_id_t mouse_id,
                                 mbr_profile_kind_t profile);
bool mbr_ux_remove_confirmed(mbr_ux_model_t *model,
                             mbr_mouse_id_t mouse_id,
                             mbr_ux_effects_t *effects);
void mbr_ux_remove_failed(mbr_ux_model_t *model, mbr_mouse_id_t mouse_id);

const mbr_saved_mouse_t *mbr_ux_live_mouse(const mbr_ux_model_t *model);
const mbr_saved_mouse_t *mbr_ux_saved_page_mouse(const mbr_ux_model_t *model);

#endif
