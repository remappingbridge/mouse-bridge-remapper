#ifndef MBR_MOUSE_REGISTRY_MOUSE_REGISTRY_H
#define MBR_MOUSE_REGISTRY_MOUSE_REGISTRY_H

#include <stddef.h>

#include "mbr/domain/domain.h"

typedef struct {
    mbr_mouse_id_t mouse_id;
    const char *name;
    mbr_profile_kind_t profile;
} mbr_saved_mouse_t;

typedef struct {
    mbr_saved_mouse_t *records;
    size_t capacity;
    size_t count;
} mbr_mouse_registry_t;

void mbr_mouse_registry_init(mbr_mouse_registry_t *registry,
                             mbr_saved_mouse_t *storage,
                             size_t capacity);
size_t mbr_mouse_registry_count(const mbr_mouse_registry_t *registry);
const mbr_saved_mouse_t *mbr_mouse_registry_get(const mbr_mouse_registry_t *registry,
                                                size_t index);
mbr_saved_mouse_t *mbr_mouse_registry_get_mut(mbr_mouse_registry_t *registry,
                                              size_t index);
size_t mbr_mouse_registry_index_of(const mbr_mouse_registry_t *registry,
                                   mbr_mouse_id_t mouse_id);
const mbr_saved_mouse_t *mbr_mouse_registry_find(const mbr_mouse_registry_t *registry,
                                                 mbr_mouse_id_t mouse_id);
bool mbr_mouse_registry_add(mbr_mouse_registry_t *registry,
                            mbr_saved_mouse_t record);
bool mbr_mouse_registry_remove(mbr_mouse_registry_t *registry,
                               mbr_mouse_id_t mouse_id);
bool mbr_mouse_registry_set_profile(mbr_mouse_registry_t *registry,
                                    mbr_mouse_id_t mouse_id,
                                    mbr_profile_kind_t profile);

#endif
