#include "mbr/ui_projector/ui_projector.h"
#include <stdio.h>
#include <string.h>
#define S(x) MBR_SCREEN_##x
bool mbr_screen_didactic(MbrScreen s) { return s==S(SEARCHING_FIRST)||s==S(FIRST_MOUSE_CONNECTED)||s==S(LEARN_THE_KEYS); }
void mbr_frame_text(MbrFrame *f,unsigned r,unsigned c,const char *s,MbrTone t) {
 if(r>=9) return;
 for(;*s && c<21;++c,++s) f->cells[r][c]=(MbrCell){*s,t};
}
static void row(MbrFrame *f,unsigned r,MbrTone t) { for(unsigned c=0;c<21;++c) f->cells[r][c].tone=t; }
static void span(MbrFrame *f,unsigned r,unsigned c,unsigned n) { for(unsigned i=0;i<n&&c+i<21;++i) f->cells[r][c+i].tone=MBR_WHITE; }
static void replace(MbrFrame *f,unsigned r,const char *s,MbrTone t) { for(unsigned c=0;c<21;++c) f->cells[r][c]=(MbrCell){' ',t}; mbr_frame_text(f,r,0,s,t); }
static bool down(const MbrView *v,MbrControl c) { return (v->pressed & (1u<<c))!=0; }
static void didactic(const MbrView *v,MbrFrame *f) {
 unsigned r=v->screen==S(SEARCHING_FIRST)?3:1;
 if(down(v,MBR_UP)) span(f,r,7,6);
 if(down(v,MBR_LEFT)) { span(f,r+1,2,3); span(f,r+2,2,4); }
 if(down(v,MBR_PRESS)) { span(f,r+1,9,3); span(f,r+2,8,5); }
 if(down(v,MBR_RIGHT)) { span(f,r+1,16,3); span(f,r+2,15,5); }
 if(down(v,MBR_DOWN)) span(f,r+3,6,8);
 if(v->screen==S(SEARCHING_FIRST)) {
  if(down(v,MBR_A)) span(f,7,1,5);
  if(down(v,MBR_X)) span(f,7,15,5);
  if(down(v,MBR_B)) span(f,8,1,5);
  if(down(v,MBR_Y)) span(f,8,15,5);
 } else {
  if(down(v,MBR_A)) span(f,5,15,5);
  if(down(v,MBR_B)) { span(f,6,15,5); span(f,6,0,11); }
  if(down(v,MBR_X)) { span(f,7,15,5); span(f,7,1,10); }
  if(down(v,MBR_Y)) span(f,8,2,18);
 }
}
void mbr_project(const MbrView *v,MbrFrame *f) {
 memset(f,0,sizeof(*f)); f->hint=9; f->didactic=mbr_screen_didactic(v->screen);
 if(v->screen>=MBR_SCREEN_COUNT) return;
 const MbrScreenTemplate *t=&mbr_screens[v->screen];
 bool is_help=strstr(t->id,"help")!=NULL;
 if(is_help) f->hint=8;
 if(!f->didactic&&!is_help) for(unsigned r=1;r<9;++r) {
  if(!strncmp(t->rows[r],"KEY ",4)||!strncmp(t->rows[r],"JOY ",4)||!strncmp(t->rows[r],"ANY KEY",7)) { f->hint=(uint8_t)r; break; }
 }
 for(unsigned r=0;r<9;++r) {
  MbrTone tone=r==0?MBR_TITLE:(f->didactic||r>=f->hint||t->rows[r][0]==' ')?MBR_ACTION:MBR_BODY;
  replace(f,r,t->rows[r],tone);
 }
 if(f->didactic) { didactic(v,f); return; }
 char text[64],name[22]; mbr_display_name(v->name,name);
 const char *profiles[]={"PASSTHROUGH","STANDARD","ESCAPE","CUSTOM"};
 const char *targets[]={"LEFT","RIGHT","MIDDLE","ESCAPE","FORWARD","BACKWARD"};
 const char *sources[]={"LEFT","RIGHT","MIDDLE","FORWARD","BACKWARD"};
 MbrProfile p=v->profile<=MBR_CUSTOM?v->profile:MBR_PASSTHROUGH;
 int selected=-1;
 if(v->screen==S(HOME_CONNECTED)) {
  replace(f,0,name,MBR_TITLE);
  if(p==MBR_PASSTHROUGH) strcpy(text," NO REMAP PASSTHROUGH");
  else snprintf(text,sizeof(text)," REMAPPED TO %s",profiles[p]);
  replace(f,2,text,MBR_CYAN); selected=1+v->selection;
 } else if(v->screen==S(HOME_SEARCHING)||v->screen==S(HOME_RETRY)) selected=1+v->selection;
 else if(v->screen==S(REMAPPER_OPTIONS)) { row(f,1+p,MBR_CYAN); selected=1+v->selection; }
 else if(v->screen==S(CUSTOM_EDIT)) {
  for(unsigned i=0;i<5;++i) { snprintf(text,sizeof(text)," %s IS %s",sources[i],targets[v->draft[i]<6?v->draft[i]:0]); replace(f,1+i,text,v->custom_applied?MBR_CYAN:MBR_ACTION); }
  selected=1+v->selection;
 } else if(v->screen>=S(LEFT)&&v->screen<=S(BACKWARD)) {
  unsigned source=(unsigned)(v->screen-S(LEFT)); row(f,1+v->draft[source],MBR_CYAN); selected=1+v->selection;
 } else if(v->screen==S(SAVED_DEVICES)) {
  snprintf(text,sizeof(text),"%u OF %u",(unsigned)(v->count?v->page+1:0),(unsigned)v->count); replace(f,0,text,MBR_TITLE);
  replace(f,1,name,v->connected?MBR_CYAN:MBR_BODY);
  replace(f,2,v->connected?"STATUS: CONNECTED":"STATUS: DISCONNECTED",MBR_BODY);
  snprintf(text,sizeof(text),"PROFILE: %s",profiles[p]); replace(f,3,text,MBR_BODY); selected=4;
 } else if(v->screen==S(REMOVE_THIS)) replace(f,1,name,MBR_BODY);
 else if(v->screen==S(PASSTHROUGH_ACTIVE)||v->screen==S(STANDARD_ACTIVE)||v->screen==S(ESCAPE_ACTIVE)) {
  for(unsigned r=1;r<f->hint;++r) row(f,r,MBR_CYAN);
 }
 if(selected>=0&&selected<9) row(f,(unsigned)selected,MBR_WHITE);
 const char *tokens[]={"JOY UP","JOY DOWN","JOY LEFT","JOY RIGHT","JOY PRESS","KEY A","KEY B","KEY X","KEY Y"};
 for(unsigned r=f->hint;r<9;++r) {
  for(unsigned c=0;c<MBR_CONTROL_COUNT;++c) if(down(v,(MbrControl)c)) {
   bool matches=strstr(t->rows[r],tokens[c])!=NULL;
   if((c==MBR_UP||c==MBR_DOWN)&&strstr(t->rows[r],"UP / DOWN")) matches=true;
   if((c==MBR_LEFT||c==MBR_RIGHT)&&strstr(t->rows[r],"RIGHT\\LEFT")) matches=true;
   if(strstr(t->rows[r],"ANY KEY")) matches=true;
   if(matches) row(f,r,MBR_WHITE);
  }
 }
}
