#pragma once
#include "mbr/application/application.h"
#include "mbr/bt_runtime/bt_runtime.h"
#include "mbr/output_state/output_state.h"
typedef struct { MouseSessionId transport;uint32_t usb_epoch;MbrOutput output; } MbrBridge;
void mbr_bridge_task(MbrBridge *b,MbrApp *app);
void mbr_bridge_release(MbrBridge *b,MbrApp *app);
