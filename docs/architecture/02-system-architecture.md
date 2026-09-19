# System architecture

Status: **FROZEN BY MBR-00**.

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
