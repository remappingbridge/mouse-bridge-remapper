#include "mbr/application/bridge.h"
#include "mbr/usb_hid/usb_hid.h"
#include "mbr/remap/remap.h"
#include <string.h>
static void snapshot(const MbrApp *a,MbrProduct *p){p->registry=a->registry;
 memcpy(p->custom,a->custom,sizeof(p->custom));
 memcpy(p->draft,a->draft,sizeof(p->draft));
 p->dirty=a->custom_dirty;
 }
static void restore(MbrApp *a,const MbrProduct *p){a->registry=p->registry;
 memcpy(a->custom,p->custom,sizeof(a->custom));
 memcpy(a->draft,p->draft,sizeof(a->draft));
 a->custom_dirty=p->dirty;
 }
void mbr_bridge_init(MbrBridge *b,MbrApp *a){memset(b,0,sizeof(*b));
 (void)mbr_product_load(&b->product);
 restore(a,&b->product);
 mbr_app_home(a);
 b->initialized=true;
 b->usb_epoch=mbr_usb_epoch();
 }
static void release_output(MbrBridge *b){mbr_output_release(&b->output);
 mbr_usb_release_all();
 }
void mbr_bridge_release(MbrBridge *b,MbrApp *a){MouseSessionId transport=b->transport,session=b->output.session;
 b->transport=0;
 release_output(b);
 if(transport){mbr_app_disconnected(a,session);
 mbr_bt_disconnect(transport);
 }}
static void sync_registry(MbrBridge *b){mbr_bt_registry(b->product.registry.mice,b->product.registry.count);
 b->synced=true;
 }
static MbrProfile profile(const MbrApp *a){int i=mbr_registry_find(&a->registry,a->sessions.live.mouse);
 return i>=0?a->registry.mice[i].profile:MBR_PASSTHROUGH;
 }
static void profile_barrier(MbrBridge *b,MbrApp *a){
 release_output(b);
 if(b->transport){mbr_output_begin(&b->output,a->sessions.live.generation);
 mbr_bt_profile(b->transport,profile(a)!=MBR_PASSTHROUGH);
 }
}
static void fail_candidate(MbrBridge *b,MbrApp *a){
 if(b->pending)mbr_bt_disconnect(b->pending);
 b->pending=0;
 b->closing=0;
 if(a->request.kind==MBR_OP_HANDOFF)(void)mbr_app_confirm(a,a->request.token,false);
 if(!a->sessions.live.ready)mbr_app_home(a);
}
static void promote(MbrBridge *b,MbrApp *a){
 if(!b->pending||b->closing)return;
 if(a->request.kind!=MBR_OP_HANDOFF||a->request.token!=b->pending_token){fail_candidate(b,a);
 return;
 }
 static MbrProduct next;
 next=b->product;
 snapshot(a,&next);
 if(!mbr_registry_put(&next.registry,&a->candidate)||!mbr_product_save(&next)){fail_candidate(b,a);
 return;
 }
 b->product=next;
 if(!mbr_app_confirm(a,b->pending_token,true)){fail_candidate(b,a);
 return;
 }
 b->transport=b->pending;
 b->pending=0;
 b->pending_token=0;
 mbr_output_begin(&b->output,a->sessions.live.generation);
 sync_registry(b);
 mbr_bt_accept(b->transport);
 mbr_bt_profile(b->transport,profile(a)!=MBR_PASSTHROUGH);
}
static bool finish_removal(MbrBridge *b,MbrApp *a){
 if(!b->product.pending_remove.id)return true;
 if((int32_t)(a->now-b->retry_at)<0)return false;
 b->retry_at=a->now+1000;
 if(!mbr_bt_forget(&b->product.pending_remove))return false;
 static MbrProduct next;
 next=b->product;
 next.pending_remove=(MbrMouse){0};
 if(!mbr_product_save(&next))return false;
 b->product=next;
 if(a->request.kind==MBR_OP_REMOVE)(void)mbr_app_confirm(a,a->request.token,true);
 else {restore(a,&b->product);
 mbr_app_home(a);
 }
 b->removing=false;
 sync_registry(b);
 return true;
}
static void operations(MbrBridge *b,MbrApp *a){
 MbrRequest r=a->request;
 if(r.kind==MBR_OP_NONE||r.kind==MBR_OP_HANDOFF)return;
 static MbrProduct next;
 next=b->product;
 snapshot(a,&next);
 bool ok=false;
 if(r.kind==MBR_OP_DRAFT&&r.source<5&&r.target<=MBR_TARGET_BACKWARD){next.draft[r.source]=r.target;
 next.dirty=true;
 ok=mbr_product_save(&next);
 }
 else if(r.kind==MBR_OP_PROFILE){
  int i=mbr_registry_find(&next.registry,r.mouse);
  if(i>=0&&a->sessions.live.ready&&a->sessions.live.generation==r.session&&r.profile<=MBR_CUSTOM){
   next.registry.mice[i].profile=r.profile;
 if(r.profile==MBR_CUSTOM){memcpy(next.custom,next.draft,sizeof(next.custom));
 next.dirty=false;
 }
   ok=mbr_product_save(&next);
  }
 }else if(r.kind==MBR_OP_REMOVE){
  int i=mbr_registry_find(&next.registry,r.mouse);
  if(i>=0){
   next.pending_remove=next.registry.mice[i];
 (void)mbr_registry_remove(&next.registry,r.mouse);
   /* Durable tombstone permits idempotent credential cleanup after any power cut. */
   if(mbr_product_save(&next)){
    b->product=next;
 b->removing=true;
 b->retry_at=a->now;
    if(a->sessions.live.mouse==r.mouse){MouseSessionId old=b->transport;
 b->transport=0;
 release_output(b);
 a->sessions.live=(MbrSession){0};
 if(old){b->closing=old;
 mbr_bt_disconnect(old);
 }}
    sync_registry(b);
 if(!b->closing)(void)finish_removal(b,a);
 return;
   }
  }
 }
 if(ok)b->product=next;
 (void)mbr_app_confirm(a,r.token,ok);
 if(ok){sync_registry(b);
 if(r.kind==MBR_OP_PROFILE)profile_barrier(b,a);
 }
}
void mbr_bridge_task(MbrBridge *b,MbrApp *a){
 if(!b->initialized)mbr_bridge_init(b,a);
 if(!b->synced)sync_registry(b);
 mbr_bt_service(a->now);
 uint32_t epoch=mbr_usb_epoch();
 if(epoch!=b->usb_epoch){b->usb_epoch=epoch;
 profile_barrier(b,a);
 }
 /* Freeze search timeout only after handoff starts, never during qualification. */
 if(b->pending)a->search.started=a->now;
 if(!b->product.pending_remove.id)mbr_bt_search(a->search.purpose,a->search.generation);
 else mbr_bt_search(MBR_SEARCH_NONE,a->search.generation);
 for(unsigned i=0;i<MBR_BT_QUEUE_CAPACITY;++i){
  MbrBtMessage m;
 bool overflow=false;
 bool got=mbr_bt_next(&m,&overflow);
  if(overflow){mbr_bridge_release(b,a);
 mbr_bt_disconnect(0);
 b->pending=b->closing=0;
 if(a->request.kind==MBR_OP_HANDOFF)(void)mbr_app_confirm(a,a->request.token,false);
 break;
 }
  if(!got)break;
  if(m.kind==MBR_BT_READY){
   if(m.session==b->pending||m.session==b->transport)continue;
   if(b->pending||b->product.pending_remove.id||!mbr_app_ready(a,m.transaction,&m.mouse)){mbr_bt_disconnect(m.session);
 continue;
 }
   b->pending=m.session;
 b->pending_token=a->request.token;
 b->incoming=m.mouse;
   if(b->transport){b->closing=b->transport;
 b->transport=0;
 release_output(b);
 a->sessions.live=(MbrSession){0};
 mbr_bt_disconnect(b->closing);
 }
  }else if(m.kind==MBR_BT_DISCONNECTED){
   if(m.session==b->closing)b->closing=0;
   if(m.session==b->pending){b->pending=0;
 if(a->request.kind==MBR_OP_HANDOFF)(void)mbr_app_confirm(a,a->request.token,false);
 if(!a->sessions.live.ready)mbr_app_home(a);
 }
   else if(m.session==b->transport){MouseSessionId session=b->output.session;
 b->transport=0;
 release_output(b);
 mbr_app_disconnected(a,session);
 }
  }else if(m.kind==MBR_BT_INPUT&&b->transport&&m.session==b->transport&&m.input.session==m.session){
   m.input.session=b->output.session;
   if(!mbr_remap_event(&b->output,&m.input,profile(a),a->custom)){mbr_bridge_release(b,a);
 continue;
 }
   if(b->output.dirty){if(!mbr_usb_submit(&b->output.pending)){mbr_bridge_release(b,a);
 continue;
 }mbr_output_consumed(&b->output);
 }
  }
 }
 if(b->product.pending_remove.id){if(!b->closing)(void)finish_removal(b,a);
 return;
 }
 promote(b,a);
 operations(b,a);
}
