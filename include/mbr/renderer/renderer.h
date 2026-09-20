#ifndef MBR_RENDERER_RENDERER_H
#define MBR_RENDERER_RENDERER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "mbr/ui_projector/ui_projector.h"

#define MBR_RENDERER_WIDTH 240u
#define MBR_RENDERER_HEIGHT 240u
#define MBR_RENDERER_TEXT_ROWS 9u
#define MBR_RENDERER_TEXT_COLS 21u
#define MBR_RENDERER_GLYPH_SCALE 2u
#define MBR_RENDERER_GLYPH_WIDTH 10u
#define MBR_RENDERER_GLYPH_HEIGHT 14u
#define MBR_RENDERER_CHAR_ADVANCE 11u
#define MBR_RENDERER_LINE_ADVANCE 27u
#define MBR_RENDERER_TEXT_X 7u
#define MBR_RENDERER_TEXT_Y 8u
#define MBR_RENDERER_TITLE_BODY_GAP 17u
#define MBR_RENDERER_BODY_LINE_GAP 12u
#define MBR_RENDERER_HINT_TOP_GAP 11u
#define MBR_RENDERER_HINT_LINE_GAP 12u
#define MBR_RENDERER_HINT_BOTTOM_GAP 12u
#define MBR_RENDERER_LEARN_LINE_GAP 11u

#define MBR_COLOR_BLACK 0x0000u
#define MBR_COLOR_WHITE 0xffffu
#define MBR_COLOR_MAGENTA 0xf81fu
#define MBR_COLOR_CYAN 0x07ffu
#define MBR_COLOR_OFF_WHITE_YELLOW 0xffb8u
#define MBR_COLOR_LIGHT_GRAY 0xc618u
#define MBR_COLOR_DARK_MAGENTA 0x0801u

typedef struct {
    void *context;
    bool (*fill_rect)(void *context, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t rgb565);
    bool (*write_rgb565)(void *context, uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *pixels);
} mbr_display_hal_t;

uint16_t mbr_renderer_tone_rgb565(mbr_ui_color_t color);
uint16_t mbr_renderer_background_rgb565(const mbr_ui_frame_t *frame, size_t row);
uint16_t mbr_renderer_separator_boundary_y(const mbr_ui_frame_t *frame);
uint16_t mbr_renderer_text_y(const mbr_ui_frame_t *frame, size_t row);
bool mbr_renderer_render(const mbr_display_hal_t *display, const mbr_ui_frame_t *frame);

bool mbr_renderer_init(mbr_display_hal_t *display);
void mbr_renderer_set_backlight(bool enabled);

#endif