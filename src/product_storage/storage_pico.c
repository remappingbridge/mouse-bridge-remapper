/* G06 dual-generation flash placement, adapted for a complete 16-Mouse registry. */
#include "mbr/product_storage/product_storage.h"
#include "hardware/flash.h"
#include "pico/flash.h"
#include "pico.h"
#include <string.h>
#if PICO_RP2350 && PICO_RP2350_A2_SUPPORTED
#define SDK_TAIL 3u
#else
#define SDK_TAIL 2u
#endif
#define PRODUCT_OFFSET (PICO_FLASH_SIZE_BYTES-(SDK_TAIL+2u)*FLASH_SECTOR_SIZE)
#define READ_BASE XIP_NOCACHE_NOALLOC_NOTRANSLATE_BASE
_Static_assert(MBR_STORAGE_RECORD_SIZE<=FLASH_SECTOR_SIZE,"Record fits a sector");
static bool safe(void){extern char __flash_binary_end;
 return (uintptr_t)&__flash_binary_end-XIP_BASE<=PRODUCT_OFFSET;
 }
static bool read_slot(void *ctx,unsigned slot,uint8_t *data){(void)ctx;
 if(slot>1||!safe())return false;
 memcpy(data,(const void *)(uintptr_t)(READ_BASE+PRODUCT_OFFSET+slot*FLASH_SECTOR_SIZE),MBR_STORAGE_RECORD_SIZE);
 return true;
 }
typedef struct {uint32_t offset;
 const uint8_t *data;
 } Mutation;
static void mutate(void *ctx){Mutation *m=ctx;
 flash_range_erase(m->offset,FLASH_SECTOR_SIZE);
 flash_range_program(m->offset,m->data,MBR_STORAGE_RECORD_SIZE);
 }
static bool write_slot(void *ctx,unsigned slot,const uint8_t *data){(void)ctx;
 if(slot>1||!safe())return false;
 Mutation m={PRODUCT_OFFSET+slot*FLASH_SECTOR_SIZE,data};
 return flash_safe_execute(mutate,&m,1000)==PICO_OK;
 }
static const MbrStorageIO io={NULL,read_slot,write_slot};
bool mbr_product_load(MbrProduct *p){return mbr_storage_load(&io,p);
 }
bool mbr_product_save(const MbrProduct *p){return mbr_storage_commit(&io,p);
 }
