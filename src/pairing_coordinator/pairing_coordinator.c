#include <stddef.h>

#include "mbr/pairing_coordinator/pairing_coordinator.h"

void mbr_pairing_candidate_init(mbr_pairing_candidate_t *candidate)
{
    if (candidate == NULL) return;
    candidate->present = false;
    candidate->replacement_ready = false;
    candidate->session_id.mouse_id.value = 0u;
    candidate->session_id.generation = 0u;
}

void mbr_pairing_candidate_set_ready(mbr_pairing_candidate_t *candidate,
                                     mbr_mouse_session_id_t session_id)
{
    if (candidate == NULL) return;
    candidate->present = true;
    candidate->replacement_ready = true;
    candidate->session_id = session_id;
}

void mbr_pairing_candidate_clear(mbr_pairing_candidate_t *candidate)
{
    mbr_pairing_candidate_init(candidate);
}
