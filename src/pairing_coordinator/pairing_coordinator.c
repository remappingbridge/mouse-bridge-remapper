#include "mbr/pairing_coordinator/pairing_coordinator.h"

void mbr_pairing_candidate_init(mbr_pairing_candidate_t *candidate)
{
    if (candidate == NULL) return;
    candidate->present = false;
    candidate->replacement_ready = false;
    candidate->transaction_id = 0u;
    candidate->mouse_id.value = 0u;
    candidate->session_id.mouse_id.value = 0u;
    candidate->session_id.generation = 0u;
}

void mbr_pairing_candidate_set_ready(mbr_pairing_candidate_t *candidate,
                                     mbr_mouse_session_id_t session_id)
{
    if (candidate == NULL) return;
    mbr_pairing_candidate_set_ready_for(candidate, 0u, session_id.mouse_id, session_id);
}

void mbr_pairing_candidate_set_ready_for(mbr_pairing_candidate_t *candidate,
                                         uint32_t transaction_id,
                                         mbr_mouse_id_t mouse_id,
                                         mbr_mouse_session_id_t session_id)
{
    if (candidate == NULL) return;
    candidate->present = true;
    candidate->replacement_ready = true;
    candidate->transaction_id = transaction_id;
    candidate->mouse_id = mouse_id;
    candidate->session_id = session_id;
}

void mbr_pairing_candidate_clear(mbr_pairing_candidate_t *candidate)
{
    mbr_pairing_candidate_init(candidate);
}

void mbr_pairing_coordinator_init(mbr_pairing_coordinator_t *coordinator)
{
    if (coordinator == NULL) return;
    coordinator->active = false;
    coordinator->next_transaction_id = 1u;
    coordinator->transaction_id = 0u;
    coordinator->duration_ms = 0u;
    coordinator->purpose = MBR_SEARCH_NONE;
    mbr_pairing_candidate_init(&coordinator->candidate);
}

uint32_t mbr_pairing_coordinator_start(mbr_pairing_coordinator_t *coordinator,
                                       mbr_search_purpose_t purpose,
                                       uint32_t duration_ms)
{
    if (coordinator == NULL || purpose == MBR_SEARCH_NONE) return 0u;
    uint32_t id = coordinator->next_transaction_id++;
    if (id == 0u) id = coordinator->next_transaction_id++;
    coordinator->active = true;
    coordinator->transaction_id = id;
    coordinator->duration_ms = duration_ms;
    coordinator->purpose = purpose;
    mbr_pairing_candidate_clear(&coordinator->candidate);
    return id;
}

bool mbr_pairing_coordinator_matches(const mbr_pairing_coordinator_t *coordinator,
                                     uint32_t transaction_id,
                                     mbr_search_purpose_t purpose)
{
    return coordinator != NULL && coordinator->active &&
           coordinator->transaction_id == transaction_id &&
           coordinator->purpose == purpose;
}

void mbr_pairing_coordinator_stop(mbr_pairing_coordinator_t *coordinator)
{
    if (coordinator == NULL) return;
    coordinator->active = false;
    coordinator->transaction_id = 0u;
    coordinator->duration_ms = 0u;
    coordinator->purpose = MBR_SEARCH_NONE;
    mbr_pairing_candidate_clear(&coordinator->candidate);
}
