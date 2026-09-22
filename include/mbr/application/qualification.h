#pragma once
#include "mbr/application/application.h"
#include "mbr/usb_hid/usb_hid.h"
typedef struct { unsigned mode,selection,gallery,usb_mode; MbrInteraction interaction; uint32_t epoch,next_motion,confirm_at,first_at; } MbrQualification;
void mbr_qualification_init(MbrQualification *q);
void mbr_qualification_event(MbrQualification *q,MbrApp *a,MbrControl c,bool down);
void mbr_qualification_tick(MbrQualification *q,MbrApp *a,uint32_t now);
void mbr_qualification_frame(const MbrQualification *q,const MbrApp *a,MbrFrame *f);
