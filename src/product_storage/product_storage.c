#include "mbr/product_storage/product_storage.h"
#include <string.h>
#define STRIDE 76u
#define HEADER 32u
static uint32_t get32(const uint8_t *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);
 }
static void put32(uint8_t *p,uint32_t x){for(unsigned i=0;i<4;++i)p[i]=(uint8_t)(x>>(8*i));
 }
static uint32_t crc(const uint8_t *p,size_t n){uint32_t c=~0u;
 for(size_t i=0;i<n;++i){c^=p[i];
 for(unsigned j=0;j<8;++j)c=(c>>1)^(0xedb88320u& (uint32_t)-(int32_t)(c&1));
 }return ~c;
 }
void mbr_product_defaults(MbrProduct *p){memset(p,0,sizeof(*p));
 const MbrTarget d[]={MBR_TARGET_LEFT,MBR_TARGET_RIGHT,MBR_TARGET_MIDDLE,MBR_TARGET_FORWARD,MBR_TARGET_BACKWARD};
 memcpy(p->custom,d,sizeof(d));
 memcpy(p->draft,d,sizeof(d));
 }
static void mouse_encode(uint8_t *d,const MbrMouse *m){put32(d,m->id);
 memcpy(d+4,m->name,64);
 memcpy(d+68,m->address,6);
 d[74]=m->address_type;
 d[75]=(uint8_t)(m->profile|(m->bonded?0x80:0));
 }
static bool mouse_decode(const uint8_t *d,MbrMouse *m){memset(m,0,sizeof(*m));
 m->id=get32(d);
 memcpy(m->name,d+4,64);
 if(m->name[63])return false;
 memcpy(m->address,d+68,6);
 m->address_type=d[74];
 m->profile=(MbrProfile)(d[75]&0x7f);
 m->bonded=(d[75]&0x80)!=0;
 return m->profile<=MBR_CUSTOM&&m->address_type<=3;
 }
bool mbr_storage_encode(const MbrProduct *p,uint32_t g,uint8_t out[MBR_STORAGE_RECORD_SIZE]){
 if(p->registry.count>MBR_SAVED_CAPACITY)return false;
 memset(out,0,MBR_STORAGE_RECORD_SIZE);
 memcpy(out,"MBR8",4);
 put32(out+4,1);
 put32(out+8,g);
 put32(out+12,MBR_STORAGE_RECORD_SIZE);
 out[16]=(uint8_t)p->registry.count;
 out[17]=p->dirty;
 for(unsigned i=0;i<5;++i){if(p->custom[i]>MBR_TARGET_BACKWARD||p->draft[i]>MBR_TARGET_BACKWARD)return false;
 out[18+i]=(uint8_t)p->custom[i];
 out[23+i]=(uint8_t)p->draft[i];
 }
 for(size_t i=0;i<p->registry.count;++i){if(!p->registry.mice[i].id)return false;
 mouse_encode(out+HEADER+i*STRIDE,&p->registry.mice[i]);
 }
 mouse_encode(out+HEADER+MBR_SAVED_CAPACITY*STRIDE,&p->pending_remove);
 put32(out+MBR_STORAGE_RECORD_SIZE-4,crc(out,MBR_STORAGE_RECORD_SIZE-4));
 return true;
}
bool mbr_storage_decode(const uint8_t d[MBR_STORAGE_RECORD_SIZE],MbrProduct *p,uint32_t *g){
 if(memcmp(d,"MBR8",4)||get32(d+4)!=1||get32(d+12)!=MBR_STORAGE_RECORD_SIZE||get32(d+MBR_STORAGE_RECORD_SIZE-4)!=crc(d,MBR_STORAGE_RECORD_SIZE-4)||d[16]>MBR_SAVED_CAPACITY||d[17]>1)return false;
 mbr_product_defaults(p);
 p->dirty=d[17]!=0;
 for(unsigned i=0;i<5;++i){if(d[18+i]>MBR_TARGET_BACKWARD||d[23+i]>MBR_TARGET_BACKWARD)return false;
 p->custom[i]=(MbrTarget)d[18+i];
 p->draft[i]=(MbrTarget)d[23+i];
 }
 for(unsigned i=0;i<d[16];++i){MbrMouse m;
 if(!mouse_decode(d+HEADER+i*STRIDE,&m)||!m.id||mbr_registry_find(&p->registry,m.id)>=0||!mbr_registry_put(&p->registry,&m))return false;
 }
 if(!mouse_decode(d+HEADER+MBR_SAVED_CAPACITY*STRIDE,&p->pending_remove))return false;
 if(p->pending_remove.id&&mbr_registry_find(&p->registry,p->pending_remove.id)>=0)return false;
 if(g)*g=get32(d+8);
 return true;
}
/* Private static scratch: the storage service is single-owner, never in callbacks. */
static uint8_t records[2][MBR_STORAGE_RECORD_SIZE],encoded[MBR_STORAGE_RECORD_SIZE];
static MbrProduct decoded[2];
static int newest(const MbrStorageIO *io,uint32_t gen[2]){
 bool valid[2];
 for(unsigned i=0;i<2;++i)valid[i]=io->read(io->context,i,records[i])&&mbr_storage_decode(records[i],&decoded[i],&gen[i]);
 if(!valid[0])return valid[1]?1:-1;
 if(!valid[1])return 0;
 return (int32_t)(gen[1]-gen[0])>0?1:0;
}
bool mbr_storage_load(const MbrStorageIO *io,MbrProduct *p){uint32_t gen[2]={0};
 int n=newest(io,gen);
 if(n<0){mbr_product_defaults(p);
 return false;
 }*p=decoded[n];
 return true;
 }
bool mbr_storage_commit(const MbrStorageIO *io,const MbrProduct *p){
 uint32_t gen[2]={0};
 int n=newest(io,gen);
 unsigned target=n==0?1:0;
 uint32_t next=n<0?1:gen[n]+1;
 if(!mbr_storage_encode(p,next,encoded))return false;
 /* Unchanged reconnects must not erase another flash sector. */
 if(n>=0&&!memcmp(encoded+16,records[n]+16,MBR_STORAGE_RECORD_SIZE-20))return true;
 if(!mbr_storage_decode(encoded,&decoded[target],NULL))return false;
 if(!io->write(io->context,target,encoded)||!io->read(io->context,target,records[target]))return false;
 return !memcmp(encoded,records[target],MBR_STORAGE_RECORD_SIZE)&&mbr_storage_decode(records[target],&decoded[target],NULL);
}
