#include "mbr/interaction/interaction.h"

void mbr_interaction_init(mbr_interaction_t *interaction)
{
    if (interaction == NULL) return;
    interaction->locked = false;
    interaction->pressed = false;
    interaction->pressed_control = MBR_CONTROL_JOY_UP;
}

void mbr_interaction_set_locked(mbr_interaction_t *interaction, bool locked)
{
    if (interaction == NULL) return;
    interaction->locked = locked;
    interaction->pressed = false;
}

bool mbr_interaction_is_locked(const mbr_interaction_t *interaction)
{
    return interaction != NULL && interaction->locked;
}

bool mbr_interaction_press(mbr_interaction_t *interaction, mbr_control_t control)
{
    if (interaction == NULL || (unsigned)control >= MBR_CONTROL_COUNT) return false;
    interaction->pressed = true;
    interaction->pressed_control = control;
    return true;
}

bool mbr_interaction_release(mbr_interaction_t *interaction, mbr_control_t control)
{
    if (interaction == NULL || !interaction->pressed ||
        interaction->pressed_control != control) return false;
    interaction->pressed = false;
    return true;
}

bool mbr_interaction_is_pressed(const mbr_interaction_t *interaction,
                                mbr_control_t control)
{
    return interaction != NULL && interaction->pressed &&
           interaction->pressed_control == control;
}
