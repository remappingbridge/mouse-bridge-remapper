#ifndef MBR_MOUSE_SESSION_MOUSE_SESSION_H
#define MBR_MOUSE_SESSION_MOUSE_SESSION_H

#include <stdbool.h>

#include "mbr/domain/domain.h"

typedef struct {
    bool occupied;
    bool ready;
    mbr_mouse_session_id_t session_id;
} mbr_authoritative_mouse_slot_t;

void mbr_mouse_session_slot_init(mbr_authoritative_mouse_slot_t *slot);
bool mbr_mouse_session_slot_promote(mbr_authoritative_mouse_slot_t *slot,
                                    mbr_mouse_session_id_t session_id);
void mbr_mouse_session_slot_clear(mbr_authoritative_mouse_slot_t *slot);
unsigned mbr_mouse_session_ready_count(const mbr_authoritative_mouse_slot_t *slot);

#endif
