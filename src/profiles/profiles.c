#include "mbr/profiles/profiles.h"

void mbr_custom_draft_init(mbr_custom_draft_t *draft)
{
    if (draft == NULL) return;
    draft->targets[MBR_MOUSE_SOURCE_LEFT] = MBR_MOUSE_TARGET_LEFT;
    draft->targets[MBR_MOUSE_SOURCE_RIGHT] = MBR_MOUSE_TARGET_RIGHT;
    draft->targets[MBR_MOUSE_SOURCE_MIDDLE] = MBR_MOUSE_TARGET_MIDDLE;
    draft->targets[MBR_MOUSE_SOURCE_FORWARD] = MBR_MOUSE_TARGET_FORWARD;
    draft->targets[MBR_MOUSE_SOURCE_BACKWARD] = MBR_MOUSE_TARGET_BACKWARD;
    draft->dirty = false;
}

bool mbr_custom_draft_set(mbr_custom_draft_t *draft,
                          mbr_mouse_source_t source,
                          mbr_mouse_target_t target)
{
    if (draft == NULL || (unsigned)source >= MBR_MOUSE_SOURCE_COUNT ||
        (unsigned)target >= MBR_MOUSE_TARGET_COUNT) return false;
    draft->targets[source] = target;
    draft->dirty = true;
    return true;
}

const char *mbr_profile_label(mbr_profile_kind_t profile)
{
    static const char *const labels[MBR_PROFILE_COUNT] = {
        "PASSTHROUGH", "STANDARD", "ESCAPE", "CUSTOM"
    };
    return (unsigned)profile < MBR_PROFILE_COUNT ? labels[profile] : "PASSTHROUGH";
}

const char *mbr_profile_home_summary(mbr_profile_kind_t profile)
{
    static const char *const labels[MBR_PROFILE_COUNT] = {
        "NO REMAP PASSTHROUGH",
        "REMAPPED TO STANDARD",
        "REMAPPED TO ESCAPE",
        "REMAPPED TO CUSTOM"
    };
    return (unsigned)profile < MBR_PROFILE_COUNT ? labels[profile] : labels[0];
}

const char *mbr_mouse_source_label(mbr_mouse_source_t source)
{
    static const char *const labels[MBR_MOUSE_SOURCE_COUNT] = {
        "LEFT", "RIGHT", "MIDDLE", "FORWARD", "BACKWARD"
    };
    return (unsigned)source < MBR_MOUSE_SOURCE_COUNT ? labels[source] : "LEFT";
}

const char *mbr_mouse_target_label(mbr_mouse_target_t target)
{
    static const char *const labels[MBR_MOUSE_TARGET_COUNT] = {
        "LEFT", "RIGHT", "MIDDLE", "ESCAPE", "FORWARD", "BACKWARD"
    };
    return (unsigned)target < MBR_MOUSE_TARGET_COUNT ? labels[target] : "LEFT";
}
