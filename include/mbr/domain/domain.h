#ifndef MBR_DOMAIN_DOMAIN_H
#define MBR_DOMAIN_DOMAIN_H

#include <stdint.h>

typedef struct {
    uint64_t value;
} mbr_mouse_id_t;

typedef struct {
    mbr_mouse_id_t mouse_id;
    uint32_t generation;
} mbr_mouse_session_id_t;

const char *mbr_version(void);

#endif
