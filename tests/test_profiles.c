#include "mbr/remap/remap.h"
#include "mbr/logitech_hidpp/logitech_hidpp.h"
#include <assert.h>
int main(void){
 MbrTarget custom[]={MBR_TARGET_ESCAPE,MBR_TARGET_ESCAPE,MBR_TARGET_MIDDLE,MBR_TARGET_FORWARD,MBR_TARGET_BACKWARD},map[5];
 assert(mbr_profiles_mapping(MBR_STANDARD,custom,map));assert(map[0]==MBR_TARGET_FORWARD&&map[1]==MBR_TARGET_BACKWARD&&map[3]==MBR_TARGET_RIGHT&&map[4]==MBR_TARGET_LEFT);
 MbrOutput o;mbr_output_begin(&o,7);MbrMouseEvent e={.session=7,.type=MBR_MOUSE_BUTTON,.data.button={0,true}};
 assert(mbr_remap_event(&o,&e,MBR_STANDARD,custom)&&o.pending.buttons==16);e.data.button.pressed=false;assert(mbr_remap_event(&o,&e,MBR_STANDARD,custom)&&!o.pending.buttons);
 e.data.button.pressed=true;assert(mbr_remap_event(&o,&e,MBR_CUSTOM,custom)&&o.pending.escape);e.data.button.button=1;assert(mbr_remap_event(&o,&e,MBR_CUSTOM,custom));e.data.button.button=0;e.data.button.pressed=false;assert(mbr_remap_event(&o,&e,MBR_CUSTOM,custom)&&o.pending.escape);e.data.button.button=1;assert(mbr_remap_event(&o,&e,MBR_CUSTOM,custom)&&!o.pending.escape);
 mbr_logitech_hidpp_t h;mbr_logitech_hidpp_init(&h);mbr_logitech_hidpp_on_connect(&h);mbr_logitech_hidpp_set_forward_desired(&h,true);mbr_hidpp_output_t out;assert(mbr_logitech_hidpp_next_output(&h,&out));assert(out.payload[3]==0x1b&&out.payload[4]==4);mbr_logitech_hidpp_output_result(&h,out.kind,true);
 uint8_t feature[]={0xff,0,2,5};mbr_hidpp_input_result_t r;assert(mbr_logitech_hidpp_process_input(&h,0x11,feature,sizeof(feature),&r));assert(mbr_logitech_hidpp_next_output(&h,&out)&&out.payload[5]==3);mbr_logitech_hidpp_output_result(&h,out.kind,true);uint8_t ack[]={0xff,5,0x32};assert(mbr_logitech_hidpp_process_input(&h,0x11,ack,sizeof(ack),&r));assert(mbr_logitech_hidpp_claims_forward(&h));
 uint8_t held[]={0xff,5,0,0,0x56,0,0};assert(mbr_logitech_hidpp_process_input(&h,0x11,held,sizeof(held),&r)&&r.held_changed&&r.forward_held);assert(mbr_logitech_hidpp_process_input(&h,0x11,held,sizeof(held),&r)&&!r.held_changed);held[4]=0;assert(mbr_logitech_hidpp_process_input(&h,0x11,held,sizeof(held),&r)&&r.held_changed&&!r.forward_held);
 mbr_logitech_hidpp_set_forward_desired(&h,false);assert(mbr_logitech_hidpp_next_output(&h,&out)&&out.payload[5]==2);mbr_logitech_hidpp_output_result(&h,out.kind,true);assert(mbr_logitech_hidpp_process_input(&h,0x11,ack,sizeof(ack),&r));assert(!mbr_logitech_hidpp_claims_forward(&h));return 0;
}
