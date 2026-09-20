#include "mbr/domain/domain.h"
#include <string.h>
void mbr_display_name(const char *name, char out[22]) {
    size_t n=0; bool useful=false;
    if(name) for(size_t i=0; i<MBR_NAME_CAPACITY && n<21 && name[i]; ++i) {
        unsigned char c=(unsigned char)name[i];
        if(c>='a' && c<='z') c=(unsigned char)(c-'a'+'A');
        if((c>='A'&&c<='Z')||(c>='0'&&c<='9')||strchr(" -:.=/\\><?!,()", c)) {
            out[n++]=(char)c; if(c!=' ') useful=true;
        }
    }
    out[n]=0;
    if(!useful) strcpy(out,"UNKNOWN MOUSE");
}

static bool word_char(unsigned char c) {
 return (c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_';
}
void mbr_home_title(const char *name,char out[22]) {
 /* Detect a complete word in the full bounded original name, before truncation. */
 char upper[MBR_NAME_CAPACITY+1];size_t n=0;bool mouse=false;
 if(name) for(;n<MBR_NAME_CAPACITY&&name[n];++n) {
  unsigned char c=(unsigned char)name[n];upper[n]=(char)(c>='a'&&c<='z'?c-'a'+'A':c);
 }
 upper[n]=0;
 for(size_t i=0;i+5<=n;++i) if(!memcmp(upper+i,"MOUSE",5)&&
   (!i||!word_char((unsigned char)upper[i-1]))&&
   (i+5==n||!word_char((unsigned char)upper[i+5]))) mouse=true;
 mbr_display_name(name,out);
 if(!strcmp(out,"UNKNOWN MOUSE")) return;
 if(strlen(out)>15)out[15]=0;
 n=strlen(out);while(n&&out[n-1]==' ')out[--n]=0;
 if(!mouse)strcat(out," MOUSE");
}
