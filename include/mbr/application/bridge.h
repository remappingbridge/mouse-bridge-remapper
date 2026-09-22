#pragma once
#include "mbr/application/application.h"
#include "mbr/bt_runtime/bt_runtime.h"
#include "mbr/output_state/output_state.h"
#include "mbr/product_storage/product_storage.h"
typedef struct {
 MouseSessionId transport,pending,closing;uint32_t usb_epoch,pending_token,retry_at;
 MbrOutput output;MbrProduct product;MbrMouse incoming;
 bool initialized,synced,removing;
} MbrBridge;
void mbr_bridge_init(MbrBridge *b,MbrApp *app);
void mbr_bridge_task(MbrBridge *b,MbrApp *app);
void mbr_bridge_release(MbrBridge *b,MbrApp *app);
