/* RAM-only qualification executable. This translation unit is not in production. */
#include "mbr/application/qualification.h"
#include <string.h>
#define S(x) MBR_SCREEN_##x
/* mode 0 menu, 1 gallery, 2 interactive fixture, 3 USB menu, 4 USB output */
void mbr_qualification_init(MbrQualification *q) { memset(q,0,sizeof(*q));q->epoch=1; }
static void mode(MbrQualification *q,unsigned m) { q->mode=m;q->selection=0;++q->epoch;memset(&q->interaction,0,sizeof(q->interaction));mbr_usb_release_all(); }
static void seed(MbrApp *a,MbrScreen screen) {
 uint32_t now=a->now;mbr_app_init(a,now);
 MbrMouse m={.id=1,.name="LOGITECH LIFT",.profile=MBR_ESCAPE};mbr_registry_put(&a->registry,&m);
 m.id=2;strcpy(m.name,"SECOND SAVED MOUSE WITH LONG NAME");m.profile=MBR_STANDARD;mbr_registry_put(&a->registry,&m);
 mbr_session_promote(&a->sessions,1);mbr_search_cancel(&a->search);a->screen=screen;++a->epoch;
 if(screen==S(SEARCHING_FIRST)) mbr_app_init(a,now);
 if(screen==S(HOME_SEARCHING)||screen==S(HOME_RETRY)||screen==S(HOME_SEARCHING_HELP)||screen==S(HOME_RETRY_HELP)) {
  a->sessions.live=(MbrSession){0};
  if(screen==S(HOME_SEARCHING)||screen==S(HOME_SEARCHING_HELP))mbr_search_start(&a->search,MBR_SEARCH_SAVED,now);
 }
 if(screen==S(PAIR_NEW)||screen==S(HELP_PAIR_NEW))mbr_search_start(&a->search,MBR_SEARCH_NEW,now);
 const MbrScreen help_s[]={S(HOME_SEARCHING_HELP),S(HOME_RETRY_HELP),S(HELP_PAIR_NEW),S(HELP_RETRY_PAIR_NEW),S(HELP_HOME_CONNECTED),S(HELP_REMAPPER_OPTIONS),S(HELP_REMOVE_THIS)};
 const MbrScreen owners[]={S(HOME_SEARCHING),S(HOME_RETRY),S(PAIR_NEW),S(RETRY_PAIR_NEW),S(HOME_CONNECTED),S(REMAPPER_OPTIONS),S(REMOVE_THIS)};
 for(unsigned i=0;i<7;++i)if(screen==help_s[i])a->help_owner=owners[i];
 if(screen==S(PASSTHROUGH_ACTIVE))a->registry.mice[0].profile=MBR_PASSTHROUGH;
 if(screen==S(STANDARD_ACTIVE))a->registry.mice[0].profile=MBR_STANDARD;
 if(screen==S(ESCAPE_NOT_ACTIVE))a->registry.mice[0].profile=MBR_PASSTHROUGH;
 if(screen>=S(LEFT)&&screen<=S(BACKWARD))a->source=(uint8_t)(screen-S(LEFT));
}
void mbr_qualification_event(MbrQualification *q,MbrApp *a,MbrControl c,bool down) {
 if(q->mode==2) { mbr_app_event(a,c,down); return; }
 bool release=mbr_interaction_event(&q->interaction,c,down,q->epoch);
 if(q->mode==4 && q->usb_mode==0) {
  uint16_t p=q->interaction.pressed;
  mbr_output_state_t state={.buttons=(uint8_t)(((p&(1u<<MBR_UP))?1:0)|((p&(1u<<MBR_DOWN))?2:0)|((p&(1u<<MBR_PRESS))?4:0)|((p&(1u<<MBR_LEFT))?8:0)|((p&(1u<<MBR_RIGHT))?16:0))};
  mbr_usb_submit(&state);
 } else if(q->mode==4 && q->usb_mode==3) { mbr_output_state_t s={.escape=(q->interaction.pressed&(1u<<MBR_A))!=0};mbr_usb_submit(&s); }
 if(!release) return;
 if(q->mode==0||q->mode==3) {
  unsigned n=q->mode==0?3:4;
  if(c==MBR_UP||c==MBR_DOWN) q->selection=(q->selection+n+(c==MBR_UP?-1:1))%n;
  else if(c==MBR_B && q->mode==3)mode(q,0);
  else if(c==MBR_PRESS) {
   unsigned s=q->selection;
   if(q->mode==3) { q->usb_mode=s;mode(q,4); }
   else if(s==0) { q->gallery=0;mode(q,1); }
   else if(s==1) { seed(a,S(FIRST_MOUSE_CONNECTED));mode(q,2); }
   else mode(q,3);
  }
 } else if(q->mode==1) {
  if(c==MBR_A||c==MBR_B)q->gallery=(q->gallery+MBR_SCREEN_COUNT+(c==MBR_B?-1:1))%MBR_SCREEN_COUNT;
  else if(c==MBR_Y)mode(q,0);
  else if(c==MBR_X) { seed(a,(MbrScreen)q->gallery);q->first_at=a->now;mode(q,2); }
 } else if(q->mode==4 && c==MBR_B)mode(q,3);
}
void mbr_qualification_tick(MbrQualification *q,MbrApp *a,uint32_t now) {
 if(q->mode==2) {
  if(a->screen==S(SEARCHING_FIRST)&&(uint32_t)(now-q->first_at)>=3000) {
   MbrMouse m={.id=1,.name="RAM FIXTURE MOUSE"};mbr_app_ready(a,a->search.generation,&m);
  }
  if(a->request.kind!=MBR_OP_NONE) {
   if(!q->confirm_at)q->confirm_at=now;
   if((uint32_t)(now-q->confirm_at)>=500) { mbr_app_confirm(a,a->request.token,true);q->confirm_at=0; }
  } else q->confirm_at=0;
 } else if(q->mode==4&&(q->usb_mode==1||q->usb_mode==2)&&(uint32_t)(now-q->next_motion)>=20) {
  q->next_motion=now;uint16_t p=q->interaction.pressed;
  int32_t x=((p&(1u<<MBR_RIGHT))?2:0)-((p&(1u<<MBR_LEFT))?2:0);
  int32_t y=((p&(1u<<MBR_DOWN))?2:0)-((p&(1u<<MBR_UP))?2:0);
  if(x||y) { mbr_output_state_t s={0};if(q->usb_mode==1){s.x=x;s.y=y;}else{s.pan=x/2;s.wheel=-y/2;}mbr_usb_submit(&s); }
 }
}
static void menu(MbrFrame *f,const char *title) {
 memset(f,0,sizeof(*f));f->hint=6;
 for(unsigned r=0;r<9;++r)for(unsigned c=0;c<21;++c)f->cells[r][c]=(MbrCell){' ',MBR_ACTION};
 mbr_frame_text(f,0,0,title,MBR_TITLE);
}
void mbr_qualification_frame(const MbrQualification *q,const MbrApp *a,MbrFrame *f) {
 if(q->mode==2) { MbrView v;mbr_app_view(a,&v);mbr_project(&v,f);return; }
 if(q->mode==1) {
  MbrApp fixture;memset(&fixture,0,sizeof(fixture));seed(&fixture,(MbrScreen)q->gallery);MbrView v;mbr_app_view(&fixture,&v);v.pressed=q->interaction.pressed;mbr_project(&v,f);return;
 }
 if(q->mode==0) {
  menu(f,"MBR-04 QUALIFICATION");mbr_frame_text(f,1,0,"RAM FIXTURES ONLY",MBR_BODY);
  const char *rows[]={" SCREEN GALLERY"," INTERACTIVE UI"," USB OUTPUT TESTS"};
  for(unsigned i=0;i<3;++i)mbr_frame_text(f,2+i,0,rows[i],q->selection==i?MBR_WHITE:MBR_ACTION);
  mbr_frame_text(f,6,0,"JOY UP / DOWN: SELECT",MBR_ACTION);mbr_frame_text(f,7,0,"JOY PRESS: ACCESS",MBR_ACTION);mbr_frame_text(f,8,0,"NO BLUETOOTH IN MBR04",MBR_ACTION);
 } else if(q->mode==3) {
  menu(f,"USB OUTPUT TESTS");const char *rows[]={" FIVE MOUSE BUTTONS"," X / Y MOVEMENT"," WHEEL / PAN"," ESCAPE HOLD"};
  for(unsigned i=0;i<4;++i)mbr_frame_text(f,1+i,0,rows[i],q->selection==i?MBR_WHITE:MBR_ACTION);
  mbr_frame_text(f,6,0,"JOY UP / DOWN: SELECT",MBR_ACTION);mbr_frame_text(f,7,0,"JOY PRESS: ACCESS",MBR_ACTION);mbr_frame_text(f,8,0,"KEY B: MENU",MBR_ACTION);
 } else {
  const char *titles[]={"USB FIVE BUTTONS","USB X / Y","USB WHEEL / PAN","USB ESCAPE HOLD"};menu(f,titles[q->usb_mode]);
  if(q->usb_mode==0) { mbr_frame_text(f,1,0,"UP: LEFT BUTTON",MBR_BODY);mbr_frame_text(f,2,0,"DOWN: RIGHT BUTTON",MBR_BODY);mbr_frame_text(f,3,0,"PRESS: MIDDLE BUTTON",MBR_BODY);mbr_frame_text(f,4,0,"LEFT: BACK BUTTON",MBR_BODY);mbr_frame_text(f,5,0,"RIGHT: FORWARD BUTTON",MBR_BODY); }
  else if(q->usb_mode==1)mbr_frame_text(f,2,0,"HOLD JOYSTICK TO MOVE",MBR_BODY);
  else if(q->usb_mode==2) {mbr_frame_text(f,2,0,"UP / DOWN: WHEEL",MBR_BODY);mbr_frame_text(f,3,0,"LEFT / RIGHT: PAN",MBR_BODY);}
  else {mbr_frame_text(f,2,0,"HOLD KEY A: ESCAPE",MBR_BODY);mbr_frame_text(f,3,0,"RELEASE A: RELEASE",MBR_BODY);}
  f->hint=8;mbr_frame_text(f,8,0,"KEY B: MENU",MBR_ACTION);
 }
}
