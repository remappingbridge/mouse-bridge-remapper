#ifndef MBR_PAIRING_COORDINATOR_PAIRING_COORDINATOR_H
#define MBR_PAIRING_COORDINATOR_PAIRING_COORDINATOR_H

#include <stdbool.h>

#include "mbr/domain/domain.h"

typedef struct {
    bool present;
    bool replacement_ready;
    mbr_mouse_session_id_t session_id;
} mbr_pairing_candidate_t;

void mbr_pairing_candidate_init(mbr_pairing_candidate_t *candidate);
void mbr_pairing_candidate_set_ready(mbr_pairing_candidate_t *candidate,
                                     mbr_mouse_session_id_t session_id);
void mbr_pairing_candidate_clear(mbr_pairing_candidate_t *candidate);

#endif
