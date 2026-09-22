#pragma once
#include "mbr/domain/domain.h"
/* UI template order L/R/M/Forward/Back; transport order L/R/M/Back/Forward. */
bool mbr_profiles_mapping(MbrProfile profile,const MbrTarget custom[5],MbrTarget out[5]);
