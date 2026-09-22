#include "mbr/bt_runtime/bt_runtime.h"
bool mbr_bt_push(MbrBtQueue *q,const MbrBtMessage *m) {
 if(q->overflow)return false;
 uint8_t next=(uint8_t)((q->write+1)%MBR_BT_QUEUE_CAPACITY);
 if(next==q->read) {q->read=q->write=0;q->overflow=true;return false;}
 q->items[q->write]=*m;q->write=next;return true;
}
bool mbr_bt_pop(MbrBtQueue *q,MbrBtMessage *m) {
 if(q->overflow||q->read==q->write)return false;
 *m=q->items[q->read];q->read=(uint8_t)((q->read+1)%MBR_BT_QUEUE_CAPACITY);return true;
}
