#ifndef MBR_INTERACTION_INTERACTION_H
#define MBR_INTERACTION_INTERACTION_H

#include <stdbool.h>

#include "mbr/domain/domain.h"

typedef struct {
    bool locked;
    bool pressed;
    mbr_control_t pressed_control;
} mbr_interaction_t;

void mbr_interaction_init(mbr_interaction_t *interaction);
void mbr_interaction_set_locked(mbr_interaction_t *interaction, bool locked);
bool mbr_interaction_is_locked(const mbr_interaction_t *interaction);
bool mbr_interaction_press(mbr_interaction_t *interaction, mbr_control_t control);
bool mbr_interaction_release(mbr_interaction_t *interaction, mbr_control_t control);
bool mbr_interaction_is_pressed(const mbr_interaction_t *interaction,
                                mbr_control_t control);

#endif
