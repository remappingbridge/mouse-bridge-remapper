#include "mbr/remap/remap.h"
bool mbr_remap_event(MbrOutput *o,const MbrMouseEvent *e,MbrProfile p,const MbrTarget custom[5]) {
 MbrTarget map[5];return mbr_profiles_mapping(p,custom,map)&&mbr_output_event(o,e,map);
}
