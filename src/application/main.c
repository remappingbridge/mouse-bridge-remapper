#include "mbr/application/application.h"
#ifndef MBR_QUALIFICATION
#include "mbr/application/bridge.h"
#endif
#include "mbr/renderer/renderer.h"
#include "mbr/hat/hat.h"
#include "mbr/usb_hid/usb_hid.h"
#ifdef MBR_QUALIFICATION
#include "mbr/application/qualification.h"
#endif
#include "pico/stdlib.h"
#include <string.h>
static uint16_t pixels[240*240];
int main(void) {
 static MbrApp app;static MbrHat hat;static MbrFrame frame,previous;bool have_frame=false;unsigned flush_y=240;
 mbr_app_init(&app,to_ms_since_boot(get_absolute_time()));mbr_hat_init();mbr_display_init();mbr_usb_init();
#ifdef MBR_QUALIFICATION
 static MbrQualification qualification;mbr_qualification_init(&qualification);
#endif
 #ifndef MBR_QUALIFICATION
 static MbrBridge bridge;bool radio_attempted=false;
 #endif
 uint32_t scan=0;
 for(;;) {
  uint32_t now=to_ms_since_boot(get_absolute_time());mbr_usb_task();mbr_app_tick(&app,now);
#ifndef MBR_QUALIFICATION
  if(have_frame&&flush_y==240&&!radio_attempted) {radio_attempted=true;(void)mbr_bt_start();}
  if(radio_attempted)mbr_bridge_task(&bridge,&app);
#endif
  if((uint32_t)(now-scan)>=1) {
   scan=now;mbr_hat_sample(&hat,mbr_hat_read(),now);
   if(hat.overflow) { memset(&app.interaction,0,sizeof(app.interaction));hat.overflow=false;mbr_usb_release_all();
#ifdef MBR_QUALIFICATION
    memset(&qualification.interaction,0,sizeof(qualification.interaction));
#else
    mbr_bridge_release(&bridge,&app);
#endif
   }
   MbrHatEvent e;while(mbr_hat_pop(&hat,&e)) {
#ifdef MBR_QUALIFICATION
    mbr_qualification_event(&qualification,&app,e.control,e.down);
#else
    mbr_app_event(&app,e.control,e.down);
#endif
   }
  }
#ifdef MBR_QUALIFICATION
  mbr_qualification_tick(&qualification,&app,now);
#endif
  if(flush_y==240) {
#ifdef MBR_QUALIFICATION
   mbr_qualification_frame(&qualification,&app,&frame);
#else
   MbrView view;mbr_app_view(&app,&view);mbr_project(&view,&frame);
#endif
   if(!have_frame||memcmp(&frame,&previous,sizeof(frame))) { mbr_render(&frame,pixels);previous=frame;have_frame=true;flush_y=0; }
  }
  if(flush_y<240) { mbr_display_rows(flush_y,2,pixels+flush_y*240);flush_y+=2; }
  mbr_display_backlight(have_frame&&!app.locked);
  tight_loop_contents();
 }
}
