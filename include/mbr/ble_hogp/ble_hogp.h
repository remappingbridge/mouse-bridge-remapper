#pragma once
#include "mbr/domain/domain.h"
#define MBR_HOGP_MAX_FIELDS 48u
#define MBR_HOGP_MAX_REPORTS 16u
typedef enum { MBR_HOGP_FIELD_BUTTON, MBR_HOGP_FIELD_X, MBR_HOGP_FIELD_Y, MBR_HOGP_FIELD_WHEEL, MBR_HOGP_FIELD_PAN } MbrHogpFieldKind;
typedef struct { uint8_t report_id; MbrHogpFieldKind kind; uint16_t bit_offset; uint8_t bit_size,button_index; bool signed_value; } MbrHogpField;
typedef struct { uint8_t report_id; uint16_t input_bits; uint8_t button_mask; } MbrHogpReport;
typedef struct {
 MouseSessionId session; MbrHogpField fields[MBR_HOGP_MAX_FIELDS];size_t field_count;
 MbrHogpReport reports[MBR_HOGP_MAX_REPORTS];size_t report_count;
 uint8_t aggregate_buttons;bool configured;
} MbrHogpParser;
typedef bool (*MbrHogpEmit)(void *context,const MbrMouseEvent *event);
bool mbr_hogp_configure(MbrHogpParser *p,MouseSessionId session,const uint8_t *map,size_t length);
bool mbr_hogp_has_mouse(const MbrHogpParser *p);
bool mbr_hogp_normalize(const MbrHogpParser *p,uint8_t id,const uint8_t *report,size_t length,const uint8_t **payload,size_t *payload_length);
bool mbr_hogp_parse(MbrHogpParser *p,uint8_t id,const uint8_t *report,size_t length,MbrHogpEmit emit,void *context);
/* Radio adapter calls below require the runtime async-context lock. */
typedef enum { MBR_BT_READY, MBR_BT_INPUT, MBR_BT_DISCONNECTED } MbrBtKind;
typedef struct {
 MbrBtKind kind;MouseSessionId session;uint32_t transaction;
 MbrMouse mouse;MbrMouseEvent input;
} MbrBtMessage;
typedef bool (*MbrHogpPublish)(const MbrBtMessage *message);
void mbr_hogp_setup(MbrHogpPublish publish);
void mbr_hogp_accept(MouseSessionId session);
void mbr_hogp_search(MbrSearch purpose,uint32_t transaction);
void mbr_hogp_disconnect(MouseSessionId session);
void mbr_hogp_registry(const MbrMouse *mice,size_t count);
bool mbr_hogp_forget(const MbrMouse *mouse);
void mbr_hogp_profile(MouseSessionId session,bool remapped);
void mbr_hogp_tick(uint32_t now);
