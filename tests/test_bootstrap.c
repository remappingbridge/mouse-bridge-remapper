#include <assert.h>
#include <string.h>

#include "mbr/domain/domain.h"
#include "mbr/mouse_session/mouse_session.h"
#include "mbr/pairing_coordinator/pairing_coordinator.h"

int main(void)
{
    assert(strcmp(mbr_version(), "0.1.0-mbr01") == 0);

    mbr_authoritative_mouse_slot_t live;
    mbr_mouse_session_slot_init(&live);
    assert(mbr_mouse_session_ready_count(&live) == 0u);

    const mbr_mouse_session_id_t first = {{1u}, 1u};
    const mbr_mouse_session_id_t second = {{2u}, 1u};
    assert(mbr_mouse_session_slot_promote(&live, first));
    assert(mbr_mouse_session_ready_count(&live) == 1u);
    assert(!mbr_mouse_session_slot_promote(&live, second));
    assert(mbr_mouse_session_ready_count(&live) == 1u);

    mbr_pairing_candidate_t candidate;
    mbr_pairing_candidate_init(&candidate);
    mbr_pairing_candidate_set_ready(&candidate, second);
    assert(candidate.present);
    assert(candidate.replacement_ready);
    assert(mbr_mouse_session_ready_count(&live) == 1u);

    mbr_pairing_candidate_clear(&candidate);
    mbr_mouse_session_slot_clear(&live);
    assert(mbr_mouse_session_ready_count(&live) == 0u);
    return 0;
}
