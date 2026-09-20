#include "mbr/domain/domain.h"
#include <string.h>
void mbr_display_name(const char *name, char out[22]) {
    size_t n=0; bool useful=false;
    if(name) for(size_t i=0; name[i] && i<MBR_NAME_CAPACITY && n<21; ++i) {
        unsigned char c=(unsigned char)name[i];
        if(c>='a' && c<='z') c=(unsigned char)(c-'a'+'A');
        if((c>='A'&&c<='Z')||(c>='0'&&c<='9')||strchr(" -:.=/\\><?!,()", c)) {
            out[n++]=(char)c; if(c!=' ') useful=true;
        }
    }
    out[n]=0;
    if(!useful) strcpy(out,"UNKNOWN MOUSE");
}
