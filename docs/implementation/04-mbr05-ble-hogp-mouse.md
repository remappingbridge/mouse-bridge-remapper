# MBR-05 — Canonical BLE HOGP Mouse

Status: **IMPLEMENTED / PHYSICAL ACCEPTANCE PENDING**.

## Scope

MBR-05 implements the accepted G05-class BLE Mouse transport under the frozen
Mouse Bridge Remapper architecture:

- one CYW43/BTstack runtime owner;
- BLE HOGP Mouse only;
- security/bonding with NoInputNoOutput + secure connection + bonding;
- HIDS Report Protocol connection;
- Report Map classification before Mouse acceptance;
- rejection of keyboard-only candidates;
- canonical five-button Mouse, relative X/Y, wheel and horizontal pan events;
- duplicated Report-ID framing normalization;
- malformed/truncated report rejection;
- bounded runtime queue between BTstack callbacks and application state;
- session generation IDs and stale-event filtering;
- bonded reconnect using the BTstack LE device database/resolving/whitelist path;
- 8-second bonded connection-attempt bound followed by generic scanning;
- disconnect and queue-overflow output cleanup;
- fixed USB Mouse forwarding through the MBR-04 report builder.

## Canonical boundary

BLE report layouts terminate at:

```text
BLE HOGP Report Map
        |
        v
mbr_mouse_event_t
        |
        v
mbr_output_state_t
        |
        v
mbr_usb_hid_mouse_report_t
```

No BTstack or BLE descriptor types cross into the UI, renderer or USB report
builder.

## Report Map rules

The host-pure parser recognizes:

- Generic Desktop / Mouse application collections;
- Button usages 1..5;
- relative X;
- relative Y;
- relative Wheel;
- Consumer AC Pan.

HID reports can use Report IDs. If BTstack supplies a duplicated leading Report
ID, the adapter strips it only when the remaining length exactly matches the
Report Map expectation. Any other framing mismatch is rejected.

A descriptor that exposes only Keyboard input is rejected. Advertisement
appearance is an early filter, not final qualification.

## Session safety

Every accepted READY connection gets:

- a deterministic Mouse identity derived from BLE address/type;
- a monotonically increasing session generation.

Runtime messages carry the complete session identity. The application only
applies Mouse events whose session identity matches the current authoritative
slot.

On disconnect or runtime queue overflow:

1. held Mouse buttons are released;
2. pending relative X/Y/wheel/pan are cleared;
3. old session events become ineligible;
4. USB report servicing continues.

## Reconnect

The Pico adapter first attempts a bonded connection using the BTstack LE device
database, resolving list and whitelist. The attempt is bounded to 8 seconds.
Failure falls back to generic BLE HOGP scanning.

This is transport-level reconnect only. Full persistent saved-device policy,
SEARCH_SAVED transaction semantics and Pair New replacement handoff remain
later gates.

## Deferred scope

MBR-05 does not implement:

- persistent product state;
- multiple saved-device registry semantics;
- Pair New candidate/handoff;
- G06 Standard/Escape/Custom remap behavior;
- Logitech HID++;
- full real UX transaction/effect execution.

Those behaviors remain explicitly owned by later gates and are not silently
pulled into MBR-05.
