/* One authoritative Mouse plus one qualification candidate. SDK objects stay here. */
#include "mbr/ble_hogp/ble_hogp.h"
#include "mbr/logitech_hidpp/logitech_hidpp.h"
#include "btstack.h"
#include "ble/le_device_db.h"
#include <string.h>
typedef enum { FREE,CONNECTING,SECURING,READING_NAME,DISCOVERING,READY,CLOSING } PeerState;
typedef struct {
 PeerState state;
 MouseSessionId generation;
 uint32_t transaction,started,hidpp_sent;
 hci_con_handle_t handle;
 uint16_t cid;
 MbrMouse mouse;
 MbrHogpParser parser;
 mbr_logitech_hidpp_t hidpp;
 bool authoritative,repaired,hidpp_capable,name_complete;
} Peer;
static Peer peers[2];
static MbrMouse known[MBR_SAVED_CAPACITY];
 static size_t known_count;
static MbrSearch wanted;
 static uint32_t transaction,now,retry_at;
 static MouseSessionId serial;
static bool working,scanning;
 static Peer *connecting;
static uint8_t descriptors[8192];
static MbrHogpPublish publish;
static btstack_packet_callback_registration_t hci_registration,sm_registration;
typedef struct {uint8_t address[6],type;
 uint32_t until;
 bool used;
 } Rejected;
static Rejected rejected[8];
 static unsigned rejected_next;
typedef struct {uint8_t address[6],type;
 bool used,connectable,hid,name_complete;
 char name[MBR_NAME_CAPACITY];
 } Advert;
static Advert adverts[8];
 static unsigned advert_next;
static void start_search(void);
static void gatt_event(uint8_t,uint16_t,uint8_t *,uint16_t);
static Peer *by_handle(hci_con_handle_t h){for(unsigned i=0;i<2;++i)if(peers[i].state!=FREE&&peers[i].handle==h)return &peers[i];
 return NULL;
 }
static Peer *by_cid(uint16_t c){for(unsigned i=0;i<2;++i)if(c&&peers[i].state!=FREE&&peers[i].cid==c)return &peers[i];
 return NULL;
 }
static Peer *by_session(MouseSessionId s){for(unsigned i=0;i<2;++i)if(peers[i].state!=FREE&&peers[i].generation==s)return &peers[i];
 return NULL;
 }
static int saved_id(MouseId id){for(size_t i=0;i<known_count;++i)if(known[i].id==id)return (int)i;
 return -1;
 }
static int saved_address(uint8_t type,const uint8_t *address){for(size_t i=0;i<known_count;++i)if((known[i].address_type&1u)==(type&1u)&&!memcmp(known[i].address,address,6))return (int)i;
 return -1;
 }
static bool status(Peer *p,MbrBtKind kind){MbrBtMessage m={.kind=kind,.session=p->generation,.transaction=p->transaction,.mouse=p->mouse};
 return publish(&m);
 }
static void cool_down(Peer *p,uint32_t duration){Rejected *r=&rejected[rejected_next++%8];
 memcpy(r->address,p->mouse.address,6);
 r->type=p->mouse.address_type;
 r->until=now+duration;
 r->used=true;
 }
static bool rejected_address(uint8_t type,const uint8_t *address){for(unsigned i=0;i<8;++i)if(rejected[i].used&&rejected[i].type==type&&!memcmp(rejected[i].address,address,6)&&(int32_t)(rejected[i].until-now)>0)return true;
 return false;
 }
static void close_peer(Peer *p){
 if(!p||p->state==FREE||p->state==CLOSING)return;
 p->authoritative=false;
 if(p->handle!=HCI_CON_HANDLE_INVALID){p->state=CLOSING;
 p->started=now;
 (void)gap_disconnect(p->handle);
 return;
 }
 if(connecting==p){p->state=CLOSING;
 p->started=now;
 (void)gap_connect_cancel();
 return;
 }
 p->state=FREE;
}
static bool emit(void *ctx,const MbrMouseEvent *event){
 Peer *p=ctx;
 if(!p->authoritative)return true;
 if(event->type==MBR_MOUSE_BUTTON&&event->data.button.button==4&&mbr_logitech_hidpp_claims_forward(&p->hidpp))return true;
 MbrBtMessage m={.kind=MBR_BT_INPUT,.session=p->generation,.input=*event};
 return publish(&m);
}
static void emit_forward(Peer *p,bool down){if(!p->authoritative)return;
 MbrBtMessage m={.kind=MBR_BT_INPUT,.session=p->generation,.input={.session=p->generation,.type=MBR_MOUSE_BUTTON,.data.button={4,down}}};
 if(!publish(&m))close_peer(p);
 }
static MouseId identity(uint8_t type,const uint8_t *address){uint32_t id=(2166136261u^type)*16777619u;
 for(unsigned i=0;i<6;++i)id=(id^address[i])*16777619u;
 return id?id:1;
 }
static void resolve_identity(Peer *p){
 int index=sm_le_device_index(p->handle),type=p->mouse.address_type;
 bd_addr_t address;
 sm_key_t irk;
 memcpy(address,p->mouse.address,6);
 if(index>=0)le_device_db_info(index,&type,address,irk);
 p->mouse.bonded=index>=0;
 p->mouse.address_type=(uint8_t)type&1u;
 memcpy(p->mouse.address,address,6);
 p->mouse.id=identity(p->mouse.address_type,address);
 int i=saved_id(p->mouse.id);
 if(i>=0){p->mouse.profile=known[i].profile;
 if(!p->mouse.name[0])memcpy(p->mouse.name,known[i].name,MBR_NAME_CAPACITY);
 }
}
void mbr_hogp_registry(const MbrMouse *mice,size_t count){if(count>MBR_SAVED_CAPACITY)count=MBR_SAVED_CAPACITY;
 memcpy(known,mice,count*sizeof(*mice));
 known_count=count;
 }
bool mbr_hogp_forget(const MbrMouse *m){
 for(int i=0;i<le_device_db_max_count();++i){int type;
 bd_addr_t address;
 sm_key_t irk;
 le_device_db_info(i,&type,address,irk);
 if(type!=(int)BD_ADDR_TYPE_UNKNOWN&&(uint8_t)type==m->address_type&&!memcmp(address,m->address,6))le_device_db_remove(i);
 }
 (void)gap_load_resolving_list_from_le_device_db();
 return true;
}
void mbr_hogp_profile(MouseSessionId session,bool remapped){
 Peer *p=by_session(session);
 if(!p)return;
 p->parser.aggregate_buttons=0;
 for(size_t i=0;i<p->parser.report_count;++i)p->parser.reports[i].button_mask=0;
 mbr_logitech_hidpp_set_forward_desired(&p->hidpp,remapped&&p->hidpp_capable);
}
void mbr_hogp_accept(MouseSessionId session){
 Peer *p=by_session(session);
 if(!p||p->state!=READY)return;
 for(unsigned i=0;i<2;++i)if(peers[i].authoritative&& &peers[i]!=p)return;
 p->authoritative=true;
 mbr_hogp_profile(session,p->mouse.profile!=MBR_PASSTHROUGH);
}
void mbr_hogp_disconnect(MouseSessionId session){for(unsigned i=0;i<2;++i)if(!session||peers[i].generation==session)close_peer(&peers[i]);
 }
static void start_search(void){
 if(!working||scanning||connecting||wanted==MBR_SEARCH_NONE||(int32_t)(retry_at-now)>0)return;
 unsigned free_count=0;
 for(unsigned i=0;i<2;++i){if(peers[i].state==FREE)++free_count;
 else if(!peers[i].authoritative)return;
 }
 if(!free_count)return;
 gap_set_scan_parameters(1,48,48);
 gap_start_scan();
 scanning=true;
}
void mbr_hogp_search(MbrSearch purpose,uint32_t tx){
 if(wanted==purpose&&transaction==tx){start_search();
 return;
 }
 bool first_cycle=wanted==MBR_SEARCH_FIRST&&purpose==MBR_SEARCH_FIRST;
 wanted=purpose;
 transaction=tx;
 if(scanning){gap_stop_scan();
 scanning=false;
 }
 for(unsigned i=0;i<2;++i)if(peers[i].state!=FREE&&!peers[i].authoritative){
  if(first_cycle&&peers[i].state!=CLOSING){peers[i].transaction=tx;
 if(peers[i].state==READY)(void)status(&peers[i],MBR_BT_READY);
 }
  else close_peer(&peers[i]);
 }
 start_search();
}
static void start_hids(Peer *p){p->state=DISCOVERING;
 p->started=now;
 if(hids_client_connect(p->handle,gatt_event,HID_PROTOCOL_MODE_REPORT,&p->cid)!=ERROR_CODE_SUCCESS){cool_down(p,1000);
 close_peer(p);
 }}
static void name_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size){
 (void)channel;
 (void)size;
 if(type!=HCI_EVENT_PACKET)return;
 if(hci_event_packet_get_type(packet)==GATT_EVENT_CHARACTERISTIC_VALUE_QUERY_RESULT){
  Peer *p=by_handle(gatt_event_characteristic_value_query_result_get_handle(packet));
 if(!p||p->state!=READING_NAME)return;
  unsigned n=gatt_event_characteristic_value_query_result_get_value_length(packet);
 if(n>=MBR_NAME_CAPACITY)n=MBR_NAME_CAPACITY-1;
  memcpy(p->mouse.name,gatt_event_characteristic_value_query_result_get_value(packet),n);
 p->mouse.name[n]=0;
 }else if(hci_event_packet_get_type(packet)==GATT_EVENT_QUERY_COMPLETE){Peer *p=by_handle(gatt_event_query_complete_get_handle(packet));
 if(p&&p->state==READING_NAME)start_hids(p);
 }
}
static void secured(Peer *p){
 resolve_identity(p);
 bool saved=saved_id(p->mouse.id)>=0;
 if((wanted==MBR_SEARCH_NEW&&saved)||(wanted==MBR_SEARCH_SAVED&&!saved)){cool_down(p,16000);
 close_peer(p);
 return;
 }
 p->state=READING_NAME;
 p->started=now;
 /* Advertising name avoids an unnecessary serialized GATT query before HIDS. */
 if(p->name_complete||gatt_client_read_value_of_characteristics_by_uuid16(name_event,p->handle,1,0xffff,ORG_BLUETOOTH_CHARACTERISTIC_GAP_DEVICE_NAME)!=ERROR_CODE_SUCCESS)start_hids(p);
}
static void gatt_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size){
 (void)channel;
 (void)size;
 if(type!=HCI_EVENT_PACKET||hci_event_packet_get_type(packet)!=HCI_EVENT_GATTSERVICE_META)return;
 switch(hci_event_gattservice_meta_get_subevent_code(packet)){
 case GATTSERVICE_SUBEVENT_HID_SERVICE_CONNECTED:{
  Peer *p=by_cid(gattservice_subevent_hid_service_connected_get_hids_cid(packet));
 if(!p||p->state!=DISCOVERING)return;
  if(gattservice_subevent_hid_service_connected_get_status(packet)!=ERROR_CODE_SUCCESS){cool_down(p,1500);
 close_peer(p);
 return;
 }
  const uint8_t *map=hids_client_descriptor_storage_get_descriptor_data(p->cid,0);
 uint16_t length=hids_client_descriptor_storage_get_descriptor_len(p->cid,0);
  if(gattservice_subevent_hid_service_connected_get_num_instances(packet)!=1||!mbr_hogp_configure(&p->parser,p->generation,map,length)){cool_down(p,30000);
 close_peer(p);
 return;
 }
  p->hidpp_capable=false;
 for(size_t i=0;i<p->parser.report_count;++i)if(p->parser.reports[i].report_id==0x11)p->hidpp_capable=true;
  mbr_logitech_hidpp_on_connect(&p->hidpp);
 p->state=READY;
 if(!status(p,MBR_BT_READY))close_peer(p);
 break;
 }
 case GATTSERVICE_SUBEVENT_HID_SERVICE_DISCONNECTED:{Peer *p=by_cid(gattservice_subevent_hid_service_disconnected_get_hids_cid(packet));
 if(p)close_peer(p);
 break;
 }
 case GATTSERVICE_SUBEVENT_HID_REPORT:{
  Peer *p=by_cid(gattservice_subevent_hid_report_get_hids_cid(packet));
 if(!p||p->state!=READY)return;
  uint8_t id=gattservice_subevent_hid_report_get_report_id(packet);
 const uint8_t *raw=gattservice_subevent_hid_report_get_report(packet);
 size_t length=gattservice_subevent_hid_report_get_report_len(packet);
  const uint8_t *payload;
 size_t n;
 if(!mbr_hogp_normalize(&p->parser,id,raw,length,&payload,&n)){close_peer(p);
 return;
 }
  mbr_hidpp_input_result_t result;
  if(mbr_logitech_hidpp_process_input(&p->hidpp,id,payload,n,&result)){if(result.held_changed)emit_forward(p,result.forward_held);
 }
  else if(!mbr_hogp_parse(&p->parser,id,raw,length,emit,p))close_peer(p);
  break;
 }
 default:break;
 }
}
static Advert *advert_for(uint8_t type,const uint8_t *address){
 for(unsigned i=0;i<8;++i)if(adverts[i].used&&adverts[i].type==type&&!memcmp(adverts[i].address,address,6))return &adverts[i];
 Advert *a=&adverts[advert_next++%8];
 memset(a,0,sizeof(*a));
 a->used=true;
 a->type=type;
 memcpy(a->address,address,6);
 return a;
}
static void advertisement(uint8_t *packet){
 if(!scanning)return;
 bd_addr_t address;
 gap_event_advertising_report_get_address(packet,address);
 uint8_t type=gap_event_advertising_report_get_address_type(packet);
 if(rejected_address(type,address))return;
 for(unsigned i=0;i<2;++i)if(peers[i].state!=FREE&&peers[i].mouse.address_type==type&&!memcmp(peers[i].mouse.address,address,6))return;
 Advert *a=advert_for(type,address);
 uint8_t ev=gap_event_advertising_report_get_advertising_event_type(packet);
 if(ev==0||ev==1)a->connectable=true;
 uint8_t n=gap_event_advertising_report_get_data_length(packet);
 const uint8_t *data=gap_event_advertising_report_get_data(packet);
 unsigned appearance=0;
 if(ad_data_contains_uuid16(n,data,ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE))a->hid=true;
 ad_context_t ad;
 for(ad_iterator_init(&ad,n,(uint8_t *)data);ad_iterator_has_more(&ad);ad_iterator_next(&ad)){
  unsigned kind=ad_iterator_get_data_type(&ad),len=ad_iterator_get_data_len(&ad);
 const uint8_t *value=ad_iterator_get_data(&ad);
  if(kind==BLUETOOTH_DATA_TYPE_APPEARANCE&&len>=2)appearance=little_endian_read_16(value,0);
  if(kind==BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME||kind==BLUETOOTH_DATA_TYPE_SHORTENED_LOCAL_NAME){if(len>=MBR_NAME_CAPACITY)len=MBR_NAME_CAPACITY-1;
 memcpy(a->name,value,len);
 a->name[len]=0;
 a->name_complete=kind==BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME;
 }
 }
 if(appearance>=960&&appearance<=1023&&appearance!=960&&appearance!=962)return;
 if(appearance==962)a->hid=true;
 int saved=saved_address(type,address);
 if(wanted==MBR_SEARCH_NEW&&saved>=0)return;
 /* Controller-resolved identity addresses make private bonded advertisers eligible. */
 if(wanted==MBR_SEARCH_SAVED&&saved<0)return;
 if(!a->connectable||(!a->hid&&saved<0))return;
 Peer *p=NULL;
 for(unsigned i=0;i<2;++i)if(peers[i].state==FREE){p=&peers[i];
 break;
 }if(!p)return;
 memset(p,0,sizeof(*p));
 p->handle=HCI_CON_HANDLE_INVALID;
 p->state=CONNECTING;
 p->transaction=transaction;
 p->started=now;
 if(++serial==0)++serial;
 p->generation=serial;
 memcpy(p->mouse.address,address,6);
 p->mouse.address_type=type;
 memcpy(p->mouse.name,a->name,MBR_NAME_CAPACITY);
 p->name_complete=a->name_complete;
 if(saved>=0){p->mouse.id=known[saved].id;
 p->mouse.profile=known[saved].profile;
 }
 gap_stop_scan();
 scanning=false;
 connecting=p;
 if(gap_connect(address,type)!=ERROR_CODE_SUCCESS){connecting=NULL;
 p->state=FREE;
 retry_at=now+200;
 }
}
static void hci_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size){
 (void)channel;
 (void)size;
 if(type!=HCI_EVENT_PACKET)return;
 switch(hci_event_packet_get_type(packet)){
 case BTSTACK_EVENT_STATE:if(btstack_event_state_get_state(packet)==HCI_STATE_WORKING){working=true;
 (void)gap_load_resolving_list_from_le_device_db();
 start_search();
 }break;
 case GAP_EVENT_ADVERTISING_REPORT:advertisement(packet);
 break;
 case HCI_EVENT_META_GAP:
  if(hci_event_gap_meta_get_subevent_code(packet)==GAP_SUBEVENT_LE_CONNECTION_COMPLETE){
   Peer *p=connecting;
 connecting=NULL;
   if(gap_subevent_le_connection_complete_get_status(packet)!=ERROR_CODE_SUCCESS){if(p){cool_down(p,1000);
 p->state=FREE;
 }retry_at=now+100;
 break;
 }
   hci_con_handle_t h=gap_subevent_le_connection_complete_get_connection_handle(packet);
   if(!p){gap_disconnect(h);
 break;
 }
   bool cancel=p->state==CLOSING;
 p->handle=h;
 gap_subevent_le_connection_complete_get_peer_address(packet,p->mouse.address);
 p->mouse.address_type=gap_subevent_le_connection_complete_get_peer_address_type(packet);
   p->state=SECURING;
 p->started=now;
 if(cancel){close_peer(p);
 break;
 }sm_request_pairing(h);
  }break;
 case HCI_EVENT_DISCONNECTION_COMPLETE:{
  Peer *p=by_handle(hci_event_disconnection_complete_get_connection_handle(packet));
 if(!p)return;
  (void)status(p,MBR_BT_DISCONNECTED);
 if(connecting==p)connecting=NULL;
 memset(p,0,sizeof(*p));
 p->handle=HCI_CON_HANDLE_INVALID;
 retry_at=now+100;
 break;
 }
 default:break;
 }
}
static void sm_event(uint8_t type,uint16_t channel,uint8_t *packet,uint16_t size){
 (void)channel;
 (void)size;
 if(type!=HCI_EVENT_PACKET)return;
 Peer *p=NULL;
 switch(hci_event_packet_get_type(packet)){
 case SM_EVENT_JUST_WORKS_REQUEST:p=by_handle(sm_event_just_works_request_get_handle(packet));
 if(p&&p->state==SECURING)sm_just_works_confirm(p->handle);
 break;
 case SM_EVENT_PAIRING_COMPLETE:
  p=by_handle(sm_event_pairing_complete_get_handle(packet));
 if(!p||p->state!=SECURING)return;
  if(sm_event_pairing_complete_get_status(packet)==ERROR_CODE_SUCCESS)secured(p);
 else{cool_down(p,2000);
 close_peer(p);
 }break;
 case SM_EVENT_REENCRYPTION_COMPLETE:
  p=by_handle(sm_event_reencryption_complete_get_handle(packet));
 if(!p||p->state!=SECURING)return;
  if(sm_event_reencryption_complete_get_status(packet)==ERROR_CODE_SUCCESS)secured(p);
  else if(!p->repaired){int index=sm_le_device_index(p->handle);
 if(index>=0)le_device_db_remove(index);
 p->repaired=true;
 sm_request_pairing(p->handle);
 }
  else{cool_down(p,2000);
 close_peer(p);
 }break;
 default:break;
 }
}
void mbr_hogp_tick(uint32_t time){
 now=time;
 for(unsigned i=0;i<2;++i){Peer *p=&peers[i];
  if(p->state!=FREE&&p->state!=READY&&p->state!=CLOSING&&(uint32_t)(now-p->started)>12000){cool_down(p,2000);
 close_peer(p);
 }
  if(p->state==CLOSING&&(uint32_t)(now-p->started)>2000&&p->handle!=HCI_CON_HANDLE_INVALID){p->started=now;
 (void)gap_disconnect(p->handle);
 }
  if(p->state!=READY||!p->authoritative||!p->hidpp_capable)continue;
  if(p->hidpp.waiting_for!=MBR_HIDPP_OUTPUT_NONE&&(uint32_t)(now-p->hidpp_sent)>1000){
   /* A failed optional negotiation cannot stop generic Mouse input. */
   if(p->hidpp.forward_held)emit_forward(p,false);
   bool was_applied=p->hidpp.forward_applied;
   p->hidpp.waiting_for=MBR_HIDPP_OUTPUT_NONE;
 p->hidpp.feature_failed=true;
 p->hidpp.forward_desired=false;
   if(was_applied){close_peer(p);
 continue;
 }
  }
  if(p->hidpp.feature_failed&&p->hidpp.forward_applied&&!p->hidpp.forward_desired){close_peer(p);
 continue;
 }
  mbr_hidpp_output_t out;
 if(mbr_logitech_hidpp_next_output(&p->hidpp,&out)){
   uint8_t status_code=hids_client_send_write_report(p->cid,out.report_id,HID_REPORT_TYPE_OUTPUT,out.payload,(uint8_t)out.payload_len);
   if(status_code==ERROR_CODE_SUCCESS){mbr_logitech_hidpp_output_result(&p->hidpp,out.kind,true);
 p->hidpp_sent=now;
 }
   else if(status_code!=GATT_CLIENT_IN_WRONG_STATE){p->hidpp.feature_failed=true;
 p->hidpp.forward_desired=false;
 }
  }
 }
 start_search();
}
void mbr_hogp_setup(MbrHogpPublish callback){
 publish=callback;
 memset(peers,0,sizeof(peers));
 for(unsigned i=0;i<2;++i)peers[i].handle=HCI_CON_HANDLE_INVALID;
 hids_client_init(descriptors,sizeof(descriptors));
 hci_registration.callback=hci_event;
 hci_add_event_handler(&hci_registration);
 sm_registration.callback=sm_event;
 sm_add_event_handler(&sm_registration);
}
