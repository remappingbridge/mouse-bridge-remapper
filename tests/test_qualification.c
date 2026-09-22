#include "mbr/application/qualification.h"
#include <assert.h>
static mbr_output_state_t output;
bool mbr_usb_submit(const mbr_output_state_t *s){output=*s;return true;}
void mbr_usb_release_all(void){output=(mbr_output_state_t){0};}
static void tap(MbrQualification *q,MbrApp *a,MbrControl c){mbr_qualification_event(q,a,c,true);mbr_qualification_event(q,a,c,false);}
int main(void){
 MbrQualification q;MbrApp a;MbrFrame f;mbr_app_init(&a,0);mbr_qualification_init(&q);
 tap(&q,&a,MBR_PRESS);assert(q.mode==1);
 for(unsigned s=0;s<MBR_SCREEN_COUNT;++s){assert(q.gallery==s);mbr_qualification_frame(&q,&a,&f);tap(&q,&a,MBR_A);}assert(q.gallery==0);
 tap(&q,&a,MBR_Y);tap(&q,&a,MBR_DOWN);tap(&q,&a,MBR_DOWN);tap(&q,&a,MBR_PRESS);assert(q.mode==3);
 tap(&q,&a,MBR_PRESS);assert(q.mode==4);mbr_qualification_event(&q,&a,MBR_UP,true);assert(output.buttons==1);mbr_qualification_event(&q,&a,MBR_UP,false);assert(output.buttons==0);
 tap(&q,&a,MBR_B);for(unsigned i=0;i<3;++i)tap(&q,&a,MBR_DOWN);tap(&q,&a,MBR_PRESS);assert(q.usb_mode==3);mbr_qualification_event(&q,&a,MBR_A,true);assert(output.escape);mbr_qualification_event(&q,&a,MBR_A,false);assert(!output.escape);tap(&q,&a,MBR_B);assert(!output.escape&&q.mode==3);
 mbr_qualification_init(&q);tap(&q,&a,MBR_PRESS);tap(&q,&a,MBR_X);assert(q.mode==2&&a.screen==MBR_SCREEN_SEARCHING_FIRST);mbr_app_tick(&a,3000);mbr_qualification_tick(&q,&a,3000);assert(a.request.kind==MBR_OP_HANDOFF);mbr_qualification_tick(&q,&a,3500);assert(a.screen==MBR_SCREEN_FIRST_MOUSE_CONNECTED);
 return 0;
}
