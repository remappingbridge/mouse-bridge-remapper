#pragma once
#include "mbr/profiles/profiles.h"
#include "mbr/output_state/output_state.h"
bool mbr_remap_event(MbrOutput *output,const MbrMouseEvent *event,MbrProfile profile,const MbrTarget custom[5]);
