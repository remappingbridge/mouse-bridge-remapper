#include "mbr/output_state/output_state.h"

#include <string.h>

void mbr_output_state_clear(mbr_output_state_t *state)
{
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
}
