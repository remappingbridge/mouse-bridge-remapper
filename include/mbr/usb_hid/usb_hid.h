#pragma once
#include "mbr/output_state/output_state.h"
#define MBR_USB_VID 0xcafeu
#define MBR_USB_PID 0x4011u
#define MBR_USB_BCD 0x0100u
#define MBR_USB_MANUFACTURER "tiagooliveirajs"
#define MBR_USB_PRODUCT "Mouse Bridge Remapper"
typedef struct { uint8_t bytes[5]; } MbrMouseReport;
typedef struct { uint8_t bytes[8]; } MbrEscapeReport;
typedef struct {
 mbr_output_state_t queue[32]; uint8_t read,write; bool mouse_done,escape_done;
 MbrMouseReport last_mouse; MbrEscapeReport last_escape;
} MbrUsbQueue;
typedef bool (*MbrUsbSend)(void *ctx,unsigned interface,const void *report,size_t size);
void mbr_usb_reports(const mbr_output_state_t *state,MbrMouseReport *mouse,MbrEscapeReport *escape);
bool mbr_usb_enqueue(MbrUsbQueue *q,const mbr_output_state_t *state);
void mbr_usb_drain(MbrUsbQueue *q,MbrUsbSend send,void *ctx);
void mbr_usb_release(MbrUsbQueue *q);
void mbr_usb_init(void);
void mbr_usb_task(void);
bool mbr_usb_submit(const mbr_output_state_t *state);
void mbr_usb_release_all(void);
const uint8_t *mbr_usb_device_descriptor(size_t *length);
const uint8_t *mbr_usb_configuration_descriptor(size_t *length);
const uint8_t *mbr_usb_report_descriptor(unsigned interface,size_t *length);
const char *mbr_usb_string(unsigned index);
