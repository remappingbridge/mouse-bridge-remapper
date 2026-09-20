# MBR-04 — Fixed USB Mouse and synthetic Escape

Status: **IMPLEMENTED / PHYSICAL ACCEPTANCE PENDING** on the stacked branch `mbr/mbr-04-usb-hid`.

## Scope

MBR-04 establishes the fixed host-facing USB product before live BLE forwarding.

Implemented:

- `usb_hid` is the sole TinyUSB owner;
- fixed VID/PID/bcdDevice `0xCAFE:0x4011:0x0100`;
- manufacturer `tiagooliveirajs`;
- product `Mouse Bridge Remapper`;
- no serial string;
- interface 0: HID Mouse;
- interface 1: HID Keyboard used only for synthetic Escape;
- TinyUSB HID count fixed at two with CDC/MSC/MIDI/vendor disabled;
- five-button Mouse report with X/Y/wheel/pan;
- eight-byte boot-style Keyboard report with Escape as the only generated key;
- pure host report builders consume only `mbr_output_state_t`;
- Pico TinyUSB service/report submission isolated inside `usb_hid`;
- production firmware services USB without any forced re-enumeration;
- isolated USB/HID qualification firmware for physical report and Escape verification;
- production and qualification stdio/CDC remain disabled.

## Report contract

Mouse report:

```text
byte 0: Left, Right, Middle, Forward, Backward button bits
byte 1: X
byte 2: Y
byte 3: wheel
byte 4: horizontal pan
```

All relative fields are signed and clamped to the HID logical range `-127..127` by the pure report builder.

Keyboard report:

```text
byte 0: modifiers = 0
byte 1: reserved = 0
byte 2: Escape (0x29) while held, otherwise 0
bytes 3..7: zero
```

No Bluetooth Keyboard input is introduced by this interface.

## Qualification firmware

`mbr_usb_hid_qualification.uf2` is isolated from the production executable.

HAT controls:

- JOY UP: Mouse Left button;
- JOY DOWN: Mouse Right button;
- JOY PRESS: Mouse Middle button;
- KEY X: Mouse Forward button;
- KEY Y: Mouse Backward button;
- KEY A: synthetic Escape press/release.

The qualification firmware also emits deterministic X/Y/wheel/pan fixture reports once per second while preserving any held button state. It never enables USB stdio, UART stdio or diagnostic CDC.

## Physical acceptance boundary

CI/build success is not physical acceptance. The operator must verify:

1. exact VID/PID/bcdDevice;
2. exact manufacturer/product strings and no serial;
3. exactly two HID interfaces, Mouse then Keyboard;
4. no CDC/MSC/MIDI/vendor-debug interface;
5. stable enumeration without Bluetooth or UI-triggered re-enumeration;
6. five Mouse buttons;
7. X/Y movement;
8. wheel;
9. horizontal pan;
10. Escape down while KEY A is held and Escape release when KEY A is released;
11. no unexpected keyboard keys are generated;
12. production firmware remains free of diagnostic USB/UART stdio.

The MBR-04 candidate is not accepted or merged while the predecessor MBR-03 physical gate remains open.
