#include "mbr/application/bridge.h"
#include "mbr/usb_hid/usb_hid.h"
static const MbrTarget passthrough[5]={MBR_TARGET_LEFT,MBR_TARGET_RIGHT,MBR_TARGET_MIDDLE,MBR_TARGET_FORWARD,MBR_TARGET_BACKWARD};
void mbr_bridge_release(MbrBridge *b,MbrApp *a) {
 MouseSessionId transport=b->transport,session=b->output.session;b->transport=0;
 mbr_output_release(&b->output);mbr_usb_release_all();
 if(transport) {mbr_app_disconnected(a,session);mbr_bt_disconnect(transport);}
}
void mbr_bridge_task(MbrBridge *b,MbrApp *a) {
 uint32_t epoch=mbr_usb_epoch();
 if(epoch!=b->usb_epoch) {b->usb_epoch=epoch;mbr_bridge_release(b,a);}
 mbr_bt_search(a->search.purpose,a->search.generation);
 for(unsigned i=0;i<MBR_BT_QUEUE_CAPACITY;++i) {
  MbrBtMessage m;bool overflow=false;bool got=mbr_bt_next(&m,&overflow);
  if(overflow) {mbr_bridge_release(b,a);mbr_bt_disconnect(0);break;}
  if(!got)break;
  if(m.kind==MBR_BT_READY) {
   if(b->transport||a->sessions.live.ready||!mbr_app_ready(a,m.transaction,&m.mouse)) {mbr_bt_disconnect(m.session);continue;}
   /* MBR-05 has a RAM registry only; persistence/handoff belong to later gates. */
   if(!mbr_app_confirm(a,a->request.token,true)) {mbr_bt_disconnect(m.session);continue;}
   b->transport=m.session;mbr_output_begin(&b->output,a->sessions.live.generation);mbr_bt_accept(m.session);
  } else if(m.session==b->transport&&b->transport) {
   if(m.kind==MBR_BT_DISCONNECTED) {mbr_bridge_release(b,a);continue;}
   if(m.kind!=MBR_BT_INPUT||m.input.session!=m.session)continue;
   m.input.session=b->output.session;
   if(!mbr_output_event(&b->output,&m.input,passthrough)) {mbr_bridge_release(b,a);continue;}
   if(b->output.dirty) {
    if(!mbr_usb_submit(&b->output.pending)) {mbr_bridge_release(b,a);continue;}
    mbr_output_consumed(&b->output);
   }
  }
 }
 /* Do not acknowledge profile/storage/remove effects before their gates. */
 if(a->request.kind!=MBR_OP_NONE&&a->request.kind!=MBR_OP_HANDOFF)
  (void)mbr_app_confirm(a,a->request.token,false);
}
