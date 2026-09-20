#include "mbr/ble_hogp/ble_hogp.h"
#include "mbr/usb_hid/usb_hid.h"
#include <assert.h>
#include <string.h>
static MbrMouseEvent events[16];static unsigned count;
static bool emit(void *ctx,const MbrMouseEvent *e) {(void)ctx;assert(count<16);events[count++]=*e;return true;}
static bool fail(void *ctx,const MbrMouseEvent *e) {(void)ctx;(void)e;return false;}
// Five buttons, signed 12-bit XY, wheel and AC Pan on separate report ID.
static const uint8_t map[]={
 0x05,1,0x09,2,0xa1,1,0x85,1,0x09,1,0xa1,0,
 0x05,9,0x19,1,0x29,5,0x15,0,0x25,1,0x75,1,0x95,5,0x81,2,
 0x75,3,0x95,1,0x81,1,
 0x05,1,0x09,0x30,0x09,0x31,0x16,0,0xf8,0x26,0xff,7,0x75,12,0x95,2,0x81,6,
 0x85,2,0x09,0x38,0x15,0x81,0x25,0x7f,0x75,8,0x95,1,0x81,6,
 0x05,0x0c,0x0a,0x38,2,0x81,6,0xc0,0xc0};
int main(void) {
 MbrHogpParser p;assert(mbr_hogp_configure(&p,42,map,sizeof(map)));assert(mbr_hogp_has_mouse(&p));
 const uint8_t raw[]={0x19,0xff,0x0f,0x80}; // left/forward/back, x=-1,y=-2048
 assert(mbr_hogp_parse(&p,1,raw,sizeof(raw),emit,NULL));assert(count==4);
 assert(events[0].session==42&&events[0].data.button.button==0&&events[0].data.button.pressed);
 assert(events[1].data.button.button==3&&events[2].data.button.button==4);
 assert(events[3].type==MBR_MOUSE_MOVE&&events[3].data.move.dx==-1&&events[3].data.move.dy==-2048);
 count=0;uint8_t duplicate[]={1,0x19,0xff,0x0f,0x80};
 assert(mbr_hogp_parse(&p,1,duplicate,sizeof(duplicate),emit,NULL)&&count==1); // duplicate Down idempotent
 uint8_t badprefix[]={2,0x19,0xff,0x0f,0x80};assert(!mbr_hogp_parse(&p,1,badprefix,sizeof(badprefix),emit,NULL));
 assert(!mbr_hogp_parse(&p,1,raw,3,emit,NULL));assert(!mbr_hogp_parse(&p,99,raw,4,emit,NULL));
 count=0;const uint8_t scroll[]={0xfe,3};assert(mbr_hogp_parse(&p,2,scroll,2,emit,NULL));
 assert(count==1&&events[0].type==MBR_MOUSE_WHEEL&&events[0].data.wheel.vertical==-2&&events[0].data.wheel.horizontal==3);
 count=0;const uint8_t up[]={0,0,0,0};assert(mbr_hogp_parse(&p,1,up,4,emit,NULL)&&count==3);
 for(unsigned i=0;i<3;++i)assert(!events[i].data.button.pressed);
 assert(!mbr_hogp_parse(&p,1,raw,4,fail,NULL)); // caller must fail closed on sink overflow
 assert(!mbr_hogp_configure(&p,0,map,sizeof(map))&&!p.configured);
 // Truncated maps, unbalanced collection/global stack, invalid size/id reject.
 for(size_t i=0;i<sizeof(map);++i)assert(!mbr_hogp_configure(&p,1,map,i));
 uint8_t bad[sizeof(map)];memcpy(bad,map,sizeof(map));bad[7]=0;assert(!mbr_hogp_configure(&p,1,bad,sizeof(bad)));
 memcpy(bad,map,sizeof(map));bad[3]=6;assert(!mbr_hogp_configure(&p,1,bad,sizeof(bad))); // keyboard
 uint8_t composite[sizeof(map)+6];memcpy(composite,map,sizeof(map));memcpy(composite+sizeof(map),(uint8_t[]){0x09,6,0xa1,1,0xc0,0},6);
 assert(!mbr_hogp_configure(&p,1,composite,sizeof(composite)-1));
 // Accepted USB Mouse descriptor is also a no-ID canonical test fixture.
 size_t length;const uint8_t *usb=mbr_usb_report_descriptor(0,&length);assert(mbr_hogp_configure(&p,2,usb,length));
 count=0;uint8_t report[]={31,127,0x81,1,0xff};assert(mbr_hogp_parse(&p,0,report,5,emit,NULL)&&count==7);
 uint8_t zero_prefix[]={0,31,127,0x81,1,0xff};count=0;assert(mbr_hogp_parse(&p,0,zero_prefix,6,emit,NULL)); // BTstack prefixes even ID zero
 return 0;
}
