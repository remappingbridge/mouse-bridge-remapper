#include "mbr/ble_hogp/ble_hogp.h"
#include "mbr/bt_runtime/bt_runtime.h"
#include "btstack.h"
#include "mbr_hog_host.h"
#include "pico/cyw43_arch.h"
static MbrBtQueue queue;
static bool started;
static void lock(void) { async_context_acquire_lock_blocking(cyw43_arch_async_context());
  }
static void unlock(void) { async_context_release_lock(cyw43_arch_async_context());
  }
bool mbr_bt_start(void) {
 if(started)return true;
 if(cyw43_arch_init()!=0)return false;
 lock();
 l2cap_init();
 sm_init();
 sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
 sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION|SM_AUTHREQ_BONDING);
 gatt_client_init();
 att_server_init(profile_data,NULL,NULL);
 mbr_hogp_setup(mbr_bt_publish);
 hci_power_control(HCI_POWER_ON);
 started=true;
 unlock();
 return true;
}
bool mbr_bt_publish(const MbrBtMessage *m) { return mbr_bt_push(&queue,m);
  }
void mbr_bt_search(MbrSearch purpose,uint32_t tx) {
 if(!started)return;
 lock();
 static MbrSearch previous;
 if(previous==MBR_SEARCH_FIRST&&purpose==MBR_SEARCH_FIRST){
  /* FIRST cycles renew discovery ownership without rejecting an already-ready peer. */
  for(unsigned i=queue.read;i!=queue.write;i=(i+1)%MBR_BT_QUEUE_CAPACITY){
   MbrBtMessage *m=&queue.items[i];
   if(m->kind==MBR_BT_READY)m->transaction=tx;
  }
 }
 previous=purpose;
 mbr_hogp_search(purpose,tx);
 unlock();
}
bool mbr_bt_next(MbrBtMessage *m,bool *overflow) {
 *overflow=false;
 if(!started)return false;
 lock();
 *overflow=queue.overflow;
 queue.overflow=false;
 bool got=mbr_bt_pop(&queue,m);
 unlock();
 return got;
}
void mbr_bt_disconnect(MouseSessionId session) {
 if(!started)return;
 lock();
 mbr_hogp_disconnect(session);
 unlock();
}

void mbr_bt_accept(MouseSessionId session) {
 if(!started)return;
 lock();
 mbr_hogp_accept(session);
 unlock();
}
void mbr_bt_registry(const MbrMouse *mice,size_t count) {if(!started)return;
 lock();
 mbr_hogp_registry(mice,count);
 unlock();
 }
bool mbr_bt_forget(const MbrMouse *mouse) {if(!started)return false;
 lock();
 bool ok=mbr_hogp_forget(mouse);
 unlock();
 return ok;
 }
void mbr_bt_profile(MouseSessionId session,bool remapped) {
 if(!started)return;
 lock();
 /* Drop old-mapping input at the exact profile barrier; keep lifecycle messages. */
 static MbrBtQueue next;
 next=(MbrBtQueue){0};
 MbrBtMessage m;
 while(mbr_bt_pop(&queue,&m))if(m.kind!=MBR_BT_INPUT||m.session!=session)(void)mbr_bt_push(&next,&m);
 next.overflow=next.overflow||queue.overflow;
 queue=next;
 mbr_hogp_profile(session,remapped);
 unlock();
}
void mbr_bt_service(uint32_t now) {if(!started)return;
 lock();
 mbr_hogp_tick(now);
 unlock();
 }
