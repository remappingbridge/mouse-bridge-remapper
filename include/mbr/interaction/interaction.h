#pragma once
#include "mbr/domain/screens.h"
typedef struct { uint16_t pressed; uint32_t owners[MBR_CONTROL_COUNT]; } MbrInteraction;
bool mbr_interaction_event(MbrInteraction *i,MbrControl c,bool down,uint32_t epoch);
