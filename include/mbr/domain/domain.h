#ifndef MBR_DOMAIN_DOMAIN_H
#define MBR_DOMAIN_DOMAIN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t value;
} mbr_mouse_id_t;

typedef struct {
    mbr_mouse_id_t mouse_id;
    uint32_t generation;
} mbr_mouse_session_id_t;

typedef enum {
    MBR_MOUSE_BUTTON_LEFT = 0,
    MBR_MOUSE_BUTTON_RIGHT,
    MBR_MOUSE_BUTTON_MIDDLE,
    MBR_MOUSE_BUTTON_FORWARD,
    MBR_MOUSE_BUTTON_BACKWARD,
    MBR_MOUSE_BUTTON_COUNT
} mbr_mouse_button_t;

typedef enum {
    MBR_MOUSE_EVENT_BUTTON = 0,
    MBR_MOUSE_EVENT_MOVE,
    MBR_MOUSE_EVENT_WHEEL
} mbr_mouse_event_type_t;

typedef struct {
    mbr_mouse_event_type_t type;
    union {
        struct {
            mbr_mouse_button_t button;
            bool pressed;
        } button;
        struct {
            int16_t dx;
            int16_t dy;
        } move;
        struct {
            int16_t vertical;
            int16_t horizontal;
        } wheel;
    } data;
} mbr_mouse_event_t;

typedef enum {
    MBR_CONTROL_JOY_UP = 0,
    MBR_CONTROL_JOY_DOWN,
    MBR_CONTROL_JOY_LEFT,
    MBR_CONTROL_JOY_RIGHT,
    MBR_CONTROL_JOY_PRESS,
    MBR_CONTROL_KEY_A,
    MBR_CONTROL_KEY_B,
    MBR_CONTROL_KEY_X,
    MBR_CONTROL_KEY_Y,
    MBR_CONTROL_COUNT
} mbr_control_t;

typedef enum {
    MBR_PROFILE_PASSTHROUGH = 0,
    MBR_PROFILE_STANDARD,
    MBR_PROFILE_ESCAPE,
    MBR_PROFILE_CUSTOM,
    MBR_PROFILE_COUNT
} mbr_profile_kind_t;

typedef enum {
    MBR_MOUSE_SOURCE_LEFT = 0,
    MBR_MOUSE_SOURCE_RIGHT,
    MBR_MOUSE_SOURCE_MIDDLE,
    MBR_MOUSE_SOURCE_FORWARD,
    MBR_MOUSE_SOURCE_BACKWARD,
    MBR_MOUSE_SOURCE_COUNT
} mbr_mouse_source_t;

typedef enum {
    MBR_MOUSE_TARGET_LEFT = 0,
    MBR_MOUSE_TARGET_RIGHT,
    MBR_MOUSE_TARGET_MIDDLE,
    MBR_MOUSE_TARGET_ESCAPE,
    MBR_MOUSE_TARGET_FORWARD,
    MBR_MOUSE_TARGET_BACKWARD,
    MBR_MOUSE_TARGET_COUNT
} mbr_mouse_target_t;

typedef enum {
    MBR_SEARCH_NONE = 0,
    MBR_SEARCH_FIRST_MOUSE,
    MBR_SEARCH_SAVED,
    MBR_SEARCH_PAIR_NEW
} mbr_search_purpose_t;

enum {
    MBR_FIRST_MOUSE_DURATION_MS = 8000u,
    MBR_SEARCH_SAVED_DURATION_MS = 8000u,
    MBR_PAIR_NEW_DURATION_MS = 15000u,
    MBR_UI_SEMANTIC_WIDTH = 21u,
    MBR_UI_MAX_ROWS = 9u,
    MBR_UI_MAX_SPANS_PER_ROW = 6u,
    MBR_UX_MAX_EFFECTS = 4u
};

typedef enum {
    MBR_SCREEN_SEARCHING_FIRST = 0,
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
    MBR_SCREEN_LEARN_THE_KEYS,
    MBR_SCREEN_COUNT
} mbr_screen_id_t;

typedef enum {
    MBR_UI_COLOR_TITLE = 0,
    MBR_UI_COLOR_BODY,
    MBR_UI_COLOR_OPTION,
    MBR_UI_COLOR_WHITE,
    MBR_UI_COLOR_CYAN
} mbr_ui_color_t;

bool mbr_mouse_id_equal(mbr_mouse_id_t left, mbr_mouse_id_t right);
bool mbr_mouse_session_id_equal(mbr_mouse_session_id_t left,
                                mbr_mouse_session_id_t right);
const char *mbr_screen_name(mbr_screen_id_t screen);
const char *mbr_version(void);

#endif
