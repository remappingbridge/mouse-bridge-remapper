# Remap, Escape and USB contract

Status: **FROZEN BY MBR-00; IMPLEMENTED BY MBR-04 CANDIDATE**.

## Pure remap core

Profiles/remap consume canonical Mouse events and do not depend on BTstack, TinyUSB, GPIO, SPI or raw BLE report structures.

Outputs are canonical Mouse target events or synthetic `EscapeDown` / `EscapeUp` intent.

## Exact mappings

### Passthrough
- Left→Left
- Right→Right
- Middle→Middle
- Forward→Forward
- Backward→Backward

### Standard
- Left→Forward
- Right→Backward
- Middle→Middle
- Forward→Left
- Backward→Right

### Escape
- Left→Escape
- Right→Backward
- Middle→Forward
- Forward→Left
- Backward→Right

Relative X/Y, wheel and pan remain unchanged.

### Custom
Sources: Left, Right, Middle, Forward, Backward.

Targets: Left, Right, Middle, Escape, Forward, Backward.

Draft edits are persistent separately from confirmed active profile state. Full Apply is authoritative only after runtime + persistence confirmation.

## Held ownership within one live Mouse

Even with one authoritative Mouse, held-state ownership is explicit because two physical source buttons can map to the same target.

A target stays down until every current-session source owning it has released. Disconnect, replacement handoff, removal, profile transition or continuity-invalidating failure clears the outgoing session's held ownership.

Synthetic Escape uses the same principle.

## Escape boundary

```text
Mouse canonical event
 -> remap
 -> Escape intent
 -> held Escape state
 -> usb_hid minimal Keyboard report
```

No Bluetooth Keyboard input path exists.

## Sole USB owner

`usb_hid` alone owns:

- TinyUSB descriptors/device service;
- Mouse report construction/submission;
- minimal Keyboard report construction/submission for synthetic Escape.

No other module calls TinyUSB device/report primitives directly.

## MBR-00 fixed USB identity

- VID `0xCAFE`
- PID `0x4011`
- bcdDevice `0x0100`
- manufacturer string `tiagooliveirajs`
- product string `Mouse Bridge Remapper`
- no serial string (`iSerialNumber = 0`)
- interface 0: HID Mouse
- interface 1: HID Keyboard, restricted by product logic to synthetic Escape output
- no CDC, MSC, MIDI or vendor-debug interface

`0xCAFE` is a project/local development VID convention and is not a claim of commercial USB-IF vendor allocation.

This USB device is technically a composite USB HID device because it exposes Mouse and Keyboard HID interfaces. The product's prohibition on “Composite” refers to Bluetooth Composite device pairing/domain support; it does not prohibit the fixed minimal Escape output interface required by the documented profiles.

## Fixed enumeration

Descriptor identity exists from boot. Bluetooth connection/search/pairing, profile changes, lock/unlock and removal/reconnect never trigger forced USB disconnect/reconnect or descriptor-shape changes.

## Backpressure

Persistent button/Escape state cannot be discarded because an endpoint is temporarily busy. Relative motion may be chunked/accumulated in bounded storage and consumed only when the USB submission contract accepts it.

## Logitech HID++

HID++ operates upstream of canonical remap output. Supported Forward diversion must preserve real down/hold/up. Passthrough removes unnecessary diversion. Unsupported peers fall back safely to Standard HOGP input.


## MBR-04 implementation boundary

The candidate implementation places every TinyUSB primitive inside `usb_hid`. The host-pure side exposes the fixed identity and pure builders from canonical `mbr_output_state_t`; the Pico side owns `tud_init`, `tud_task`, descriptor callbacks and HID report submission.

The fixed descriptor exposes exactly two HID interfaces:

- interface 0: Mouse, five buttons + X/Y/wheel/pan;
- interface 1: boot-style Keyboard, used by product logic only for synthetic Escape.

USB strings are fixed to `tiagooliveirajs` and `Mouse Bridge Remapper`; no serial string is emitted. TinyUSB CDC/MSC/MIDI/vendor classes are disabled. The product never calls forced USB disconnect/reconnect.

The MBR-04 qualification firmware is isolated from production and exercises report fixtures plus live Escape press/release from the HAT.
