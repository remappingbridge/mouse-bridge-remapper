#include "mbr/pairing_coordinator/pairing_coordinator.h"
void mbr_search_start(MbrSearchTransaction *s,MbrSearch p,uint32_t now) {
    if(++s->generation==0) ++s->generation;
    s->purpose=p; s->started=now; s->duration=p==MBR_SEARCH_NEW?MBR_PAIR_MS:MBR_SAVED_MS;
}
void mbr_search_cancel(MbrSearchTransaction *s) { s->purpose=MBR_SEARCH_NONE; }
bool mbr_search_expired(const MbrSearchTransaction *s,uint32_t now) { return s->purpose!=MBR_SEARCH_NONE && (uint32_t)(now-s->started)>=s->duration; }
bool mbr_search_eligible(const MbrSearchTransaction *s,uint32_t gen,bool saved) {
    return s->purpose!=MBR_SEARCH_NONE && s->generation==gen && (s->purpose==MBR_SEARCH_SAVED?saved:!saved);
}
