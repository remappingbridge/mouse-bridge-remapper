#include "mbr/application/application.h"
#include <string.h>
#define S(x) MBR_SCREEN_##x
static bool home(MbrScreen s) { return s==S(SEARCHING_FIRST)||s==S(HOME_CONNECTED)||s==S(HOME_SEARCHING)||s==S(HOME_RETRY); }
static bool help(MbrScreen s) { return s==S(HOME_SEARCHING_HELP)||s==S(HOME_RETRY_HELP)||s==S(HELP_PAIR_NEW)||s==S(HELP_RETRY_PAIR_NEW)||s==S(HELP_HOME_CONNECTED)||s==S(HELP_REMAPPER_OPTIONS)||s==S(HELP_REMOVE_THIS); }
static void show(MbrApp *a,MbrScreen s) { a->screen=s; a->selection=0; ++a->epoch; }
static void search(MbrApp *a,MbrSearch p) { mbr_search_start(&a->search,p,a->now); }
void mbr_app_home(MbrApp *a) {
 if(!a->registry.count) { show(a,S(SEARCHING_FIRST)); if(a->search.purpose!=MBR_SEARCH_FIRST) search(a,MBR_SEARCH_FIRST); }
 else if(a->sessions.live.ready) { mbr_search_cancel(&a->search); show(a,S(HOME_CONNECTED)); }
 else { show(a,S(HOME_SEARCHING)); search(a,MBR_SEARCH_SAVED); }
}
void mbr_app_init(MbrApp *a,uint32_t now) {
 memset(a,0,sizeof(*a)); a->now=now;
 const MbrTarget defaults[]={MBR_TARGET_LEFT,MBR_TARGET_RIGHT,MBR_TARGET_MIDDLE,MBR_TARGET_FORWARD,MBR_TARGET_BACKWARD};
 memcpy(a->draft,defaults,sizeof(defaults)); mbr_app_home(a);
}
static MbrProfile live_profile(const MbrApp *a) { int i=mbr_registry_find(&a->registry,a->sessions.live.mouse); return i<0?MBR_PASSTHROUGH:a->registry.mice[i].profile; }
static void pair(MbrApp *a) { show(a,S(PAIR_NEW)); search(a,MBR_SEARCH_NEW); }
static void request(MbrApp *a,MbrOperation op,MouseId id,MbrProfile p) {
 if(a->request.kind!=MBR_OP_NONE) return;
 if(++a->request_counter==0) ++a->request_counter;
 a->request=(MbrRequest){.kind=op,.token=a->request_counter,.mouse=id,.session=a->sessions.live.generation,.profile=p,.source=a->source,.target=(MbrTarget)a->selection};
}
static void contextual_help(MbrApp *a) {
 MbrScreen dest;
 switch(a->screen) {
 case S(HOME_SEARCHING): dest=S(HOME_SEARCHING_HELP); break;
 case S(HOME_RETRY): dest=S(HOME_RETRY_HELP); break;
 case S(PAIR_NEW): dest=S(HELP_PAIR_NEW); break;
 case S(RETRY_PAIR_NEW): dest=S(HELP_RETRY_PAIR_NEW); break;
 case S(HOME_CONNECTED): dest=S(HELP_HOME_CONNECTED); break;
 case S(REMAPPER_OPTIONS): dest=S(HELP_REMAPPER_OPTIONS); break;
 case S(REMOVE_THIS): dest=S(HELP_REMOVE_THIS); break;
 default:return;
 }
 a->help_owner=a->screen; show(a,dest);
}
static unsigned options(MbrScreen s) {
 if(s==S(HOME_CONNECTED)||s==S(REMAPPER_OPTIONS)) return 4;
 if(s==S(HOME_SEARCHING)||s==S(HOME_RETRY)) return 3;
 if(s==S(CUSTOM_EDIT)) return 5;
 if(s>=S(LEFT)&&s<=S(BACKWARD)) return 6;
 return 0;
}
static void profile_page(MbrApp *a,MbrProfile p) {
 bool active=live_profile(a)==p;
 const MbrScreen off[]={S(PASSTHROUGH_NOT_ACTIVE),S(STANDARD_NOT_ACTIVE),S(ESCAPE_NOT_ACTIVE),S(CUSTOM_EDIT)};
 const MbrScreen on[]={S(PASSTHROUGH_ACTIVE),S(STANDARD_ACTIVE),S(ESCAPE_ACTIVE),S(CUSTOM_EDIT)};
 show(a,active?on[p]:off[p]);
}
void mbr_app_event(MbrApp *a,MbrControl c,bool down) {
 if(!mbr_interaction_event(&a->interaction,c,down,a->epoch)) return;
 if(a->locked) {
  if(!mbr_screen_didactic(a->screen)||c==MBR_X) { a->locked=false; ++a->epoch; }
  return;
 }
 if(help(a->screen)) { show(a,a->help_owner); return; }
 if(a->screen==S(SEARCHING_FIRST)) return;
 if(a->screen==S(FIRST_MOUSE_CONNECTED)||a->screen==S(LEARN_THE_KEYS)) {
  if(c==MBR_B) { a->locked=true; ++a->epoch; }
  if(c==MBR_Y) mbr_app_home(a);
  return;
 }
 if(c==MBR_X) { contextual_help(a); return; }
 if(c==MBR_Y) {
  for(unsigned r=1;r<9;++r) if(strcmp(mbr_screens[a->screen].rows[r],"KEY Y: LOCK")==0) { a->locked=true; ++a->epoch; }
  return;
 }
 unsigned n=options(a->screen);
 if(n && (c==MBR_UP||c==MBR_DOWN)) { a->selection=(uint8_t)((a->selection+n+(c==MBR_UP?-1:1))%n); return; }
 switch(a->screen) {
 case S(HOME_SEARCHING): case S(HOME_RETRY): case S(HOME_CONNECTED):
  if(c==MBR_B && a->screen==S(HOME_SEARCHING)) { mbr_search_cancel(&a->search); show(a,S(HOME_RETRY)); }
  else if(c==MBR_A && a->screen==S(HOME_RETRY)) mbr_app_home(a);
  else if(c==MBR_PRESS) {
   unsigned sel=a->selection;
   if(sel==0) pair(a);
   else if(a->screen==S(HOME_CONNECTED)&&sel==1) show(a,S(REMAPPER_OPTIONS));
   else if(sel==(a->screen==S(HOME_CONNECTED)?2u:1u)) { a->page=0; show(a,S(SAVED_DEVICES)); }
   else show(a,S(LEARN_THE_KEYS));
  } break;
 case S(PAIR_NEW): case S(RETRY_PAIR_NEW):
  if(c==MBR_B) { mbr_search_cancel(&a->search); a->request=(MbrRequest){0}; a->sessions.candidate=(MbrSession){0}; mbr_app_home(a); }
  else if(c==MBR_A && a->screen==S(RETRY_PAIR_NEW)) pair(a);
  break;
 case S(REMAPPER_OPTIONS):
  if(c==MBR_LEFT) mbr_app_home(a);
  else if(c==MBR_PRESS) profile_page(a,(MbrProfile)a->selection);
  break;
 case S(PASSTHROUGH_ACTIVE):case S(PASSTHROUGH_NOT_ACTIVE):case S(STANDARD_ACTIVE):case S(STANDARD_NOT_ACTIVE):case S(ESCAPE_ACTIVE):case S(ESCAPE_NOT_ACTIVE):
  if(c==MBR_B) show(a,S(REMAPPER_OPTIONS));
  else if(c==MBR_LEFT && a->screen==S(ESCAPE_ACTIVE)) mbr_app_home(a);
  else if(c==MBR_A && (a->screen==S(PASSTHROUGH_NOT_ACTIVE)||a->screen==S(STANDARD_NOT_ACTIVE)||a->screen==S(ESCAPE_NOT_ACTIVE)))
   request(a,MBR_OP_PROFILE,a->sessions.live.mouse,a->screen==S(PASSTHROUGH_NOT_ACTIVE)?MBR_PASSTHROUGH:a->screen==S(STANDARD_NOT_ACTIVE)?MBR_STANDARD:MBR_ESCAPE);
  break;
 case S(CUSTOM_EDIT):
  if(c==MBR_A) request(a,MBR_OP_PROFILE,a->sessions.live.mouse,MBR_CUSTOM);
  else if(c==MBR_PRESS) { a->source=a->selection; show(a,(MbrScreen)(S(LEFT)+a->source)); a->selection=(uint8_t)a->draft[a->source]; }
  break;
 case S(LEFT):case S(RIGHT):case S(MIDDLE):case S(FORWARD):case S(BACKWARD):
  if(c==MBR_A) request(a,MBR_OP_DRAFT,0,MBR_CUSTOM);
  break;
 case S(SAVED_DEVICES):
  if(c==MBR_B) mbr_app_home(a);
  else if(a->registry.count && (c==MBR_LEFT||c==MBR_RIGHT)) a->page=(uint8_t)((a->page+a->registry.count+(c==MBR_LEFT?-1:1))%a->registry.count);
  else if(c==MBR_PRESS && a->registry.count) show(a,S(REMOVE_THIS));
  break;
 case S(REMOVE_THIS):
  if(c==MBR_B) show(a,S(SAVED_DEVICES));
  else if(c==MBR_A && a->page<a->registry.count) request(a,MBR_OP_REMOVE,a->registry.mice[a->page].id,MBR_PASSTHROUGH);
  break;
 default: break;
 }
}
void mbr_app_tick(MbrApp *a,uint32_t now) {
 a->now=now; if(!mbr_search_expired(&a->search,now)) return;
 MbrSearch purpose=a->search.purpose; mbr_search_cancel(&a->search);
 if(a->request.kind==MBR_OP_HANDOFF) { a->request=(MbrRequest){0}; a->sessions.candidate=(MbrSession){0}; }
 if(purpose==MBR_SEARCH_FIRST) { if(!a->registry.count) search(a,MBR_SEARCH_FIRST); }
 else if(purpose==MBR_SEARCH_SAVED) { if(a->screen==S(HOME_SEARCHING)) show(a,S(HOME_RETRY)); else if(a->screen==S(HOME_SEARCHING_HELP)) a->help_owner=S(HOME_RETRY); }
 else if(purpose==MBR_SEARCH_NEW) { if(a->screen==S(PAIR_NEW)) show(a,S(RETRY_PAIR_NEW)); else if(a->screen==S(HELP_PAIR_NEW)) a->help_owner=S(RETRY_PAIR_NEW); }
}
bool mbr_app_ready(MbrApp *a,uint32_t tx,const MbrMouse *m) {
 if(!m||!m->id||a->request.kind!=MBR_OP_NONE || !mbr_search_eligible(&a->search,tx,mbr_registry_find(&a->registry,m->id)>=0)) return false;
 if(mbr_search_expired(&a->search,a->now)) return false;
 a->candidate=*m; a->sessions.candidate=(MbrSession){.mouse=m->id,.ready=false};
 request(a,MBR_OP_HANDOFF,m->id,m->profile); return true;
}
bool mbr_app_confirm(MbrApp *a,uint32_t token,bool success) {
 MbrRequest r=a->request; if(r.kind==MBR_OP_NONE||r.token!=token) return false;
 a->request=(MbrRequest){0};
 if(!success) { if(r.kind==MBR_OP_HANDOFF) a->sessions.candidate=(MbrSession){0}; return true; }
 int i=mbr_registry_find(&a->registry,r.mouse);
 if(r.kind==MBR_OP_HANDOFF) {
  /* Adapter confirms required effects before this call. MBR-05 supports only
   * initial/same-boot RAM registration; durable replacement is MBR-07. */
  bool first=a->registry.count==0;
  if(!mbr_registry_put(&a->registry,&a->candidate)) return false;
  a->sessions.live=(MbrSession){0};
  if(!mbr_session_promote(&a->sessions,r.mouse)) return false;
  mbr_search_cancel(&a->search);
  if(help(a->screen)) a->help_owner=first?S(FIRST_MOUSE_CONNECTED):S(HOME_CONNECTED);
  else show(a,first?S(FIRST_MOUSE_CONNECTED):S(HOME_CONNECTED));
 } else if(r.kind==MBR_OP_DRAFT) {
  a->draft[r.source]=r.target; a->custom_dirty=true; show(a,S(CUSTOM_EDIT)); a->selection=r.source;
 } else if(r.kind==MBR_OP_PROFILE) {
  if(i<0 || !a->sessions.live.ready || a->sessions.live.generation!=r.session) return false;
  a->registry.mice[i].profile=r.profile; if(r.profile==MBR_CUSTOM) a->custom_dirty=false;
  profile_page(a,r.profile);
 } else if(r.kind==MBR_OP_REMOVE) {
  if(a->sessions.live.mouse==r.mouse) a->sessions.live=(MbrSession){0};
  if(!mbr_registry_remove(&a->registry,r.mouse)) return false;
  if(!a->registry.count) mbr_app_home(a);
  else { a->page=(uint8_t)(a->page%a->registry.count); show(a,S(SAVED_DEVICES)); }
 }
 return true;
}
void mbr_app_disconnected(MbrApp *a,MouseSessionId generation) {
 if(!mbr_session_clear(&a->sessions,generation)) return;
 if(a->request.kind==MBR_OP_PROFILE) a->request=(MbrRequest){0};
 if(home(a->screen)) mbr_app_home(a);
 else if(a->screen==S(HELP_HOME_CONNECTED)) { search(a,MBR_SEARCH_SAVED); a->help_owner=S(HOME_SEARCHING); }
}
void mbr_app_view(const MbrApp *a,MbrView *v) {
 memset(v,0,sizeof(*v)); v->screen=a->screen; v->selection=a->selection; v->pressed=a->interaction.pressed;
 v->page=a->page; v->count=a->registry.count; memcpy(v->draft,a->draft,sizeof(v->draft));
 int i=mbr_registry_find(&a->registry,a->sessions.live.mouse);
 if(a->screen==S(SAVED_DEVICES)||a->screen==S(REMOVE_THIS)) i=a->page<a->registry.count?a->page:-1;
 if(i>=0) { const MbrMouse *m=&a->registry.mice[i]; memcpy(v->name,m->name,sizeof(v->name)); v->profile=m->profile; v->connected=a->sessions.live.ready&&a->sessions.live.mouse==m->id; }
 v->custom_applied=live_profile(a)==MBR_CUSTOM&&!a->custom_dirty;
}
