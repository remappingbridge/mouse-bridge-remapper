#include "btstack.h"
#include "ble/le_device_db.h"
#include "mbr/ble_hogp/ble_hogp.h"
#include "mbr/usb_hid/usb_hid.h"
#include <assert.h>
#include <string.h>
static btstack_packet_handler_t hci_cb,sm_cb,gatt_cb;
static unsigned scans,connects,cancels,disconnects,pairs;
static MbrBtMessage messages[32];static unsigned count;
static uint16_t next_cid=1;static uint8_t map[512];static size_t map_len;
static uint8_t db_type=BD_ADDR_TYPE_UNKNOWN;static bd_addr_t db_addr;
void hci_add_event_handler(btstack_packet_callback_registration_t *r){hci_cb=r->callback;}
void sm_add_event_handler(btstack_packet_callback_registration_t *r){sm_cb=r->callback;}
void gap_set_scan_parameters(uint8_t t,uint16_t i,uint16_t w){assert(t==1&&i==w);}
void gap_start_scan(void){++scans;}
void gap_stop_scan(void){}
uint8_t gap_connect(const bd_addr_t a,bd_addr_type_t t){(void)a;(void)t;++connects;return 0;}
uint8_t gap_connect_cancel(void){++cancels;return 0;}
uint8_t gap_disconnect(hci_con_handle_t h){(void)h;++disconnects;return 0;}
uint8_t gap_load_resolving_list_from_le_device_db(void){return 0;}
void sm_request_pairing(hci_con_handle_t h){(void)h;++pairs;}
void sm_just_works_confirm(hci_con_handle_t h){(void)h;}
int sm_le_device_index(hci_con_handle_t h){(void)h;return db_type==BD_ADDR_TYPE_UNKNOWN?-1:0;}
int le_device_db_max_count(void){return 1;}
void le_device_db_info(int i,int *t,bd_addr_t a,sm_key_t k){assert(i==0);*t=db_type;memcpy(a,db_addr,6);memset(k,0,16);}
void le_device_db_remove(int i){assert(i==0);db_type=BD_ADDR_TYPE_UNKNOWN;}
void hids_client_init(uint8_t *p,uint16_t n){(void)p;assert(n>=4096);}
uint8_t hids_client_connect(hci_con_handle_t h,btstack_packet_handler_t cb,hid_protocol_mode_t mode,uint16_t *cid){(void)h;assert(mode==HID_PROTOCOL_MODE_REPORT);gatt_cb=cb;*cid=next_cid++;return 0;}
const uint8_t *hids_client_descriptor_storage_get_descriptor_data(uint16_t c,uint8_t i){(void)c;assert(i==0);return map;}
uint16_t hids_client_descriptor_storage_get_descriptor_len(uint16_t c,uint8_t i){(void)c;(void)i;return (uint16_t)map_len;}
uint8_t hids_client_send_write_report(uint16_t c,uint8_t id,hid_report_type_t t,const uint8_t *p,uint8_t n){(void)c;(void)id;(void)t;(void)p;(void)n;return 0;}
uint8_t gatt_client_read_value_of_characteristics_by_uuid16(btstack_packet_handler_t cb,hci_con_handle_t h,uint16_t s,uint16_t e,uint16_t u){(void)cb;(void)h;(void)s;(void)e;(void)u;return 1;}
static bool sink(const MbrBtMessage *m){assert(count<32);messages[count++]=*m;return true;}
static void send(btstack_packet_handler_t cb,uint8_t *p,unsigned n){cb(HCI_EVENT_PACKET,0,p,(uint16_t)n);}
static void adv(uint8_t event,uint8_t type,uint8_t addr,const uint8_t *data,unsigned n){
 uint8_t p[80]={GAP_EVENT_ADVERTISING_REPORT,0,event,type,addr,0,0,0,0,0,0,0};p[1]=(uint8_t)(10+n);p[11]=(uint8_t)n;memcpy(p+12,data,n);send(hci_cb,p,n+12);
}
static void connection(unsigned handle,uint8_t type,uint8_t addr){uint8_t p[40]={HCI_EVENT_META_GAP,38,GAP_SUBEVENT_LE_CONNECTION_COMPLETE,0};p[4]=(uint8_t)handle;p[7]=type;p[8]=addr;send(hci_cb,p,sizeof(p));}
static void security(unsigned handle){uint8_t p[20]={SM_EVENT_PAIRING_COMPLETE,18};p[2]=(uint8_t)handle;send(sm_cb,p,sizeof(p));}
static void ready(unsigned cid){uint8_t p[]={HCI_EVENT_GATTSERVICE_META,6,GATTSERVICE_SUBEVENT_HID_SERVICE_CONNECTED,(uint8_t)cid,0,0,1,1};send(gatt_cb,p,sizeof(p));}
static void disconnected(unsigned handle){uint8_t p[]={HCI_EVENT_DISCONNECTION_COMPLETE,4,0,(uint8_t)handle,0,0};send(hci_cb,p,sizeof(p));}
int main(void){
 size_t n;const uint8_t *d=mbr_usb_report_descriptor(0,&n);memcpy(map,d,n);map_len=n;
 mbr_hogp_setup(sink);uint8_t working[]={BTSTACK_EVENT_STATE,1,HCI_STATE_WORKING};send(hci_cb,working,sizeof(working));
 mbr_hogp_search(MBR_SEARCH_FIRST,1);assert(scans==1);
 // HID UUID arriving in scan response must combine with initial connectable advertisement.
 uint8_t flags[]={2,1,6};adv(0,0,1,flags,sizeof(flags));assert(connects==0);
 uint8_t hid[]={3,3,0x12,0x18};adv(4,0,1,hid,sizeof(hid));assert(connects==1);
 // Eight-second FIRST renewal must retain in-progress connection/security/discovery.
 mbr_hogp_tick(8000);mbr_hogp_search(MBR_SEARCH_FIRST,2);assert(!cancels);
 connection(40,0,1);assert(pairs==1);security(40);ready(1);assert(count==1&&messages[0].kind==MBR_BT_READY&&messages[0].transaction==2);
 unsigned a=messages[0].session;MbrMouse saved=messages[0].mouse;mbr_hogp_accept(a);mbr_hogp_registry(&saved,1);
 mbr_hogp_search(MBR_SEARCH_NEW,3);unsigned attempts=connects;
 adv(0,2,1,hid,sizeof(hid));assert(connects==attempts); // identity-address aliases are saved, never NEW.
 uint8_t appearance[]={3,0x19,0xc2,3};adv(0,0,2,appearance,sizeof(appearance));assert(connects==attempts+1&&!disconnects);
 connection(41,0,2);security(41);ready(2);assert(count==2&&messages[1].session!=a);
 mbr_hogp_accept(messages[1].session); // cannot make candidate authoritative while old is live
 mbr_hogp_search(MBR_SEARCH_NONE,4);assert(disconnects==1); // closes only candidate
 disconnected(41);mbr_hogp_disconnect(a);assert(disconnects==2);disconnected(40);
 // Bonded advertiser with identity type 2 is matched against saved public type 0.
 mbr_hogp_tick(8200);mbr_hogp_search(MBR_SEARCH_SAVED,5);adv(0,2,1,flags,sizeof(flags));assert(connects==attempts+2);
 connection(42,2,1);security(42);ready(3);assert(messages[count-1].mouse.id==saved.id);mbr_hogp_accept(messages[count-1].session);
 // Cancel during CONNECTING and reject late successful completion safely.
 mbr_hogp_search(MBR_SEARCH_NEW,6);adv(0,0,3,hid,sizeof(hid));unsigned before=disconnects;
 mbr_hogp_search(MBR_SEARCH_NONE,7);assert(cancels==1);connection(43,0,3);assert(disconnects==before+1);disconnected(43);
 // Malformed/Keyboard map never reaches READY.
 mbr_hogp_tick(8400);mbr_hogp_search(MBR_SEARCH_NEW,8);adv(0,0,4,hid,sizeof(hid));connection(44,0,4);security(44);
 map[3]=6;unsigned old_count=count;ready(4);assert(count==old_count&&disconnects==before+2);disconnected(44);
 // Credential removal is idempotent.
 db_type=0;memcpy(db_addr,saved.address,6);assert(mbr_hogp_forget(&saved)&&db_type==BD_ADDR_TYPE_UNKNOWN);assert(mbr_hogp_forget(&saved));
 return 0;
}
