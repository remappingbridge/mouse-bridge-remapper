#pragma once
#include "mbr/domain/domain.h"
#define MBR_MOTION_LIMIT 32767
/* Held ownership is restricted to five physical buttons of one session. */
typedef struct {
 MouseSessionId session; bool held[5]; MbrTarget owner[5]; uint8_t refs[6];
 mbr_output_state_t pending; bool dirty;
} MbrOutput;
void mbr_output_clear(mbr_output_state_t *s);
void mbr_output_begin(MbrOutput *o,MouseSessionId session);
void mbr_output_release(MbrOutput *o);
bool mbr_output_event(MbrOutput *o,const MbrMouseEvent *e,const MbrTarget mapping[5]);
void mbr_output_consumed(MbrOutput *o);
