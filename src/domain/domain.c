#include "mbr/domain/domain.h"

#ifndef MBR_VERSION_STRING
#define MBR_VERSION_STRING "0.1.0-mbr01"
#endif

const char *mbr_version(void)
{
    return MBR_VERSION_STRING;
}
