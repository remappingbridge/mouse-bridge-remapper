#pragma once
#include "mbr/domain/domain.h"
typedef struct { MbrSearch purpose; uint32_t generation, started, duration; } MbrSearchTransaction;
void mbr_search_start(MbrSearchTransaction *s,MbrSearch purpose,uint32_t now);
void mbr_search_cancel(MbrSearchTransaction *s);
bool mbr_search_expired(const MbrSearchTransaction *s,uint32_t now);
bool mbr_search_eligible(const MbrSearchTransaction *s,uint32_t generation,bool saved);
