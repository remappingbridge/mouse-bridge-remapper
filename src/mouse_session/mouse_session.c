#include "mbr/mouse_session/mouse_session.h"

void mbr_mouse_session_slot_init(mbr_authoritative_mouse_slot_t *slot)
{
    if (slot == NULL) return;
    slot->occupied = false;
    slot->ready = false;
    slot->session_id.mouse_id.value = 0u;
    slot->session_id.generation = 0u;
}

bool mbr_mouse_session_slot_promote(mbr_authoritative_mouse_slot_t *slot,
                                    mbr_mouse_session_id_t session_id)
{
    if (slot == NULL || slot->occupied) return false;
    slot->occupied = true;
    slot->ready = true;
    slot->session_id = session_id;
    return true;
}

void mbr_mouse_session_slot_clear(mbr_authoritative_mouse_slot_t *slot)
{
    mbr_mouse_session_slot_init(slot);
}

unsigned mbr_mouse_session_ready_count(const mbr_authoritative_mouse_slot_t *slot)
{
    return slot != NULL && slot->occupied && slot->ready ? 1u : 0u;
}

bool mbr_mouse_session_matches(const mbr_authoritative_mouse_slot_t *slot,
                               mbr_mouse_session_id_t session_id)
{
    return slot != NULL && slot->occupied && slot->ready &&
           mbr_mouse_session_id_equal(slot->session_id, session_id);
}
