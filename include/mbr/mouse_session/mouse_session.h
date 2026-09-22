#pragma once
#include "mbr/domain/domain.h"
typedef struct { MbrSession live, candidate; MouseSessionId next_generation; } MbrSessions;
bool mbr_session_promote(MbrSessions *s, MouseId id);
bool mbr_session_clear(MbrSessions *s, MouseSessionId generation);
