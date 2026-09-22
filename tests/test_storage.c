#include "mbr/product_storage/product_storage.h"
#include <assert.h>
#include <string.h>
static uint8_t flash[2][MBR_STORAGE_RECORD_SIZE];static int cut=-1;
static bool read_slot(void *ctx,unsigned slot,uint8_t *out){(void)ctx;memcpy(out,flash[slot],MBR_STORAGE_RECORD_SIZE);return true;}
static bool write_slot(void *ctx,unsigned slot,const uint8_t *in){(void)ctx;memset(flash[slot],255,MBR_STORAGE_RECORD_SIZE);size_t n=cut<0?MBR_STORAGE_RECORD_SIZE:(size_t)cut;memcpy(flash[slot],in,n);return cut<0;}
int main(void){
 const MbrStorageIO io={NULL,read_slot,write_slot};MbrProduct p,q;mbr_product_defaults(&p);memset(flash,255,sizeof(flash));assert(!mbr_storage_load(&io,&q));
 MbrMouse m={.id=42,.name="LOGITECH LIFT",.profile=MBR_ESCAPE,.address={1,2,3,4,5,6},.address_type=1,.bonded=true};assert(mbr_registry_put(&p.registry,&m));assert(mbr_storage_commit(&io,&p));
 p.draft[0]=MBR_TARGET_ESCAPE;p.dirty=true;assert(mbr_storage_commit(&io,&p));assert(mbr_storage_load(&io,&q));assert(q.dirty&&q.draft[0]==MBR_TARGET_ESCAPE&&q.custom[0]==MBR_TARGET_LEFT&&q.registry.mice[0].bonded);
 static uint8_t baseline[2][MBR_STORAGE_RECORD_SIZE];memcpy(baseline,flash,sizeof(flash));
 for(int n=0;n<(int)MBR_STORAGE_RECORD_SIZE;n+=31){memcpy(flash,baseline,sizeof(flash));cut=n;p.registry.mice[0].profile=MBR_STANDARD;assert(!mbr_storage_commit(&io,&p));assert(mbr_storage_load(&io,&q));assert(q.registry.mice[0].profile==MBR_ESCAPE&&q.dirty);}
 cut=-1;memcpy(flash,baseline,sizeof(flash));flash[1][40]^=1;assert(mbr_storage_load(&io,&q));assert(!q.dirty);
 memcpy(flash,baseline,sizeof(flash));p.pending_remove=p.registry.mice[0];assert(mbr_registry_remove(&p.registry,42));assert(mbr_storage_commit(&io,&p));assert(mbr_storage_load(&io,&q));assert(q.pending_remove.id==42&&!q.registry.count);
 uint8_t record[MBR_STORAGE_RECORD_SIZE];uint32_t gen;assert(mbr_storage_encode(&p,UINT32_MAX,record));assert(mbr_storage_decode(record,&q,&gen)&&gen==UINT32_MAX);record[4]=2;assert(!mbr_storage_decode(record,&q,&gen));
 return 0;
}
