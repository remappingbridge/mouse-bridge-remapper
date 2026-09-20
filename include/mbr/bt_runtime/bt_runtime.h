#pragma once
#include "mbr/ble_hogp/ble_hogp.h"
#define MBR_BT_QUEUE_CAPACITY 64u
typedef struct { MbrBtMessage items[MBR_BT_QUEUE_CAPACITY];uint8_t read,write;bool overflow; } MbrBtQueue;
/* Pure queue: both producer and consumer must hold the same owner lock. */
bool mbr_bt_push(MbrBtQueue *q,const MbrBtMessage *m);
bool mbr_bt_pop(MbrBtQueue *q,MbrBtMessage *m);
bool mbr_bt_start(void);
void mbr_bt_search(MbrSearch purpose,uint32_t transaction);
bool mbr_bt_next(MbrBtMessage *message,bool *overflow);
void mbr_bt_accept(MouseSessionId session);
void mbr_bt_disconnect(MouseSessionId session);
/* Callback-side only, already inside the async owner. */
bool mbr_bt_publish(const MbrBtMessage *message);
