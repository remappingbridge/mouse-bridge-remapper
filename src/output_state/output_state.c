#include "mbr/output_state/output_state.h"
#include <string.h>
void mbr_output_clear(mbr_output_state_t *s) { memset(s,0,sizeof(*s)); }
void mbr_output_begin(MbrOutput *o,MouseSessionId session) {
 memset(o,0,sizeof(*o));o->session=session;
}
void mbr_output_release(MbrOutput *o) { memset(o,0,sizeof(*o));o->dirty=true; }
static int32_t bounded(int32_t a,int16_t b) {
 int32_t v=a+b;return v>MBR_MOTION_LIMIT?MBR_MOTION_LIMIT:v< -MBR_MOTION_LIMIT?-MBR_MOTION_LIMIT:v;
}
bool mbr_output_event(MbrOutput *o,const MbrMouseEvent *e,const MbrTarget mapping[5]) {
 if(!e||!o->session||e->session!=o->session)return false;
 if(e->type==MBR_MOUSE_BUTTON) {
  unsigned b=e->data.button.button;if(b>=5)return false;
  bool down=e->data.button.pressed;if(o->held[b]==down)return true;
  MbrTarget t=down?mapping[b]:o->owner[b];if(t>MBR_TARGET_BACKWARD)return false;
  o->held[b]=down;o->owner[b]=t;
  if(down)++o->refs[t];else if(o->refs[t])--o->refs[t];
  o->pending.buttons=(uint8_t)((o->refs[0]?1:0)|(o->refs[1]?2:0)|(o->refs[2]?4:0)|
   (o->refs[MBR_TARGET_FORWARD]?8:0)|(o->refs[MBR_TARGET_BACKWARD]?16:0));
  o->pending.escape=o->refs[MBR_TARGET_ESCAPE]!=0;
 } else if(e->type==MBR_MOUSE_MOVE) {
  o->pending.x=bounded(o->pending.x,e->data.move.dx);o->pending.y=bounded(o->pending.y,e->data.move.dy);
 } else if(e->type==MBR_MOUSE_WHEEL) {
  o->pending.wheel=bounded(o->pending.wheel,e->data.wheel.vertical);o->pending.pan=bounded(o->pending.pan,e->data.wheel.horizontal);
 } else return false;
 o->dirty=true;return true;
}
void mbr_output_consumed(MbrOutput *o) {
 o->pending.x=o->pending.y=o->pending.wheel=o->pending.pan=0;o->dirty=false;
}
