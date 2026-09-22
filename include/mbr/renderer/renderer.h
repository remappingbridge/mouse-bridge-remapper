#pragma once
#include "mbr/ui_projector/ui_projector.h"
#define MBR_LCD_WIDTH 240u
#define MBR_LCD_HEIGHT 240u
#define MBR_BLACK 0x0000u
#define MBR_DARK_MAGENTA 0x0801u
#define MBR_LIGHT_GRAY 0xc618u
uint16_t mbr_tone_rgb565(MbrTone tone);
unsigned mbr_text_y(const MbrFrame *f,unsigned row);
unsigned mbr_hint_boundary(const MbrFrame *f);
void mbr_render(const MbrFrame *f,uint16_t pixels[240*240]);
bool mbr_display_init(void);
void mbr_display_backlight(bool on);
void mbr_display_rows(unsigned y,unsigned count,const uint16_t *pixels);
