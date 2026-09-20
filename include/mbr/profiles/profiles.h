#ifndef MBR_PROFILES_PROFILES_H
#define MBR_PROFILES_PROFILES_H

#include <stdbool.h>

#include "mbr/domain/domain.h"

typedef struct {
    mbr_mouse_target_t targets[MBR_MOUSE_SOURCE_COUNT];
    bool dirty;
} mbr_custom_draft_t;

void mbr_custom_draft_init(mbr_custom_draft_t *draft);
bool mbr_custom_draft_set(mbr_custom_draft_t *draft,
                          mbr_mouse_source_t source,
                          mbr_mouse_target_t target);
const char *mbr_profile_label(mbr_profile_kind_t profile);
const char *mbr_profile_home_summary(mbr_profile_kind_t profile);
const char *mbr_mouse_source_label(mbr_mouse_source_t source);
const char *mbr_mouse_target_label(mbr_mouse_target_t target);

#endif
