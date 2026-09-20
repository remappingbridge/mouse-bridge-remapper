#include <assert.h>
#include <string.h>
#include "mbr/mouse_session/mouse_session.h"
#include "mbr/mouse_registry/mouse_registry.h"
#include "mbr/pairing_coordinator/pairing_coordinator.h"
int main(void) {
 MbrSessions s={0}; assert(mbr_session_promote(&s,1)); assert(!mbr_session_promote(&s,2)); assert(!mbr_session_clear(&s,999)); assert(mbr_session_clear(&s,s.live.generation)); assert(mbr_session_promote(&s,2)); assert(s.live.generation==2);
 MbrRegistry r={0}; MbrMouse m={.id=1}; assert(mbr_registry_put(&r,&m)); assert(mbr_registry_put(&r,&m)); assert(r.count==1); assert(mbr_registry_remove(&r,1)); assert(r.count==0);
 MbrSearchTransaction t={0}; mbr_search_start(&t,MBR_SEARCH_NEW,UINT32_MAX-100); assert(!mbr_search_expired(&t,50)); assert(mbr_search_expired(&t,15000)); assert(!mbr_search_eligible(&t,t.generation,true)); assert(mbr_search_eligible(&t,t.generation,false));
 char name[22]; mbr_display_name("abcdefghijklmnopqrstuvwxyz",name); assert(strcmp(name,"ABCDEFGHIJKLMNOPQRSTU")==0); mbr_display_name("\xff",name); assert(strcmp(name,"UNKNOWN MOUSE")==0);
 const char *in[]={"LIFT","mouse generic","XPTO ULTRA 2714","ABCDEFGHIJKLMNOPQRSTUV","MOUSEPAD","AB MOUSE CD","abcdefghijklmnop MOUSE","","   ",NULL};
 const char *out[]={"LIFT MOUSE","MOUSE GENERIC","XPTO ULTRA 2714 MOUSE","ABCDEFGHIJKLMNO MOUSE","MOUSEPAD MOUSE","AB MOUSE CD","ABCDEFGHIJKLMNO","UNKNOWN MOUSE","UNKNOWN MOUSE","UNKNOWN MOUSE"};
 for(unsigned i=0;i<sizeof(in)/sizeof(in[0]);++i) {mbr_home_title(in[i],name);assert(strcmp(name,out[i])==0);assert(strlen(name)<=21);}
 return 0;
}
