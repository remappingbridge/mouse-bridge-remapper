#include "mbr/domain/domain.h"

#ifndef MBR_VERSION_STRING
#define MBR_VERSION_STRING "0.1.0-mbr01"
#endif

static const char *const screen_names[MBR_SCREEN_COUNT] = {
    "searching-first",
    "first-mouse-connected",
    "home-searching",
    "home-searching-help",
    "home-retry",
    "home-retry-help",
    "pair-new",
    "help-pair-new",
    "retry-pair-new",
    "help-retry-pair-new",
    "home-connected",
    "help-home-connected",
    "remapper-options",
    "help-remapper-options",
    "passthrough-active",
    "passthrough-not-active",
    "standard-not-active",
    "standard-active",
    "escape-not-active",
    "escape-active",
    "custom-edit",
    "left",
    "right",
    "middle",
    "forward",
    "backward",
    "saved-devices",
    "remove-this",
    "help-remove-this",
    "learn-the-keys",
};

bool mbr_mouse_id_equal(mbr_mouse_id_t left, mbr_mouse_id_t right)
{
    return left.value == right.value;
}

bool mbr_mouse_session_id_equal(mbr_mouse_session_id_t left,
                                mbr_mouse_session_id_t right)
{
    return mbr_mouse_id_equal(left.mouse_id, right.mouse_id) &&
           left.generation == right.generation;
}

const char *mbr_screen_name(mbr_screen_id_t screen)
{
    if ((unsigned)screen >= MBR_SCREEN_COUNT) return "invalid";
    return screen_names[screen];
}

const char *mbr_version(void)
{
    return MBR_VERSION_STRING;
}
