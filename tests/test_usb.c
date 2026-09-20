#include "mbr/usb_hid/usb_hid.h"
#include <assert.h>
#include <string.h>
typedef struct { bool ready[2]; int x,y,wheel,pan; unsigned mouse_count,escape_count; uint8_t buttons,key; } Host;
static bool send(void *ctx,unsigned i,const void *r,size_t n) {
 Host *h=ctx;if(!h->ready[i])return false;const uint8_t *p=r;
 if(i==0){assert(n==5);h->buttons=p[0];h->x+=(int8_t)p[1];h->y+=(int8_t)p[2];h->wheel+=(int8_t)p[3];h->pan+=(int8_t)p[4];++h->mouse_count;}
 else {assert(n==8);h->key=p[2];assert(h->key==0||h->key==0x29);assert(p[0]==0&&p[1]==0);++h->escape_count;}
 return true;
}
static void descriptors(void) {
 size_t n;const uint8_t *d=mbr_usb_device_descriptor(&n);assert(n==18&&d[8]==0xfe&&d[9]==0xca&&d[10]==0x11&&d[11]==0x40&&d[12]==0&&d[13]==1&&d[16]==0);
 d=mbr_usb_configuration_descriptor(&n);assert(n==59&&d[2]==n&&d[4]==2);unsigned interfaces=0,endpoints=0;
 for(size_t i=0;i<n;i+=d[i]){assert(d[i]>=2&&i+d[i]<=n);if(d[i+1]==4){assert(d[i+2]==interfaces&&d[i+5]==3);++interfaces;}if(d[i+1]==5){assert(d[i+2]==0x81+endpoints);++endpoints;}}
 assert(interfaces==2&&endpoints==2);assert(strcmp(mbr_usb_string(1),"tiagooliveirajs")==0);assert(strcmp(mbr_usb_string(2),"Mouse Bridge Remapper")==0);assert(!mbr_usb_string(3));
 // Interpret HID short items to verify exact input report widths, no report IDs.
 for(unsigned itf=0;itf<2;++itf){d=mbr_usb_report_descriptor(itf,&n);unsigned bits=0,size=0,count=0;for(size_t pos=0;pos<n;){uint8_t tag=d[pos++];assert(tag!=0xfe&&tag!=0x85);unsigned len=tag&3;if(len==3)len=4;unsigned val=0;for(unsigned j=0;j<len;++j)val|=(unsigned)d[pos++]<<(j*8);if(tag==0x75)size=val;if(tag==0x95)count=val;if(tag==0x81)bits+=size*count;}assert(bits==(itf?64u:40u));}
}
int main(void) {
 descriptors();MbrMouseReport m;MbrEscapeReport e;
 mbr_output_state_t s={.buttons=255,.escape=true,.x=1000,.y=-1000,.wheel=10,.pan=-5};mbr_usb_reports(&s,&m,&e);assert(m.bytes[0]==31&&m.bytes[1]==127&&(int8_t)m.bytes[2]==-127&&e.bytes[2]==0x29);
 MbrUsbQueue q={0};Host h={0};assert(mbr_usb_enqueue(&q,&s));mbr_usb_drain(&q,send,&h);assert(h.mouse_count==0&&q.queue[q.read].x==1000);
 h.ready[0]=true;mbr_usb_drain(&q,send,&h);assert(h.x==127);mbr_usb_drain(&q,send,&h);assert(h.x==127); // no duplicate when other endpoint blocks
 h.ready[1]=true;for(unsigned i=0;i<20;++i)mbr_usb_drain(&q,send,&h);assert(h.x==1000&&h.y==-1000&&h.wheel==10&&h.pan==-5&&h.key==0x29);
 s=(mbr_output_state_t){0};assert(mbr_usb_enqueue(&q,&s));mbr_usb_drain(&q,send,&h);assert(h.key==0&&h.buttons==0);
 // Backpressure preserves press then release, including the Escape endpoint.
 h.ready[1]=false;s.escape=true;assert(mbr_usb_enqueue(&q,&s));s.escape=false;assert(mbr_usb_enqueue(&q,&s));mbr_usb_drain(&q,send,&h);h.ready[1]=true;mbr_usb_drain(&q,send,&h);assert(h.key==0x29);mbr_usb_drain(&q,send,&h);assert(h.key==0);
 // Overflow discards uncertain held state and queues all-up.
 bool overflow=false;s.buttons=31;s.escape=true;for(unsigned i=0;i<32;++i)if(!mbr_usb_enqueue(&q,&s))overflow=true;assert(overflow);mbr_usb_drain(&q,send,&h);assert(h.key==0&&h.buttons==0);
 return 0;
}
