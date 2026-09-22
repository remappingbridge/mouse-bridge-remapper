#include "mbr/profiles/profiles.h"
#include <string.h>
bool mbr_profiles_mapping(MbrProfile p,const MbrTarget custom[5],MbrTarget out[5]) {
 static const MbrTarget fixed[3][5]={
  {MBR_TARGET_LEFT,MBR_TARGET_RIGHT,MBR_TARGET_MIDDLE,MBR_TARGET_BACKWARD,MBR_TARGET_FORWARD},
  {MBR_TARGET_FORWARD,MBR_TARGET_BACKWARD,MBR_TARGET_MIDDLE,MBR_TARGET_RIGHT,MBR_TARGET_LEFT},
  {MBR_TARGET_ESCAPE,MBR_TARGET_BACKWARD,MBR_TARGET_FORWARD,MBR_TARGET_RIGHT,MBR_TARGET_LEFT}};
 if(p<MBR_CUSTOM){memcpy(out,fixed[p],sizeof(fixed[0]));return true;}
 if(p!=MBR_CUSTOM||!custom)return false;
 for(unsigned i=0;i<5;++i)if(custom[i]>MBR_TARGET_BACKWARD)return false;
 out[0]=custom[0];out[1]=custom[1];out[2]=custom[2];out[3]=custom[4];out[4]=custom[3];return true;
}
