#include "mbr/application/bridge.h"
#include "mbr/usb_hid/usb_hid.h"
#include <assert.h>
static MbrBtQueue queue;static MbrUsbQueue usb;static MouseSessionId accepted,dropped;
static MbrProduct stored;static bool storage_fail,have_store,forget_fail;
bool mbr_product_load(MbrProduct *p){if(have_store){*p=stored;return true;}mbr_product_defaults(p);return false;}
bool mbr_product_save(const MbrProduct *p){if(storage_fail)return false;stored=*p;have_store=true;return true;}
void mbr_bt_registry(const MbrMouse *m,size_t n){(void)m;(void)n;}
bool mbr_bt_forget(const MbrMouse *m){(void)m;return !forget_fail;}
void mbr_bt_profile(MouseSessionId s,bool remapped){(void)s;(void)remapped;}
void mbr_bt_service(uint32_t now){(void)now;}
static uint32_t epoch;static bool usb_fail;static unsigned releases;
void mbr_bt_search(MbrSearch p,uint32_t tx) {(void)p;(void)tx;}
bool mbr_bt_next(MbrBtMessage *m,bool *overflow) {*overflow=queue.overflow;queue.overflow=false;return mbr_bt_pop(&queue,m);}
void mbr_bt_accept(MouseSessionId s) {accepted=s;}
void mbr_bt_disconnect(MouseSessionId s) {dropped=s;}
bool mbr_usb_submit(const mbr_output_state_t *s) {return !usb_fail&&mbr_usb_enqueue(&usb,s);}
void mbr_usb_release_all(void) {++releases;mbr_usb_release(&usb);}
uint32_t mbr_usb_epoch(void) {return epoch;}

static void click(MbrApp *a,MbrControl c){mbr_app_event(a,c,true);mbr_app_event(a,c,false);}
static void msg(MbrBridge *b,MbrApp *a,MbrBtKind kind,unsigned session,unsigned id){
 MbrBtMessage m={.kind=kind,.session=session,.transaction=a->search.generation,.mouse={.id=id,.name="MOUSE",.address={1,2,3,4,5,6}}};
 assert(mbr_bt_push(&queue,&m));mbr_bridge_task(b,a);
}
int main(void){
 MbrApp a;MbrBridge b;mbr_app_init(&a,0);mbr_bridge_init(&b,&a);
 msg(&b,&a,MBR_BT_READY,10,1);assert(b.transport==10&&stored.registry.count==1);
 mbr_app_home(&a);a.selection=1;click(&a,MBR_PRESS);a.selection=1;click(&a,MBR_PRESS);
 assert(a.screen==MBR_SCREEN_STANDARD_NOT_ACTIVE);
 storage_fail=true;click(&a,MBR_A);mbr_bridge_task(&b,&a);assert(a.registry.mice[0].profile==MBR_PASSTHROUGH&&a.screen==MBR_SCREEN_STANDARD_NOT_ACTIVE);
 storage_fail=false;click(&a,MBR_A);mbr_bridge_task(&b,&a);assert(a.registry.mice[0].profile==MBR_STANDARD&&stored.registry.mice[0].profile==MBR_STANDARD&&a.screen==MBR_SCREEN_STANDARD_ACTIVE);
 // Dirty draft persists without applying it to the live Custom template.
 a.screen=MBR_SCREEN_LEFT;a.source=0;a.selection=MBR_TARGET_ESCAPE;click(&a,MBR_A);mbr_bridge_task(&b,&a);
 assert(a.custom_dirty&&stored.dirty&&a.custom[0]==MBR_TARGET_LEFT);
 MbrApp boot;MbrBridge boot_bridge;mbr_app_init(&boot,100);mbr_bridge_init(&boot_bridge,&boot);
 assert(boot.registry.count==1&&boot.draft[0]==MBR_TARGET_ESCAPE&&boot.custom_dirty&&boot.custom[0]==MBR_TARGET_LEFT);
 click(&a,MBR_A);mbr_bridge_task(&b,&a);assert(a.registry.mice[0].profile==MBR_CUSTOM&&!a.custom_dirty&&a.custom[0]==MBR_TARGET_ESCAPE);
 // A continues while NEW is discovering; saved peers cannot win.
 mbr_app_home(&a);click(&a,MBR_PRESS);assert(a.screen==MBR_SCREEN_PAIR_NEW&&b.transport==10);
 msg(&b,&a,MBR_BT_READY,11,1);assert(b.transport==10&&dropped==11);
 MbrBtMessage press={.kind=MBR_BT_INPUT,.session=10,.input={.session=10,.type=MBR_MOUSE_BUTTON,.data.button={0,true}}};
 assert(mbr_bt_push(&queue,&press));mbr_bridge_task(&b,&a);assert(b.output.pending.escape);
 msg(&b,&a,MBR_BT_READY,12,2);assert(!b.transport&&b.closing==10&&b.pending==12&&!b.output.pending.escape&&stored.registry.count==1);
 assert(accepted==10);msg(&b,&a,MBR_BT_DISCONNECTED,10,0);
 assert(b.transport==12&&accepted==12&&a.sessions.live.mouse==2&&stored.registry.count==2);
 // A late old report/disconnect cannot affect B.
 assert(mbr_bt_push(&queue,&press));msg(&b,&a,MBR_BT_DISCONNECTED,10,0);assert(b.transport==12&&!b.output.pending.escape);
 // Remove disconnected A without affecting B. Credential failure is recoverable.
 a.screen=MBR_SCREEN_REMOVE_THIS;a.page=0;forget_fail=true;click(&a,MBR_A);mbr_bridge_task(&b,&a);
 assert(b.transport==12&&stored.pending_remove.id==1&&stored.registry.count==1&&a.registry.count==2);
 mbr_app_init(&boot,200);mbr_bridge_init(&boot_bridge,&boot);assert(boot.registry.count==1&&boot_bridge.product.pending_remove.id==1);
 forget_fail=false;a.now+=1001;mbr_bridge_task(&b,&a);assert(b.transport==12&&a.registry.count==1&&!stored.pending_remove.id);
 // Remove live last Mouse: wait for physical disconnect before cleanup success.
 a.screen=MBR_SCREEN_REMOVE_THIS;a.page=0;click(&a,MBR_A);mbr_bridge_task(&b,&a);assert(b.closing==12&&!b.transport&&stored.pending_remove.id==2);
 msg(&b,&a,MBR_BT_DISCONNECTED,12,0);assert(a.registry.count==0&&a.screen==MBR_SCREEN_SEARCHING_FIRST&&!stored.pending_remove.id);
 // Reconnect while Saved Devices is open must not steal the page.
 msg(&b,&a,MBR_BT_READY,20,3);mbr_app_home(&a);msg(&b,&a,MBR_BT_DISCONNECTED,20,0);
 a.screen=MBR_SCREEN_SAVED_DEVICES;msg(&b,&a,MBR_BT_READY,21,3);assert(a.screen==MBR_SCREEN_SAVED_DEVICES&&b.transport==21);
 // HOME search Help returns without resetting the eight-second deadline.
 mbr_app_home(&a);msg(&b,&a,MBR_BT_DISCONNECTED,21,0);uint32_t started=a.search.started;
 a.now+=2000;click(&a,MBR_X);click(&a,MBR_B);assert(a.search.started==started);
 return 0;
}
