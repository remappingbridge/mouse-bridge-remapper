#include "mbr/hat/hat.h"
const uint8_t mbr_hat_pins[MBR_CONTROL_COUNT]={2,18,16,20,3,15,17,19,21};
void mbr_hat_sample(MbrHat *h,uint16_t pressed,uint32_t now) {
 for(unsigned i=0;i<MBR_CONTROL_COUNT;++i) {
  bool down=(pressed&(1u<<i))!=0;
  if(down!=h->raw[i]) { h->raw[i]=down; h->changed[i]=now; }
  if(down!=h->stable[i] && (uint32_t)(now-h->changed[i])>=20) {
   h->stable[i]=down;
   uint8_t next=(uint8_t)((h->write+1)%32);
   if(next==h->read) { h->overflow=true; h->read=h->write; return; }
   h->queue[h->write]=(MbrHatEvent){(MbrControl)i,down};h->write=next;
  }
 }
}
bool mbr_hat_pop(MbrHat *h,MbrHatEvent *e) { if(h->read==h->write) return false; *e=h->queue[h->read];h->read=(uint8_t)((h->read+1)%32);return true; }
