#include "mbr/application/application.h"
#include "mbr/renderer/renderer.h"
#include "mbr/hat/hat.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint16_t pixels[240*240];
static void ppm(const char *dir,int screen) {
 char path[512];snprintf(path,sizeof(path),"%s/%02d-%s.ppm",dir,screen,mbr_screens[screen].id);
 FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n240 240\n255\n");
 for(unsigned i=0;i<240*240;++i) { uint16_t p=pixels[i];unsigned char rgb[]={(unsigned char)(((p>>11)&31)*255/31),(unsigned char)(((p>>5)&63)*255/63),(unsigned char)((p&31)*255/31)};assert(fwrite(rgb,1,3,f)==3); } fclose(f);
}
int main(int argc,char **argv) {
 MbrView v={.name="LOGITECH LIFT",.count=4,.page=2,.profile=MBR_STANDARD,.connected=true,.draft={MBR_TARGET_LEFT,MBR_TARGET_RIGHT,MBR_TARGET_MIDDLE,MBR_TARGET_FORWARD,MBR_TARGET_BACKWARD}};
 for(int s=0;s<MBR_SCREEN_COUNT;++s) {
  v.screen=(MbrScreen)s;MbrFrame f;mbr_project(&v,&f);mbr_render(&f,pixels);
  assert(mbr_text_y(&f,0)==8); assert(mbr_text_y(&f,1)==39); assert(7+20*11+10<=240);
  unsigned last_bottom=22;
  for(unsigned r=1;r<9;++r) {
   bool visible=false;for(unsigned c=0;c<21;++c) if(f.cells[r][c].character!=' ')visible=true;
   if(!visible)continue;
   unsigned y=mbr_text_y(&f,r);assert(y>=last_bottom);assert(y+14<=240);last_bottom=y+14;
   if(!f.didactic&&r>=f.hint) for(unsigned c=0;c<21;++c) assert(f.cells[r][c].tone==MBR_ACTION);
  }
  if(strstr(mbr_screens[s].id,"help")) { assert(f.hint==8); for(unsigned r=1;r<=6;++r) assert(f.cells[r][0].tone==MBR_BODY); }
  if(f.hint<9) { unsigned boundary=mbr_hint_boundary(&f);assert(boundary+11==mbr_text_y(&f,f.hint));assert(pixels[boundary*240]==MBR_DARK_MAGENTA); }
  if(argc>1)ppm(argv[1],s);
 }
 assert(mbr_tone_rgb565(MBR_ACTION)==0xc618);assert(mbr_tone_rgb565(MBR_WHITE)==0xffff);
 MbrHat h={0};MbrHatEvent e; mbr_hat_sample(&h,1,0);mbr_hat_sample(&h,0,5);mbr_hat_sample(&h,1,10);mbr_hat_sample(&h,1,29);assert(!mbr_hat_pop(&h,&e));mbr_hat_sample(&h,1,30);assert(mbr_hat_pop(&h,&e)&&e.down&&e.control==MBR_UP);assert(!mbr_hat_pop(&h,&e));mbr_hat_sample(&h,0,31);mbr_hat_sample(&h,0,51);assert(mbr_hat_pop(&h,&e)&&!e.down);
 for(unsigned i=0;i<80;++i) { mbr_hat_sample(&h,(i%2)?0:1,100+i*21);mbr_hat_sample(&h,(i%2)?0:1,120+i*21); }assert(h.overflow);
 return 0;
}
