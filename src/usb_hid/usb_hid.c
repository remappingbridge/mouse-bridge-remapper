#include "mbr/usb_hid/usb_hid.h"

#include <stddef.h>

_Static_assert(sizeof(mbr_usb_hid_mouse_report_t) == MBR_USB_HID_MOUSE_REPORT_SIZE,
               "MBR mouse report must remain five bytes");
_Static_assert(sizeof(mbr_usb_hid_keyboard_report_t) == MBR_USB_HID_KEYBOARD_REPORT_SIZE,
               "MBR keyboard report must remain eight bytes");

static const mbr_usb_hid_identity_t k_identity = {
    .vid = MBR_USB_HID_VID,
    .pid = MBR_USB_HID_PID,
    .bcd_device = MBR_USB_HID_BCD_DEVICE,
    .interface_count = MBR_USB_HID_INTERFACE_COUNT,
    .mouse_interface = MBR_USB_HID_MOUSE_INTERFACE,
    .keyboard_interface = MBR_USB_HID_KEYBOARD_INTERFACE,
};

static int8_t clamp_axis(int16_t value)
{
    if (value > 127) return 127;
    if (value < -127) return -127;
    return (int8_t)value;
}

const mbr_usb_hid_identity_t *mbr_usb_hid_identity(void)
{
    return &k_identity;
}

void mbr_usb_hid_build_mouse_report(
    mbr_usb_hid_mouse_report_t *report,
    const mbr_output_state_t *state)
{
    if (report == NULL) return;
    if (state == NULL) {
        report->buttons = 0u;
        report->x = 0;
        report->y = 0;
        report->wheel = 0;
        report->pan = 0;
        return;
    }
    report->buttons = (uint8_t)(state->buttons & 0x1fu);
    report->x = clamp_axis(state->x);
    report->y = clamp_axis(state->y);
    report->wheel = clamp_axis(state->wheel);
    report->pan = clamp_axis(state->pan);
}

void mbr_usb_hid_build_keyboard_report(
    mbr_usb_hid_keyboard_report_t *report,
    const mbr_output_state_t *state)
{
    if (report == NULL) return;
    report->modifiers = 0u;
    report->reserved = 0u;
    for (size_t index = 0u; index < 6u; ++index) report->keycodes[index] = 0u;
    if (state != NULL && state->escape_down) report->keycodes[0] = MBR_USB_HID_KEY_ESCAPE;
}
