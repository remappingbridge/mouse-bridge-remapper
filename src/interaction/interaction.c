#include "mbr/interaction/interaction.h"
bool mbr_interaction_event(MbrInteraction *i,MbrControl c,bool down,uint32_t epoch) {
    if(c>=MBR_CONTROL_COUNT) return false;
    uint16_t bit=(uint16_t)(1u<<c);
    if(down) { if(!(i->pressed&bit)) i->owners[c]=epoch; i->pressed|=bit; return false; }
    bool act=(i->pressed&bit) && i->owners[c]==epoch;
    i->pressed&=(uint16_t)~bit; return act;
}
