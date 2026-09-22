# System architecture

Status: **FROZEN BY MBR-00; AMENDED BY MBR-02 UX DECISION 2026-09-20**.

## Design goals

The architecture makes these properties structural:

- many saved mice, zero or one authoritative live Mouse;
- HOME has one deterministic resolver;
- Pair New searches while the current Mouse remains live, then performs one controlled replacement handoff;
- saved-device search starts automatically when HOME is entered with saved mice and no live Mouse;
- saved search is bounded to 8 seconds; Pair New is bounded to 15 seconds;
- Bluetooth details do not leak into remap/UI/USB;
- no Bluetooth Keyboard/Composite product role;
- synthetic Escape is a narrow USB output exception;
- persistence and BT credentials have separate ownership;
- UI does not operate BTstack/TinyUSB/flash directly.

## Component model

```text
HAT GPIO
   |
   v
+-------------------+       +------------------+
| interaction       |------>| application      |
| release semantics |       | orchestration    |
+-------------------+       +--+-----+-----+----+
                                |     |     |
                     +----------+     |     +----------------+
                     v                v                      v
             +---------------+ +-------------+       +--------------+
             | ui_projector  | | profiles /  |       | pairing /    |
             | screen state  | | remap       |       | reconnect    |
             +-------+-------+ +------+------+       +------+-------+
                     |                |                     |
                     v                v                     v
             +---------------+ +-------------+       +--------------+
             | renderer      | | output      |<------| mouse_session|
             | ST7789        | | state       |       | 0 or 1 live  |
             +---------------+ +------+------+       +------+-------+
                                      |                     |
                                      v                     v
                               +-------------+       +--------------+
                               | usb_hid     |       | ble_hogp     |
                               | Mouse +     |       | live/candidate|
                               | Escape sink |       +------+-------+
                               +-------------+              |
                                                            v
                                                     +--------------+
                                                     | bt_runtime   |
                                                     | one owner    |
                                                     +--------------+

application ---> mouse_registry ---> product_storage
ble_hogp -----> logitech_hidpp for live/replacement candidate as required
bt_runtime ---> BTstack credential store
```

## Module responsibilities

### `domain`
Pure MouseId/MouseSessionId, canonical button/motion events, profile identifiers, commands/results. No Pico SDK/BTstack/TinyUSB headers.

### `mouse_session`
Owns the optional authoritative live session plus generation identity. It may track one non-authoritative replacement candidate during Pair New if transport implementation requires it, but `authoritative_ready_count <= 1` always.

### `ble_hogp`
Owns HOGP discovery/client/report-map parsing and transport normalization. A candidate cannot be promoted until it is classified as a compatible Mouse.

### `bt_runtime`
Sole CYW43/BTstack lifecycle/run-loop owner.

### `pairing_coordinator`
Owns FIRST_MOUSE, SEARCH_SAVED and PAIR_NEW transaction purpose, timeout, cancellation, one-winner acceptance and replacement handoff sequencing.

### `mouse_registry`
Owns multiple saved identities/names/profile kinds/vendor metadata. Saved is not connected.

### `profiles` / `remap`
Host-pure profile tables, global Custom template/draft, and canonical mapping to Mouse outputs or synthetic Escape intent.

### `output_state`
Owns held output for the authoritative session and bounded X/Y/wheel/pan accumulation. Ownership is needed within one Mouse because multiple physical buttons may map to the same target; it is not cross-Mouse aggregation.

### `logitech_hidpp`
Optional vendor backend for the relevant current/candidate session, with no UI or USB ownership.

### `usb_hid`
Sole TinyUSB descriptor/task/report owner. Fixed interface 0 Mouse + interface 1 minimal synthetic-Escape Keyboard.

### `product_storage`
Versioned/integrity-protected product persistence, separate from BT credentials.

### `interaction` / `ui_projector` / `renderer` / `hat`
Interaction emits semantic release actions; projector maps application state to screen models; renderer draws; HAT owns GPIO/debounce.

### `application`
Composition/orchestration only; never a second raw owner of BTstack, TinyUSB, GPIO/SPI or flash.

## Connected HOME projection and entry

`home-connected` is the live-Mouse HOME state. Its title is the current connected Mouse name, followed by four visible options: Pair New, current remap summary, Saved Devices and Learn the Keys. Pair New is a semantic navigation request only; it does not disconnect the current Mouse. The existing authoritative session remains active until replacement handoff.

## Unified HOME resolver

```text
resolve_home():
  if registry.empty():
    show searching-first
    ensure restartable FIRST_MOUSE 8-second cycle active
  else if mouse_session.authoritative_ready():
    show home-connected
    cancel/avoid SEARCH_SAVED
  else:
    show home-searching
    start 8-second SEARCH_SAVED
```

If SEARCH_SAVED expires/cancels, project `home-retry`.

A disconnect while HOME is visible invokes the resolver immediately. If another screen owns presentation, connection state updates but HOME resolution occurs on the next HOME access; this supports the Pair New help instruction to unplug the current Mouse and back out until searching appears.

## Pair New handoff

```text
request PAIR_NEW
 -> cancel incompatible saved-search transaction
 -> keep current authoritative Mouse live if present
 -> start 15-second new-only discovery
 -> ignore already-saved candidates for PAIR_NEW acceptance
 -> first unsaved candidate that is fully qualified becomes REPLACEMENT_READY
 -> freeze old authoritative session input
 -> release old held Mouse/Escape output
 -> disconnect/clear old authoritative session
 -> persist/confirm candidate
 -> promote candidate to sole authoritative ready session
 -> stop PAIR_NEW
```

If Pair New expires/cancels before `REPLACEMENT_READY` handoff, the original authoritative session is untouched.

If the current Mouse is manually unplugged during Pair New, its normal disconnect cleanup runs. Pair New may continue new-only search, but HOME, when accessed, resolves to saved search if saved mice exist and no new Mouse has taken over.

## Runtime topology

Start from accepted G06 BLE-only topology. Do not import G07 Classic-Keyboard multicore architecture.

There is no simultaneous-authoritative HIDS requirement. A transient candidate context may coexist only for qualification during an explicit replacement operation and cannot forward authoritative USB Mouse input before promotion.

Queues are bounded; parser/queue continuity loss is release-safe; flash work cannot starve radio/USB service.

## Architecture guards

Automated checks reject:

- textual `.c` includes;
- BTstack headers outside Bluetooth/runtime adapters;
- TinyUSB ownership outside `usb_hid`;
- raw SPI/GPIO outside renderer/HAT adapters;
- raw flash programming outside storage infrastructure;
- UI code calling BTstack/TinyUSB/flash;
- transport report structs in profile/remap/UI;
- Bluetooth Keyboard/Composite product modules;
- a second BT runtime owner;
- more than one authoritative ready Mouse;
- speculative simultaneous-Mouse aggregation/focus/capacity infrastructure;
- production diagnostic CDC.

## Fixed constants from MBR-00

- FIRST_MOUSE finite cycle: 8 seconds, automatically repeated while registry is empty;
- SEARCH_SAVED window: 8 seconds;
- PAIR_NEW window: 15 seconds;
- transport: BLE HOGP Mouse only;
- profile vocabulary: PASSTHROUGH, STANDARD, ESCAPE, CUSTOM;
- USB identity/interface shape: as frozen in `01-product-contract.md`.

## MBR-05 implemented wiring (clean rebuild)

`bt_runtime` initializes CYW43/BTstack on core 0 and serializes commands and queue
access using the SDK async-context lock. It composes the private `ble_hogp` adapter;
HOGP publishes through an injected callback (no reverse module dependency).
Callbacks never enter the application or TinyUSB. The 64-slot runtime ring has
63 usable entries and a fail-closed overflow latch. Application `bridge` consumes
it, validates transport/search generations, registers one live RAM session and
forwards canonical passthrough input through `output_state` to `usb_hid`.

The held owner table has five physical sources and six target refcounts, scoped
to one session. Motion saturates at ±32767 per axis. The USB queue preserves
button edges and only coalesces equal-held-state tail motion behind the in-flight
entry; endpoint acceptance controls subtraction. Disconnect/parser failure/queue
loss, USB lifecycle changes and HAT overflow enqueue neutral and invalidate the
session. UI lock does not enter this data path. Initial display flush completes
before radio initialization; later display flushes are bounded to two rows.

Production MBR-05 has real BLE passthrough. Qualification stays a separate radio-
free executable. Profiles/HID++, product flash, replacement handoff and removal
are still later gates; their unimplemented effects are never acknowledged as
successful. See `docs/implementation/06-mbr05-ble-passthrough.md`.
