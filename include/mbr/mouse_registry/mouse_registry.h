#pragma once
#include "mbr/domain/domain.h"
typedef struct { MbrMouse mice[MBR_SAVED_CAPACITY]; size_t count; } MbrRegistry;
int mbr_registry_find(const MbrRegistry *r, MouseId id);
bool mbr_registry_put(MbrRegistry *r, const MbrMouse *m);
bool mbr_registry_remove(MbrRegistry *r, MouseId id);
