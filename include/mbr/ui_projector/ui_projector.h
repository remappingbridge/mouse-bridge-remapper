#pragma once
#include "mbr/domain/screens.h"
typedef enum { MBR_TITLE, MBR_BODY, MBR_ACTION, MBR_WHITE, MBR_CYAN } MbrTone;
typedef struct { char character; MbrTone tone; } MbrCell;
typedef struct { MbrCell cells[9][21]; uint8_t hint; bool didactic; } MbrFrame;
typedef struct {
 MbrScreen screen; uint8_t selection; uint16_t pressed;
 char name[MBR_NAME_CAPACITY]; bool connected; MbrProfile profile;
 size_t page,count; MbrTarget draft[5]; bool custom_applied;
} MbrView;
void mbr_project(const MbrView *v,MbrFrame *f);
void mbr_frame_text(MbrFrame *f,unsigned row,unsigned column,const char *text,MbrTone tone);
bool mbr_screen_didactic(MbrScreen s);
