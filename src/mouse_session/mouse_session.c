#include "mbr/mouse_session/mouse_session.h"
bool mbr_session_promote(MbrSessions *s, MouseId id) {
    if(s->live.ready || !id) return false;
    if(++s->next_generation==0) ++s->next_generation;
    s->live=(MbrSession){id,s->next_generation,true}; s->candidate=(MbrSession){0}; return true;
}
bool mbr_session_clear(MbrSessions *s, MouseSessionId generation) {
    if(!s->live.ready || s->live.generation!=generation) return false;
    s->live=(MbrSession){0}; return true;
}
