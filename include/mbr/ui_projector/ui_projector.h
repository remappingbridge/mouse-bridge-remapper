#ifndef MBR_UI_PROJECTOR_UI_PROJECTOR_H
#define MBR_UI_PROJECTOR_UI_PROJECTOR_H

#include <stdbool.h>
#include <stddef.h>

#include "mbr/domain/domain.h"
#include "mbr/interaction/interaction.h"
#include "mbr/mouse_registry/mouse_registry.h"
#include "mbr/mouse_session/mouse_session.h"
#include "mbr/pairing_coordinator/pairing_coordinator.h"
#include "mbr/profiles/profiles.h"

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

typedef struct {
    uint8_t start;
    uint8_t length;
    mbr_ui_color_t color;
} mbr_ui_span_t;

typedef struct {
    char text[MBR_UI_SEMANTIC_WIDTH + 1u];
    mbr_ui_color_t base_color;
    mbr_ui_span_t spans[MBR_UI_MAX_SPANS_PER_ROW];
    size_t span_count;
} mbr_ui_row_t;

typedef struct {
    mbr_screen_id_t screen;
    mbr_ui_row_t rows[MBR_UI_MAX_ROWS];
    size_t row_count;
} mbr_ui_frame_t;

void mbr_ui_project_name(const char *source,
                         char output[MBR_UI_SEMANTIC_WIDTH + 1u]);
void mbr_ui_project(const mbr_ux_model_t *model, mbr_ui_frame_t *frame);
mbr_ui_color_t mbr_ui_color_at(const mbr_ui_frame_t *frame,
                               size_t row,
                               size_t column);

#endif
