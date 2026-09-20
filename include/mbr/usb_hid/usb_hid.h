#ifndef MBR_USB_HID_USB_HID_H
#define MBR_USB_HID_USB_HID_H

#include <stdbool.h>
#include <stdint.h>

#include "mbr/output_state/output_state.h"

#define MBR_USB_HID_VID UINT16_C(0xcafe)
#define MBR_USB_HID_PID UINT16_C(0x4011)
#define MBR_USB_HID_BCD_DEVICE UINT16_C(0x0100)
#define MBR_USB_HID_INTERFACE_COUNT 2u
#define MBR_USB_HID_MOUSE_INTERFACE 0u
#define MBR_USB_HID_KEYBOARD_INTERFACE 1u
#define MBR_USB_HID_KEYBOARD_REPORT_SIZE 8u
#define MBR_USB_HID_MOUSE_REPORT_SIZE 5u
#define MBR_USB_HID_KEY_ESCAPE 0x29u
#define MBR_USB_HID_MANUFACTURER "tiagooliveirajs"
#define MBR_USB_HID_PRODUCT "Mouse Bridge Remapper"

typedef struct {
    uint16_t vid;
    uint16_t pid;
    uint16_t bcd_device;
    uint8_t interface_count;
    uint8_t mouse_interface;
    uint8_t keyboard_interface;
} mbr_usb_hid_identity_t;

typedef struct {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
    int8_t pan;
} mbr_usb_hid_mouse_report_t;

typedef struct {
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keycodes[6];
} mbr_usb_hid_keyboard_report_t;

const mbr_usb_hid_identity_t *mbr_usb_hid_identity(void);
void mbr_usb_hid_build_mouse_report(
    mbr_usb_hid_mouse_report_t *report,
    const mbr_output_state_t *state);
void mbr_usb_hid_build_keyboard_report(
    mbr_usb_hid_keyboard_report_t *report,
    const mbr_output_state_t *state);

bool mbr_usb_hid_pico_init(void);
void mbr_usb_hid_pico_task(void);
bool mbr_usb_hid_pico_mounted(void);
bool mbr_usb_hid_pico_send_mouse(const mbr_usb_hid_mouse_report_t *report);
bool mbr_usb_hid_pico_send_keyboard(const mbr_usb_hid_keyboard_report_t *report);

#endif
