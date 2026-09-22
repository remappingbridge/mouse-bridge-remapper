#pragma once
#include "mbr/interaction/interaction.h"
#include "mbr/ui_projector/ui_projector.h"
#include "mbr/mouse_session/mouse_session.h"
#include "mbr/mouse_registry/mouse_registry.h"
#include "mbr/pairing_coordinator/pairing_coordinator.h"
typedef enum { MBR_OP_NONE, MBR_OP_PROFILE, MBR_OP_DRAFT, MBR_OP_REMOVE, MBR_OP_HANDOFF } MbrOperation;
typedef struct {
 MbrOperation kind; uint32_t token; MouseId mouse; MouseSessionId session;
 MbrProfile profile; uint8_t source; MbrTarget target;
} MbrRequest;
typedef struct {
 MbrRegistry registry; MbrSessions sessions; MbrSearchTransaction search;
 MbrInteraction interaction; MbrScreen screen,help_owner;
 uint32_t epoch,request_counter,now; uint8_t selection,page,source;
 bool locked,custom_dirty; MbrTarget draft[5]; MbrRequest request;
 MbrMouse candidate;
} MbrApp;
void mbr_app_init(MbrApp *a,uint32_t now);
void mbr_app_home(MbrApp *a);
void mbr_app_event(MbrApp *a,MbrControl control,bool down);
void mbr_app_tick(MbrApp *a,uint32_t now);
void mbr_app_view(const MbrApp *a,MbrView *v);
bool mbr_app_ready(MbrApp *a,uint32_t transaction,const MbrMouse *m);
bool mbr_app_confirm(MbrApp *a,uint32_t token,bool success);
void mbr_app_disconnected(MbrApp *a,MouseSessionId generation);
/* Explicit fixture entry point is implemented only in qualification, not here. */
