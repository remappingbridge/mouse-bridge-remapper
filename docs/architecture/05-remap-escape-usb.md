# Remap, Escape and USB contract

## Pure remap core

Profiles/remap operate on canonical events and have no dependency on BTstack, TinyUSB, GPIO, SPI or raw BLE report structures.

Input:

- current `MouseId` / `MouseSessionId`;
- confirmed profile kind;
- canonical physical Mouse event;
- global Custom template where applicable.

Output:

- canonical Mouse button/motion target events; or
- synthetic `EscapeDown` / `EscapeUp` intent.

Only the single current live mouse may feed this pipeline.

## Exact preset mappings

### Passthrough

- Left→Left
- Right→Right
- Middle→Middle
- Forward→Forward
- Backward→Backward

### Default / Standard

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

Relative X/Y, wheel and pan are unchanged by all profiles.

## Custom

Sources: Left, Right, Middle, Forward, Backward.

Allowed targets: Left, Right, Middle, Escape, Forward, Backward.

The draft is distinct from the confirmed active template/profile. A per-source Apply-and-Back updates the draft immediately. Full Apply becomes authoritative only after runtime + persistence confirmation.

## Escape exception boundary

The project is Mouse-only for Bluetooth input, but Escape remains a standard keyboard key on USB output.

The architecture therefore permits a narrowly scoped synthetic-keyboard path:

```text
Mouse canonical event
 -> remap
 -> Escape intent
 -> held Escape state
 -> usb_hid Keyboard Escape report
```

There is no reverse or peer input path from a Bluetooth Keyboard.

The synthetic-keyboard component must expose only the capabilities necessary for the documented remap output contract unless future documentation expands it.

## USB owner

`usb_hid` is the only module allowed to own:

- TinyUSB device descriptors;
- `tud_task`/device service integration;
- Mouse report construction/submission;
- Keyboard report construction/submission for synthetic Escape.

Bluetooth adapters, remap logic and UI may not call TinyUSB directly.

## Fixed identity

The USB descriptor is fixed from boot. Bluetooth connect/disconnect, searching, profile changes, lock/unlock, saved-device operations and reconnect do not force `tud_disconnect()` / `tud_connect()` re-enumeration.

No diagnostic CDC, MSC, MIDI or vendor-debug interface is added to the production product merely for troubleshooting.

Exact project VID/PID/manufacturer/product strings remain an open product decision. Until frozen, historical BLU2USB strings are evidence only and must not be copied as the final Mouse Bridge Remapper identity by accident.

## Held-state safety

Mouse button and synthetic Escape output are explicit held states for the current session.

Two physical inputs from the same mouse may map to the same output target. The implementation must therefore avoid premature release when one physical source releases while another still maps to the same held target.

This ownership/refcounting is **within the current mouse session only**; there is no cross-mouse aggregation because concurrent mouse connections are forbidden.

Before the current session is disconnected, replaced, removed or invalidated by parser/queue failure, all held output attributable to that session is released safely.

A profile change also clears stale held state from the old mapping before the new mapping becomes authoritative.

## USB backpressure

Persistent button/key state may not be discarded because an endpoint is temporarily busy. Relative motion can be chunked/accumulated within bounded storage, but state transitions require release-safe handling.

## Logitech HID++

HID++ is upstream of canonical remap output. When Forward is remapped and a supported peer requires diversion to recover true held semantics, the vendor adapter emits correct canonical Forward transitions for the current session.

Passthrough removes unneeded diversion. Unsupported peers fall back to Standard HID without breaking ordinary mouse input.
