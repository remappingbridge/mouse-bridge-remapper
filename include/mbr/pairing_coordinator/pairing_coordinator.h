#ifndef MBR_PAIRING_COORDINATOR_PAIRING_COORDINATOR_H
#define MBR_PAIRING_COORDINATOR_PAIRING_COORDINATOR_H

#include <stdbool.h>
#include <stdint.h>

#include "mbr/domain/domain.h"

typedef struct {
    bool present;
    bool replacement_ready;
    uint32_t transaction_id;
    mbr_mouse_id_t mouse_id;
    mbr_mouse_session_id_t session_id;
} mbr_pairing_candidate_t;

typedef struct {
    bool active;
    uint32_t next_transaction_id;
    uint32_t transaction_id;
    uint32_t duration_ms;
    mbr_search_purpose_t purpose;
    mbr_pairing_candidate_t candidate;
} mbr_pairing_coordinator_t;

void mbr_pairing_candidate_init(mbr_pairing_candidate_t *candidate);
void mbr_pairing_candidate_set_ready(mbr_pairing_candidate_t *candidate,
                                     mbr_mouse_session_id_t session_id);
void mbr_pairing_candidate_set_ready_for(mbr_pairing_candidate_t *candidate,
                                         uint32_t transaction_id,
                                         mbr_mouse_id_t mouse_id,
                                         mbr_mouse_session_id_t session_id);
void mbr_pairing_candidate_clear(mbr_pairing_candidate_t *candidate);

void mbr_pairing_coordinator_init(mbr_pairing_coordinator_t *coordinator);
uint32_t mbr_pairing_coordinator_start(mbr_pairing_coordinator_t *coordinator,
                                       mbr_search_purpose_t purpose,
                                       uint32_t duration_ms);
bool mbr_pairing_coordinator_matches(const mbr_pairing_coordinator_t *coordinator,
                                     uint32_t transaction_id,
                                     mbr_search_purpose_t purpose);
void mbr_pairing_coordinator_stop(mbr_pairing_coordinator_t *coordinator);

#endif
