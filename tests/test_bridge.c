#include "mbr/application/bridge.h"
#include "mbr/usb_hid/usb_hid.h"
#include <assert.h>
static MbrBtQueue queue;static MbrUsbQueue usb;static MouseSessionId accepted,dropped;
static uint32_t epoch;static bool usb_fail;static unsigned releases;
void mbr_bt_search(MbrSearch p,uint32_t tx) {(void)p;(void)tx;}
bool mbr_bt_next(MbrBtMessage *m,bool *overflow) {*overflow=queue.overflow;queue.overflow=false;return mbr_bt_pop(&queue,m);}
void mbr_bt_accept(MouseSessionId s) {accepted=s;}
void mbr_bt_disconnect(MouseSessionId s) {dropped=s;}
bool mbr_usb_submit(const mbr_output_state_t *s) {return !usb_fail&&mbr_usb_enqueue(&usb,s);}
void mbr_usb_release_all(void) {++releases;mbr_usb_release(&usb);}
uint32_t mbr_usb_epoch(void) {return epoch;}
static void ready(MbrBridge *b,MbrApp *a,MouseSessionId session,uint32_t tx) {
 MbrBtMessage m={.kind=MBR_BT_READY,.session=session,.transaction=tx,.mouse={.id=1,.name="LIFT"}};
 assert(mbr_bt_push(&queue,&m));mbr_bridge_task(b,a);
}
static void button(MbrBridge *b,MbrApp *a,MouseSessionId session,bool down) {
 MbrBtMessage m={.kind=MBR_BT_INPUT,.session=session,.input={.session=session,.type=MBR_MOUSE_BUTTON,.data.button={0,down}}};
 assert(mbr_bt_push(&queue,&m));mbr_bridge_task(b,a);
}
int main(void) {
 MbrApp a;MbrBridge b={0};mbr_app_init(&a,0);
 ready(&b,&a,5,a.search.generation+1);assert(!b.transport&&dropped==5);
 ready(&b,&a,6,a.search.generation);assert(b.transport==6&&accepted==6&&a.sessions.live.ready);
 assert(a.screen==MBR_SCREEN_FIRST_MOUSE_CONNECTED);mbr_app_home(&a);
 a.locked=true;button(&b,&a,6,true);assert(b.output.pending.buttons==1&&a.locked);
 button(&b,&a,5,false);assert(b.output.pending.buttons==1); // stale generation
 MbrBtMessage disconnect={.kind=MBR_BT_DISCONNECTED,.session=6};assert(mbr_bt_push(&queue,&disconnect));mbr_bridge_task(&b,&a);
 assert(!a.sessions.live.ready&&!b.transport&&!b.output.pending.buttons&&releases);
 assert(a.screen==MBR_SCREEN_HOME_SEARCHING);
 MouseSessionId old=a.sessions.next_generation;ready(&b,&a,7,a.search.generation);
 assert(b.transport==7&&a.sessions.live.generation!=old);
 button(&b,&a,6,true);assert(!b.output.pending.buttons);
 usb_fail=true;button(&b,&a,7,true);assert(!b.transport&&!a.sessions.live.ready&&dropped==7);usb_fail=false;
 ready(&b,&a,8,a.search.generation);assert(b.transport==8);
 queue.overflow=true;mbr_bridge_task(&b,&a);assert(!b.transport&&!a.sessions.live.ready&&dropped==0);
 ready(&b,&a,9,a.search.generation);button(&b,&a,9,true);++epoch;mbr_bridge_task(&b,&a);assert(!b.transport&&!a.sessions.live.ready);
 // Application deadlines win over a late callback already queued by the radio.
 uint32_t expired=a.search.generation;mbr_app_tick(&a,a.now+MBR_SAVED_MS);ready(&b,&a,10,expired);assert(!b.transport&&dropped==10);
 return 0;
}
