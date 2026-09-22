#pragma once
#include "mbr/mouse_registry/mouse_registry.h"
#define MBR_STORAGE_RECORD_SIZE 2048u
typedef struct { MbrRegistry registry; MbrTarget custom[5],draft[5]; bool dirty; MbrMouse pending_remove; } MbrProduct;
typedef struct {
 void *context;
 bool (*read)(void *,unsigned,uint8_t[MBR_STORAGE_RECORD_SIZE]);
 bool (*write)(void *,unsigned,const uint8_t[MBR_STORAGE_RECORD_SIZE]);
} MbrStorageIO;
void mbr_product_defaults(MbrProduct *p);
bool mbr_storage_encode(const MbrProduct *p,uint32_t generation,uint8_t out[MBR_STORAGE_RECORD_SIZE]);
bool mbr_storage_decode(const uint8_t data[MBR_STORAGE_RECORD_SIZE],MbrProduct *p,uint32_t *generation);
bool mbr_storage_load(const MbrStorageIO *io,MbrProduct *p);
bool mbr_storage_commit(const MbrStorageIO *io,const MbrProduct *p);
bool mbr_product_load(MbrProduct *p);
bool mbr_product_save(const MbrProduct *p);
