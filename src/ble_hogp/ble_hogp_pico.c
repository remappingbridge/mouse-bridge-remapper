/* Adapted from accepted BLU2USB G06; SDK types remain private to this owner. */
#include "mbr/ble_hogp/ble_hogp.h"
#include "btstack.h"
#include "ble/le_device_db.h"
#include <string.h>
typedef enum { OFF,IDLE,SCAN,CONNECT,SECURE,NAME,HIDS,READY,DISCONNECT } State;
typedef struct { MouseId id;bd_addr_t address;bd_addr_type_t type;char name[MBR_NAME_CAPACITY]; } Known;
static State state;
static bool working;
static MbrSearch wanted;
static uint32_t transaction,attempt;
static MouseSessionId generation;
static hci_con_handle_t handle=HCI_CON_HANDLE_INVALID;
static uint16_t cid;
static bd_addr_t remote;
static bd_addr_type_t remote_type;
static MbrMouse mouse;
static Known known[MBR_SAVED_CAPACITY];
static unsigned known_count;
static bd_addr_t rejected[4];
static bd_addr_type_t rejected_type[4];
static unsigned rejected_count,rejected_next;
static uint8_t descriptor[2048];
static MbrHogpParser parser;
static btstack_packet_callback_registration_t hci_registration,sm_registration;
static MbrHogpPublish publish;
static void start_search(void);
static void gatt_event(uint8_t,uint16_t,uint8_t *,uint16_t);
static bool status(MbrBtKind kind) {
 MbrBtMessage m={.kind=kind,.session=generation,.transaction=attempt,.mouse=mouse};return publish(&m);
}
static void drop(void) {
 if(state==DISCONNECT)return;
 if(state==READY)(void)status(MBR_BT_DISCONNECTED);
 if(state==SCAN)gap_stop_scan();
 if(state==CONNECT) {state=DISCONNECT;
  if(gap_connect_cancel()!=ERROR_CODE_SUCCESS) {state=IDLE;start_search();}
  return;}
 if(handle!=HCI_CON_HANDLE_INVALID) {state=DISCONNECT;gap_disconnect(handle);return;}
 state=IDLE;start_search();
}
static bool emit(void *context,const MbrMouseEvent *event) {
 (void)context;
 MbrBtMessage m={.kind=MBR_BT_INPUT,.session=generation,.input=*event};return publish(&m);
}
static MouseId identity(bd_addr_type_t type,const bd_addr_t address) {
 uint32_t id=2166136261u;id=(id^(uint8_t)type)*16777619u;
 for(unsigned i=0;i<6;++i)id=(id^address[i])*16777619u;
 return id?id:1;
}
static void resolve_identity(void) {
 int index=sm_le_device_index(handle),type=remote_type;bd_addr_t address;sm_key_t irk;
 memcpy(address,remote,sizeof(address));
 if(index>=0)le_device_db_info(index,&type,address,irk);
 mouse.id=identity((bd_addr_type_t)type,address);
 memcpy(remote,address,sizeof(remote));remote_type=(bd_addr_type_t)type;
 for(unsigned i=0;i<known_count;++i)if(known[i].id==mouse.id&&!mouse.name[0])memcpy(mouse.name,known[i].name,sizeof(mouse.name));
}
void mbr_hogp_accept(MouseSessionId session) {
 if(state!=READY||session!=generation)return;
 unsigned i=0;for(;i<known_count;++i)if(known[i].id==mouse.id)break;
 if(i==known_count) {if(known_count==MBR_SAVED_CAPACITY)return;++known_count;}
 known[i].id=mouse.id;known[i].type=remote_type;
 memcpy(known[i].address,remote,sizeof(remote));memcpy(known[i].name,mouse.name,sizeof(mouse.name));
}
static void start_search(void) {
 if(!working||state!=IDLE||wanted==MBR_SEARCH_NONE)return;
 attempt=transaction;
 memset(&mouse,0,sizeof(mouse));cid=0;
 if(++generation==0)++generation;
 if(wanted==MBR_SEARCH_SAVED) {
  unsigned added=0;(void)gap_whitelist_clear();(void)gap_load_resolving_list_from_le_device_db();
  for(unsigned i=0;i<known_count;++i)
   if(gap_whitelist_add(known[i].type,known[i].address)==ERROR_CODE_SUCCESS)++added;
  if(added&&gap_connect_with_whitelist()==ERROR_CODE_SUCCESS)state=CONNECT;
  return;
 }
 state=SCAN;gap_set_scan_parameters(1,48,48);gap_start_scan();
}
void mbr_hogp_search(MbrSearch purpose,uint32_t tx) {
 if(wanted==purpose&&transaction==tx) {start_search();return;}
 wanted=purpose;transaction=tx;
 /* Pair New replacement is MBR-07: never sacrifice the healthy current mouse. */
 if(state==READY||state==DISCONNECT||state==OFF)return;
 if(state==IDLE)start_search();else drop();
}
void mbr_hogp_disconnect(MouseSessionId session) {
 if(session==generation||session==0)drop();
}
static bool is_rejected(bd_addr_type_t type,const bd_addr_t address) {
 for(unsigned i=0;i<rejected_count;++i)if(rejected_type[i]==type&&!memcmp(rejected[i],address,6))return true;
 return false;
}
static void reject(void) {
 memcpy(rejected[rejected_next],remote,6);rejected_type[rejected_next]=remote_type;
 rejected_next=(rejected_next+1)%4;if(rejected_count<4)++rejected_count;
}
static void start_hids(void) {
 state=HIDS;
 if(hids_client_connect(handle,gatt_event,HID_PROTOCOL_MODE_REPORT,&cid)!=ERROR_CODE_SUCCESS)drop();
}
static void name_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size) {
 (void)channel;(void)size;if(type!=HCI_EVENT_PACKET||state!=NAME)return;
 switch(hci_event_packet_get_type(packet)) {
 case GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT: {
  if(gatt_event_characteristic_value_query_result_get_handle(packet)!=handle)return;
  unsigned n=gatt_event_characteristic_value_query_result_get_value_length(packet);
  if(n>=sizeof(mouse.name))n=sizeof(mouse.name)-1;
  memcpy(mouse.name,gatt_event_characteristic_value_query_result_get_value(packet),n);mouse.name[n]=0;
  break;
 }
 case GATT_EVENT_QUERY_COMPLETE:
  if(gatt_event_query_complete_get_handle(packet)==handle)start_hids();
  break;
 default:break;
 }
}
static void gatt_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size) {
 (void)channel;(void)size;
 if(type!=HCI_EVENT_PACKET||hci_event_packet_get_type(packet)!=HCI_EVENT_GATTSERVICE_META)return;
 switch(hci_event_gattservice_meta_get_subevent_code(packet)) {
 case GATTSERVICE_SUBEVENT_HID_SERVICE_CONNECTED: {
  if(state!=HIDS||gattservice_subevent_hid_service_connected_get_hids_cid(packet)!=cid)return;
  if(gattservice_subevent_hid_service_connected_get_status(packet)!=ERROR_CODE_SUCCESS) {drop();return;}
  const uint8_t *map=hids_client_descriptor_storage_get_descriptor_data(cid,0);
  uint16_t length=hids_client_descriptor_storage_get_descriptor_len(cid,0);
  if(gattservice_subevent_hid_service_connected_get_num_instances(packet)!=1||!mbr_hogp_configure(&parser,generation,map,length)) {reject();drop();return;}
  state=READY;if(!status(MBR_BT_READY))drop();
  break;
 }
 case GATTSERVICE_SUBEVENT_HID_SERVICE_DISCONNECTED:
  if(gattservice_subevent_hid_service_disconnected_get_hids_cid(packet)==cid&&state!=DISCONNECT)drop();
  break;
 case GATTSERVICE_SUBEVENT_HID_REPORT: {
  if(state!=READY||gattservice_subevent_hid_report_get_hids_cid(packet)!=cid)return;
  uint8_t id=gattservice_subevent_hid_report_get_report_id(packet);
  if(!mbr_hogp_parse(&parser,id,gattservice_subevent_hid_report_get_report(packet),
    gattservice_subevent_hid_report_get_report_len(packet),emit,NULL))drop();
  break;
 }
 default:break;
 }
}
static void hci_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size) {
 (void)channel;(void)size;if(type!=HCI_EVENT_PACKET)return;
 switch(hci_event_packet_get_type(packet)) {
 case BTSTACK_EVENT_STATE:
  if(!working&&btstack_event_state_get_state(packet)==HCI_STATE_WORKING) {working=true;state=IDLE;start_search();}break;
 case GAP_EVENT_ADVERTISING_REPORT: {
  if(state!=SCAN)return;
  bd_addr_t address;gap_event_advertising_report_get_address(packet,address);
  bd_addr_type_t at=gap_event_advertising_report_get_address_type(packet);
  if(is_rejected(at,address))return;
  uint8_t n=gap_event_advertising_report_get_data_length(packet);
  const uint8_t *data=gap_event_advertising_report_get_data(packet);
  if(!ad_data_contains_uuid16(n,data,ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE))return;
  unsigned appearance=0;ad_context_t ad;
  for(ad_iterator_init(&ad,n,(uint8_t *)data);ad_iterator_has_more(&ad);ad_iterator_next(&ad)) {
   if(ad_iterator_get_data_type(&ad)==BLUETOOTH_DATA_TYPE_APPEARANCE&&ad_iterator_get_data_len(&ad)>=2)
    appearance=little_endian_read_16(ad_iterator_get_data(&ad),0);
  }
  if(appearance>=960&&appearance<=1023&&appearance!=960&&appearance!=962)return;
  memcpy(remote,address,6);remote_type=at;gap_stop_scan();state=CONNECT;
  if(gap_connect(remote,remote_type)!=ERROR_CODE_SUCCESS) {state=IDLE;start_search();}break;
 }
 case HCI_EVENT_META_GAP:
  if(hci_event_gap_meta_get_subevent_code(packet)==GAP_SUBEVENT_LE_CONNECTION_COMPLETE&&(state==CONNECT||state==DISCONNECT)) {
   bool canceled=state==DISCONNECT;
   if(gap_subevent_le_connection_complete_get_status(packet)!=ERROR_CODE_SUCCESS) {state=IDLE;start_search();
  break;}
   handle=gap_subevent_le_connection_complete_get_connection_handle(packet);
   gap_subevent_le_connection_complete_get_peer_address(packet,remote);
   remote_type=gap_subevent_le_connection_complete_get_peer_address_type(packet);
   state=SECURE;if(canceled) {drop();
  break;}sm_request_pairing(handle);
  }break;
 case HCI_EVENT_DISCONNECTION_COMPLETE:
  if(hci_event_disconnection_complete_get_connection_handle(packet)!=handle)return;
  if(state==READY)(void)status(MBR_BT_DISCONNECTED);
  handle=HCI_CON_HANDLE_INVALID;cid=0;memset(&parser,0,sizeof(parser));state=IDLE;
  /* Application determines whether the current page allows saved search. */
  wanted=MBR_SEARCH_NONE;
  break;
 default:break;
 }
}
static void sm_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size) {
 (void)channel;(void)size;if(type!=HCI_EVENT_PACKET||handle==HCI_CON_HANDLE_INVALID)return;
 bool ready=false;
 switch(hci_event_packet_get_type(packet)) {
 case SM_EVENT_JUST_WORKS_REQUEST:
  if(state==SECURE&&sm_event_just_works_request_get_handle(packet)==handle)sm_just_works_confirm(handle);
  break;
 case SM_EVENT_PAIRING_COMPLETE:
  if(state!=SECURE||sm_event_pairing_complete_get_handle(packet)!=handle)return;
  if(sm_event_pairing_complete_get_status(packet)==ERROR_CODE_SUCCESS)ready=true;else drop();
  break;
 case SM_EVENT_REENCRYPTION_COMPLETE:
  if(state!=SECURE||sm_event_reencryption_complete_get_handle(packet)!=handle)return;
  if(sm_event_reencryption_complete_get_status(packet)==ERROR_CODE_SUCCESS)ready=true;else drop();
  break;
 default:break;
 }
 if(ready) {
  resolve_identity();state=NAME;
  if(gatt_client_read_value_of_characteristics_by_uuid16(name_event,handle,1,0xffff,ORG_BLUETOOTH_CHARACTERISTIC_GAP_DEVICE_NAME)!=ERROR_CODE_SUCCESS)start_hids();
 }
}
void mbr_hogp_setup(MbrHogpPublish callback) {
 publish=callback;state=OFF;hids_client_init(descriptor,sizeof(descriptor));
 hci_registration.callback=hci_event;hci_add_event_handler(&hci_registration);
 sm_registration.callback=sm_event;sm_add_event_handler(&sm_registration);
}
