#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "mbr/output_state/output_state.h"
#include "mbr/usb_hid/usb_hid.h"

static void test_identity(void)
{
    const mbr_usb_hid_identity_t *identity = mbr_usb_hid_identity();
    assert(identity != NULL);
    assert(identity->vid == 0xcafeu);
    assert(identity->pid == 0x4011u);
    assert(identity->bcd_device == 0x0100u);
    assert(identity->interface_count == 2u);
    assert(identity->mouse_interface == 0u);
    assert(identity->keyboard_interface == 1u);
    assert(strcmp(MBR_USB_HID_MANUFACTURER, "tiagooliveirajs") == 0);
    assert(strcmp(MBR_USB_HID_PRODUCT, "Mouse Bridge Remapper") == 0);
    assert(MBR_USB_HID_KEY_ESCAPE == 0x29u);
}

static void test_mouse_report_from_canonical_state(void)
{
    mbr_output_state_t state;
    mbr_usb_hid_mouse_report_t report;

    mbr_output_state_clear(&state);
    state.buttons = MBR_OUTPUT_BUTTON_LEFT |
                    MBR_OUTPUT_BUTTON_MIDDLE |
                    MBR_OUTPUT_BUTTON_FORWARD;
    state.x = -12;
    state.y = 23;
    state.wheel = -4;
    state.pan = 7;
    mbr_usb_hid_build_mouse_report(&report, &state);

    assert(report.buttons == 0x0du);
    assert(report.x == -12);
    assert(report.y == 23);
    assert(report.wheel == -4);
    assert(report.pan == 7);

    state.x = 900;
    state.y = -900;
    state.wheel = 300;
    state.pan = -300;
    mbr_usb_hid_build_mouse_report(&report, &state);
    assert(report.x == 127);
    assert(report.y == -127);
    assert(report.wheel == 127);
    assert(report.pan == -127);
}

static void test_escape_report(void)
{
    mbr_output_state_t state;
    mbr_usb_hid_keyboard_report_t report;

    mbr_output_state_clear(&state);
    mbr_usb_hid_build_keyboard_report(&report, &state);
    for (size_t i = 0u; i < sizeof(report.keycodes); ++i) assert(report.keycodes[i] == 0u);

    state.escape_down = true;
    mbr_usb_hid_build_keyboard_report(&report, &state);
    assert(report.modifiers == 0u);
    assert(report.reserved == 0u);
    assert(report.keycodes[0] == MBR_USB_HID_KEY_ESCAPE);
    for (size_t i = 1u; i < sizeof(report.keycodes); ++i) assert(report.keycodes[i] == 0u);

    state.escape_down = false;
    mbr_usb_hid_build_keyboard_report(&report, &state);
    for (size_t i = 0u; i < sizeof(report.keycodes); ++i) assert(report.keycodes[i] == 0u);
}

int main(void)
{
    test_identity();
    test_mouse_report_from_canonical_state();
    test_escape_report();
    puts("MBR-04 USB HID host contract: PASS");
    return 0;
}
