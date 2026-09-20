#pragma once
#include "mbr/domain/domain.h"
typedef struct { MbrControl control; bool down; } MbrHatEvent;
typedef struct {
 bool raw[MBR_CONTROL_COUNT],stable[MBR_CONTROL_COUNT]; uint32_t changed[MBR_CONTROL_COUNT];
 MbrHatEvent queue[32]; uint8_t read,write; bool overflow;
} MbrHat;
extern const uint8_t mbr_hat_pins[MBR_CONTROL_COUNT];
void mbr_hat_sample(MbrHat *h,uint16_t pressed,uint32_t now);
bool mbr_hat_pop(MbrHat *h,MbrHatEvent *e);
void mbr_hat_init(void);
uint16_t mbr_hat_read(void);
