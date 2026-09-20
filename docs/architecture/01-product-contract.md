# Product contract

Status: **FROZEN BY MBR-00; AMENDED BY MBR-02 UX DECISION 2026-09-20**.

## Purpose

Mouse Bridge Remapper is a standalone Raspberry Pi Pico 2 W appliance that pairs BLE HOGP mice, forwards one authoritative connected Mouse to a host over USB, and remaps mouse buttons locally through the Waveshare Pico-LCD-1.3 HAT.

The host requires no companion software for normal operation.

## Bluetooth product scope

The only Bluetooth product device type is **Mouse**, and the production Mouse transport is **BLE HOGP only**.

Excluded unless product documentation is changed first:

- Bluetooth Classic Mouse;
- Bluetooth Keyboard discovery/pairing/input;
- Bluetooth Composite Mouse+Keyboard as a product type;
- Keyboard/Composite saved-device registries or UI flows.

A HOGP peer is accepted only after HID capabilities are classified as compatible with the Mouse role.

## Saved mice and live capacity

Multiple Mouse records may be saved. Runtime product truth is:

```text
saved_mice = 0..N
live_authoritative_mouse = None | one MouseSession
```

A replacement candidate may temporarily have transport state while being qualified, but product-visible ready/authoritative count never exceeds one.

## Connected HOME

When one Mouse is authoritative, `home-connected` uses the connected Mouse's projected name as its dynamic title and exposes four visible options:

1. `PAIR NEW MOUSE` -> Pair New;
2. the confirmed remap summary -> `remapper-options`;
3. `SAVED DEVICES`;
4. `LEARN THE KEYS`.

The first option is the visible entry path for replacement Pair New while the current Mouse remains live and usable.

## HOME resolution

```text
no saved mice
  -> searching-first

saved mice + live Mouse
  -> home-connected

saved mice + no live Mouse
  -> home-searching + automatic 8-second saved search
```

The first saved Mouse reaching ready state wins. Search expiry or cancellation leads to `home-retry` / `DEVICE NOT FOUND`.

When the live Mouse disconnects, held output is released and connection truth is cleared. If HOME is visible, this resolver runs immediately; otherwise it runs when HOME is next accessed.

## First Mouse search

With no saved Mouse, search is logically indefinite through restartable 8-second discovery cycles. The first valid unsaved BLE HOGP Mouse accepted becomes saved and live; all other candidates from that transaction are ignored/canceled.

## Pair New

Pair New is a **15-second new-only replacement search**.

If a Mouse is already live, it remains the authoritative live Mouse and continues forwarding input during search. An already-saved candidate is not eligible for Pair New and is ignored for acceptance while the same search window continues.

When an unsaved candidate reaches replacement-ready state, perform an ordered handoff:

1. stop new input from the old live session;
2. release all held Mouse/Escape output belonging to it;
3. disconnect/clear the old live session while retaining its saved record and bond;
4. persist/confirm the new Mouse as required;
5. promote the new candidate as the sole authoritative ready Mouse;
6. stop Pair New.

If Pair New times out or is canceled before handoff, the original live Mouse remains connected and unchanged.

To reconnect a saved Mouse instead, the documented Help instructs the user to turn off/unplug the current Mouse and navigate Back until HOME enters `SEARCHING SAVED MOUSE`.

## Saved Devices

Each saved Mouse has one page.

- live Mouse: `STATUS: CONNECTED`, name cyan;
- every other saved Mouse: `STATUS: DISCONNECTED`, name ordinary body color.

At most one page is connected/cyan.

Names are stored in full normalized form within schema limits. Display uses the first 21 renderer-supported characters, with no ellipsis or scrolling. Empty/unusable name falls back to `UNKNOWN MOUSE`.

## Profiles

Canonical profile kinds and visible vocabulary:

- `PASSTHROUGH` / `PASSTHROUGH`;
- `STANDARD` / `STANDARD REMAP`;
- `ESCAPE` / `ESCAPE REMAP`;
- `CUSTOM` / `CUSTOM REMAP`.

Historical `DEFAULT REMAP` maps to `STANDARD` and is not canonical new UI text.

A newly saved Mouse begins in Passthrough. Movement, wheel and horizontal pan are never remapped by button profiles.

## Escape exception

Escape remains supported as output from Mouse remapping.

The fixed USB device therefore includes a minimal HID Keyboard output interface capable of standard Escape press/release. This is output-only infrastructure and does not authorize any Bluetooth Keyboard role.

## Custom template

One persistent global Custom template contains the five source-button mappings. Each saved Mouse stores only its confirmed profile kind. A saved Mouse using Custom uses the current global Custom template when connected.

Custom draft and dirty/unapplied state remain persistent separately from the last confirmed active profile.

## Persistence

Product state safely preserves at least:

- saved Mouse identity and full normalized name;
- confirmed per-Mouse profile kind;
- global Custom template;
- Custom draft/dirty state;
- required capability/vendor metadata.

Product state is integrity protected and power-loss safe through alternating generations or an equivalently proven mechanism. BTstack credentials are a separate ownership domain.

## Logitech HID++

HID++ behavior is automatic/capability-driven. Preserve true Forward down/hold/up when diversion is needed. Unsupported peers fall back safely to ordinary HOGP.

## Fixed USB identity

The MBR-00 USB identity is:

- VID: `0xCAFE`;
- PID: `0x4011`;
- bcdDevice: `0x0100`;
- manufacturer: `tiagooliveirajs`;
- product: `Mouse Bridge Remapper`;
- serial string: none (`iSerialNumber = 0`);
- HID interface 0: Mouse;
- HID interface 1: minimal Keyboard output for synthetic Escape;
- no CDC, MSC, MIDI or vendor-debug interface.

`0xCAFE` is retained as the project's development/local USB VID convention; this contract does not claim an allocated USB-IF vendor identity for commercial distribution.

USB identity exists from boot and never changes because of Bluetooth, search, profile, lock, removal or reconnect state. No forced Bluetooth-driven USB re-enumeration is allowed.

## Debug policy

Production behavior and acceptance must not depend on USB CDC, UART logging, debug-only USB identities, debug-only LCD pages or parallel debug firmware personalities.
