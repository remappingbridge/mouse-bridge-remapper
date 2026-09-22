#include "mbr/mouse_registry/mouse_registry.h"
#include <string.h>
int mbr_registry_find(const MbrRegistry *r,MouseId id) { for(size_t i=0;i<r->count;++i) if(r->mice[i].id==id) return (int)i; return -1; }
bool mbr_registry_put(MbrRegistry *r,const MbrMouse *m) {
    if(!m||!m->id||m->profile>MBR_CUSTOM) return false;
    int i=mbr_registry_find(r,m->id);
    if(i<0) { if(r->count==MBR_SAVED_CAPACITY) return false; i=(int)r->count++; }
    r->mice[i]=*m; r->mice[i].name[MBR_NAME_CAPACITY-1]=0; return true;
}
bool mbr_registry_remove(MbrRegistry *r,MouseId id) {
    int i=mbr_registry_find(r,id); if(i<0) return false;
    memmove(&r->mice[i],&r->mice[i+1],(r->count-(size_t)i-1)*sizeof(r->mice[0])); --r->count; return true;
}
