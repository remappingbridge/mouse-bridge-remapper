#include "mbr/usb_hid/usb_hid.h"
#include <string.h>
static int8_t clip(int32_t n) { return (int8_t)(n>127?127:n< -127?-127:n); }
void mbr_usb_reports(const mbr_output_state_t *s,MbrMouseReport *m,MbrEscapeReport *e) {
 *m=(MbrMouseReport){{s->buttons&31u,(uint8_t)clip(s->x),(uint8_t)clip(s->y),(uint8_t)clip(s->wheel),(uint8_t)clip(s->pan)}};
 *e=(MbrEscapeReport){{0,0,s->escape?0x29:0,0,0,0,0,0}};
}
void mbr_usb_release(MbrUsbQueue *q) { memset(q,0,sizeof(*q));q->write=1; }
bool mbr_usb_enqueue(MbrUsbQueue *q,const mbr_output_state_t *s) {
 uint8_t next=(uint8_t)((q->write+1)%32);
 if(next==q->read) { mbr_usb_release(q); return false; }
 q->queue[q->write]=*s; q->write=next; return true;
}
void mbr_usb_drain(MbrUsbQueue *q,MbrUsbSend send,void *ctx) {
 if(q->read==q->write) return;
 mbr_output_state_t *s=&q->queue[q->read];MbrMouseReport m;MbrEscapeReport e;mbr_usb_reports(s,&m,&e);
 if(!q->mouse_done && send(ctx,0,m.bytes,sizeof(m.bytes))) {
  s->x-=(int8_t)m.bytes[1];s->y-=(int8_t)m.bytes[2];s->wheel-=(int8_t)m.bytes[3];s->pan-=(int8_t)m.bytes[4];
  q->last_mouse=m; q->mouse_done=true;
 }
 if(!q->escape_done && send(ctx,1,e.bytes,sizeof(e.bytes))) { q->last_escape=e;q->escape_done=true; }
 if(q->mouse_done&&q->escape_done) {
  if(!s->x&&!s->y&&!s->wheel&&!s->pan) q->read=(uint8_t)((q->read+1)%32);
  q->mouse_done=false;q->escape_done=false;
 }
}
