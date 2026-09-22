#include "mbr/output_state/output_state.h"
#include "mbr/bt_runtime/bt_runtime.h"
#include <assert.h>
static const MbrTarget map[5]={MBR_TARGET_ESCAPE,MBR_TARGET_ESCAPE,MBR_TARGET_MIDDLE,MBR_TARGET_LEFT,MBR_TARGET_LEFT};
static MbrMouseEvent button(unsigned b,bool down) {return (MbrMouseEvent){.session=7,.type=MBR_MOUSE_BUTTON,.data.button={(uint8_t)b,down}};}
int main(void) {
 MbrOutput o;mbr_output_begin(&o,7);MbrMouseEvent e=button(0,true);
 assert(mbr_output_event(&o,&e,map));assert(o.pending.escape&&o.refs[MBR_TARGET_ESCAPE]==1);
 assert(mbr_output_event(&o,&e,map));assert(o.refs[MBR_TARGET_ESCAPE]==1);
 e=button(1,true);assert(mbr_output_event(&o,&e,map));e=button(0,false);assert(mbr_output_event(&o,&e,map));assert(o.pending.escape);
 // Release uses captured ownership even if mapping changes before Up.
 MbrTarget changed[5]={MBR_TARGET_LEFT,MBR_TARGET_RIGHT};e=button(1,false);assert(mbr_output_event(&o,&e,changed));assert(!o.pending.escape);
 e=button(3,true);assert(mbr_output_event(&o,&e,map));e=button(4,true);assert(mbr_output_event(&o,&e,map));e=button(3,false);assert(mbr_output_event(&o,&e,map));assert(o.pending.buttons==1);
 e.session=6;assert(!mbr_output_event(&o,&e,map));assert(o.pending.buttons==1);
 e=(MbrMouseEvent){.session=7,.type=MBR_MOUSE_MOVE,.data.move={32767,-32768}};
 for(unsigned i=0;i<10000;++i){assert(mbr_output_event(&o,&e,map));}
 assert(o.pending.x==32767&&o.pending.y==-32767);
 e=(MbrMouseEvent){.session=7,.type=MBR_MOUSE_WHEEL,.data.wheel={32767,-32768}};
 for(unsigned i=0;i<10000;++i){assert(mbr_output_event(&o,&e,map));}
 assert(o.pending.wheel==32767&&o.pending.pan==-32767);
 mbr_output_consumed(&o);assert(o.pending.buttons==1&&o.pending.x==0&&!o.dirty);
 mbr_output_release(&o);assert(!o.session&&!o.pending.buttons&&!o.pending.escape&&o.dirty);assert(!mbr_output_event(&o,&e,map));
 MbrBtQueue q={0};MbrBtMessage in={.kind=MBR_BT_INPUT,.session=7},out;
 for(unsigned i=0;i<MBR_BT_QUEUE_CAPACITY-1;++i)assert(mbr_bt_push(&q,&in));
 assert(!mbr_bt_push(&q,&in)&&q.overflow);assert(!mbr_bt_pop(&q,&out));assert(!mbr_bt_push(&q,&in));
 q.overflow=false;assert(!mbr_bt_pop(&q,&out));in.session=8;assert(mbr_bt_push(&q,&in));assert(mbr_bt_pop(&q,&out)&&out.session==8);
 return 0;
}
