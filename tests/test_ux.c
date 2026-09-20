#include "mbr/application/application.h"
#include <assert.h>
#include <string.h>
#define S(x) MBR_SCREEN_##x
static void tap(MbrApp *a,MbrControl c) { mbr_app_event(a,c,true); mbr_app_event(a,c,false); }
static void first(MbrApp *a) { MbrMouse m={.id=1,.name="LOGITECH LIFT"}; assert(mbr_app_ready(a,a->search.generation,&m)); assert(!a->sessions.live.ready); assert(mbr_app_confirm(a,a->request.token,true)); }
static void test_transitions(void) {
 MbrApp a; mbr_app_init(&a,0);
 for(unsigned c=0;c<MBR_CONTROL_COUNT;++c) tap(&a,(MbrControl)c);
 assert(a.screen==S(SEARCHING_FIRST)&&!a.locked);
 uint32_t old=a.search.generation; mbr_app_tick(&a,8000); assert(a.search.generation!=old);
 first(&a); assert(a.screen==S(FIRST_MOUSE_CONNECTED));
 tap(&a,MBR_B); assert(a.locked); tap(&a,MBR_A); assert(a.locked); tap(&a,MBR_X); assert(!a.locked); tap(&a,MBR_Y); assert(a.screen==S(HOME_CONNECTED));
 mbr_app_event(&a,MBR_PRESS,true); assert(a.screen==S(HOME_CONNECTED)); mbr_app_event(&a,MBR_PRESS,false); assert(a.screen==S(PAIR_NEW)&&a.sessions.live.ready);
 MbrMouse saved=a.registry.mice[0]; assert(!mbr_app_ready(&a,a.search.generation,&saved));
 MouseSessionId session=a.sessions.live.generation;
 tap(&a,MBR_X); assert(a.screen==S(HELP_PAIR_NEW)); tap(&a,MBR_B); assert(a.screen==S(PAIR_NEW));
 tap(&a,MBR_Y); assert(a.locked); tap(&a,MBR_B); assert(!a.locked&&a.screen==S(PAIR_NEW));
 mbr_app_tick(&a,23000); assert(a.screen==S(RETRY_PAIR_NEW)&&a.sessions.live.generation==session);
 tap(&a,MBR_B); assert(a.screen==S(HOME_CONNECTED));
 mbr_app_disconnected(&a,session+99); assert(a.sessions.live.ready);
 mbr_app_disconnected(&a,session); assert(a.screen==S(HOME_SEARCHING));
 tap(&a,MBR_X); mbr_app_tick(&a,31000); assert(a.screen==S(HOME_SEARCHING_HELP)); tap(&a,MBR_A); assert(a.screen==S(HOME_RETRY)&&a.search.purpose==MBR_SEARCH_NONE);
 tap(&a,MBR_A); assert(a.screen==S(HOME_SEARCHING));
 assert(mbr_app_ready(&a,a.search.generation,&saved)); assert(mbr_app_confirm(&a,a.request.token,true));
 assert(a.screen==S(HOME_CONNECTED));
 // A release started on the old screen cannot activate the new one.
 mbr_app_event(&a,MBR_PRESS,true); mbr_app_disconnected(&a,a.sessions.live.generation); mbr_app_event(&a,MBR_PRESS,false); assert(a.screen==S(HOME_SEARCHING));
}
static void test_confirm(void) {
 MbrApp a; mbr_app_init(&a,0); first(&a); tap(&a,MBR_Y); tap(&a,MBR_DOWN); tap(&a,MBR_PRESS); assert(a.screen==S(REMAPPER_OPTIONS));
 tap(&a,MBR_DOWN); tap(&a,MBR_PRESS); assert(a.screen==S(STANDARD_NOT_ACTIVE));
 tap(&a,MBR_A); assert(a.request.kind==MBR_OP_PROFILE&&a.screen==S(STANDARD_NOT_ACTIVE));
 assert(!mbr_app_confirm(&a,a.request.token+1,true)); assert(mbr_app_confirm(&a,a.request.token,false)); assert(a.registry.mice[0].profile==MBR_PASSTHROUGH);
 tap(&a,MBR_A); assert(mbr_app_confirm(&a,a.request.token,true)); assert(a.screen==S(STANDARD_ACTIVE));
 tap(&a,MBR_B); tap(&a,MBR_DOWN); tap(&a,MBR_DOWN); tap(&a,MBR_PRESS); assert(a.screen==S(ESCAPE_NOT_ACTIVE)); tap(&a,MBR_Y); assert(!a.locked);
 tap(&a,MBR_A); assert(mbr_app_confirm(&a,a.request.token,true)); tap(&a,MBR_LEFT); assert(a.screen==S(HOME_CONNECTED));
 tap(&a,MBR_DOWN);tap(&a,MBR_PRESS); for(int i=0;i<3;++i)tap(&a,MBR_DOWN);tap(&a,MBR_PRESS); assert(a.screen==S(CUSTOM_EDIT));
 tap(&a,MBR_PRESS); tap(&a,MBR_DOWN); tap(&a,MBR_A); assert(a.draft[0]==MBR_TARGET_LEFT); assert(mbr_app_confirm(&a,a.request.token,true)); assert(a.screen==S(CUSTOM_EDIT)&&a.draft[0]==MBR_TARGET_RIGHT&&a.custom_dirty);
 // Handoff stays non-authoritative until the adapter confirms ordered side effects.
 mbr_app_home(&a); tap(&a,MBR_PRESS); MbrMouse next={.id=2,.name="SECOND MOUSE"};
 assert(mbr_app_ready(&a,a.search.generation,&next)); assert(a.sessions.live.mouse==1&&!a.sessions.candidate.ready);
 assert(mbr_app_confirm(&a,a.request.token,true)); assert(a.sessions.live.mouse==2&&a.registry.count==2);
 mbr_app_disconnected(&a,1); assert(a.sessions.live.mouse==2);
 tap(&a,MBR_DOWN);tap(&a,MBR_DOWN);tap(&a,MBR_PRESS);assert(a.screen==S(SAVED_DEVICES)); tap(&a,MBR_PRESS);tap(&a,MBR_A);assert(a.registry.count==2);assert(mbr_app_confirm(&a,a.request.token,true)); assert(a.registry.count==1&&a.sessions.live.mouse==2);
}
static void test_projector(void) {
 MbrView v={.screen=S(REMAPPER_OPTIONS),.profile=MBR_STANDARD}; MbrFrame f; mbr_project(&v,&f); assert(f.cells[2][1].tone==MBR_CYAN); v.selection=1;mbr_project(&v,&f);assert(f.cells[2][1].tone==MBR_WHITE); assert(f.cells[8][0].tone==MBR_ACTION);
 for(int s=0;s<MBR_SCREEN_COUNT;++s) { v.screen=(MbrScreen)s; v.selection=0; mbr_project(&v,&f); assert(f.cells[0][0].tone==MBR_TITLE); }
 v.screen=S(SEARCHING_FIRST);v.pressed=1u<<MBR_RIGHT;mbr_project(&v,&f);assert(f.cells[4][16].character=='J'&&f.cells[4][16].tone==MBR_WHITE);assert(f.cells[4][9].tone==MBR_ACTION);
 v.screen=S(HOME_SEARCHING_HELP);v.pressed=0;mbr_project(&v,&f);assert(f.hint==8&&f.cells[8][0].tone==MBR_ACTION);
 v.screen=S(SAVED_DEVICES);v.connected=false;mbr_project(&v,&f);assert(f.cells[1][0].tone==MBR_BODY);
}
int main(void) { test_transitions();test_confirm();test_projector();return 0; }
