#include "mbr/ble_hogp/ble_hogp.h"
#include "mbr/bt_runtime/bt_runtime.h"
#include "btstack.h"
#include "mbr_hog_host.h"
#include "pico/cyw43_arch.h"
static MbrBtQueue queue;
static bool started;
static void lock(void) { async_context_acquire_lock_blocking(cyw43_arch_async_context()); }
static void unlock(void) { async_context_release_lock(cyw43_arch_async_context()); }
bool mbr_bt_start(void) {
 if(started)return true;
 if(cyw43_arch_init()!=0)return false;
 lock();
 l2cap_init();sm_init();sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
 sm_set_authentication_requirements(SM_AUTHREQ_SECURE_CONNECTION|SM_AUTHREQ_BONDING);
 gatt_client_init();att_server_init(profile_data,NULL,NULL);mbr_hogp_setup(mbr_bt_publish);
 hci_power_control(HCI_POWER_ON);started=true;unlock();return true;
}
bool mbr_bt_publish(const MbrBtMessage *m) { return mbr_bt_push(&queue,m); }
void mbr_bt_search(MbrSearch purpose,uint32_t tx) {
 if(!started)return;
 lock();mbr_hogp_search(purpose,tx);unlock();
}
bool mbr_bt_next(MbrBtMessage *m,bool *overflow) {
 *overflow=false;if(!started)return false;
 lock();*overflow=queue.overflow;queue.overflow=false;
 bool got=mbr_bt_pop(&queue,m);unlock();return got;
}
void mbr_bt_disconnect(MouseSessionId session) {
 if(!started)return;
 lock();mbr_hogp_disconnect(session);unlock();
}

void mbr_bt_accept(MouseSessionId session) {
 if(!started)return;
 lock();mbr_hogp_accept(session);unlock();
}
