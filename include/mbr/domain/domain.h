#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define MBR_SAVED_CAPACITY 16u
#define MBR_NAME_CAPACITY 64u
#define MBR_FIRST_MS 8000u
#define MBR_SAVED_MS 8000u
#define MBR_PAIR_MS 15000u
typedef uint32_t MouseId;
typedef uint32_t MouseSessionId;
typedef enum { MBR_UP, MBR_DOWN, MBR_LEFT, MBR_RIGHT, MBR_PRESS, MBR_A, MBR_B, MBR_X, MBR_Y, MBR_CONTROL_COUNT } MbrControl;
typedef enum { MBR_PASSTHROUGH, MBR_STANDARD, MBR_ESCAPE, MBR_CUSTOM } MbrProfile;
typedef enum { MBR_TARGET_LEFT, MBR_TARGET_RIGHT, MBR_TARGET_MIDDLE, MBR_TARGET_ESCAPE, MBR_TARGET_FORWARD, MBR_TARGET_BACKWARD } MbrTarget;
typedef struct { MouseId id; char name[MBR_NAME_CAPACITY]; MbrProfile profile; } MbrMouse;
typedef struct { MouseId mouse; MouseSessionId generation; bool ready; } MbrSession;
typedef enum { MBR_SEARCH_NONE, MBR_SEARCH_FIRST, MBR_SEARCH_SAVED, MBR_SEARCH_NEW } MbrSearch;
typedef struct { uint8_t buttons; bool escape; int32_t x,y,wheel,pan; } mbr_output_state_t;
void mbr_display_name(const char *name, char out[22]);

/* Canonical transport events carry the connection generation, never a pointer. */
typedef enum { MBR_MOUSE_BUTTON, MBR_MOUSE_MOVE, MBR_MOUSE_WHEEL } MbrMouseEventKind;
typedef struct {
 MouseSessionId session; MbrMouseEventKind type;
 union {
  struct { uint8_t button; bool pressed; } button;
  struct { int16_t dx,dy; } move;
  struct { int16_t vertical,horizontal; } wheel;
 } data;
} MbrMouseEvent;
void mbr_home_title(const char *name,char out[22]);
