#include "mbr/mouse_registry/mouse_registry.h"

#include <stdint.h>

#define MBR_INDEX_NONE ((size_t)SIZE_MAX)

void mbr_mouse_registry_init(mbr_mouse_registry_t *registry,
                             mbr_saved_mouse_t *storage,
                             size_t capacity)
{
    if (registry == NULL) return;
    registry->records = storage;
    registry->capacity = capacity;
    registry->count = 0u;
}

size_t mbr_mouse_registry_count(const mbr_mouse_registry_t *registry)
{
    return registry == NULL ? 0u : registry->count;
}

const mbr_saved_mouse_t *mbr_mouse_registry_get(const mbr_mouse_registry_t *registry,
                                                size_t index)
{
    if (registry == NULL || index >= registry->count) return NULL;
    return &registry->records[index];
}

mbr_saved_mouse_t *mbr_mouse_registry_get_mut(mbr_mouse_registry_t *registry,
                                              size_t index)
{
    if (registry == NULL || index >= registry->count) return NULL;
    return &registry->records[index];
}

size_t mbr_mouse_registry_index_of(const mbr_mouse_registry_t *registry,
                                   mbr_mouse_id_t mouse_id)
{
    if (registry == NULL) return MBR_INDEX_NONE;
    for (size_t index = 0u; index < registry->count; ++index) {
        if (mbr_mouse_id_equal(registry->records[index].mouse_id, mouse_id))
            return index;
    }
    return MBR_INDEX_NONE;
}

const mbr_saved_mouse_t *mbr_mouse_registry_find(const mbr_mouse_registry_t *registry,
                                                 mbr_mouse_id_t mouse_id)
{
    const size_t index = mbr_mouse_registry_index_of(registry, mouse_id);
    return index == MBR_INDEX_NONE ? NULL : &registry->records[index];
}

bool mbr_mouse_registry_add(mbr_mouse_registry_t *registry,
                            mbr_saved_mouse_t record)
{
    if (registry == NULL || registry->records == NULL) return false;
    const size_t existing = mbr_mouse_registry_index_of(registry, record.mouse_id);
    if (existing != MBR_INDEX_NONE) {
        registry->records[existing] = record;
        return true;
    }
    if (registry->count >= registry->capacity) return false;
    registry->records[registry->count++] = record;
    return true;
}

bool mbr_mouse_registry_remove(mbr_mouse_registry_t *registry,
                               mbr_mouse_id_t mouse_id)
{
    if (registry == NULL) return false;
    const size_t index = mbr_mouse_registry_index_of(registry, mouse_id);
    if (index == MBR_INDEX_NONE) return false;
    for (size_t cursor = index + 1u; cursor < registry->count; ++cursor)
        registry->records[cursor - 1u] = registry->records[cursor];
    --registry->count;
    return true;
}

bool mbr_mouse_registry_set_profile(mbr_mouse_registry_t *registry,
                                    mbr_mouse_id_t mouse_id,
                                    mbr_profile_kind_t profile)
{
    const size_t index = mbr_mouse_registry_index_of(registry, mouse_id);
    if (index == MBR_INDEX_NONE || (unsigned)profile >= MBR_PROFILE_COUNT) return false;
    registry->records[index].profile = profile;
    return true;
}
