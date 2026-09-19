# System architecture

## Design goals

The architecture must make these product properties structural rather than accidental:

- many mice may be saved, but only one may be live at a time;
- HOME has one deterministic connection target and one remap target;
- Pair New replaces the current live connection instead of overlapping sessions;
- saved-device search starts automatically whenever HOME is entered with saved mice and no live connection;
- a bounded saved search ends at `DEVICE NOT FOUND` if no saved mouse reconnects;
- one discovery transaction accepts at most one winner;
- Bluetooth details do not leak into remap/UI/USB code;
- Bluetooth Keyboard and Composite product roles do not exist;
- synthetic Escape remains possible through a narrowly scoped USB output path;
- persistence and BT credentials remain separate ownership domains;
- UI never directly operates BTstack or flash.

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
                               | Mouse +     |       | one session  |
                               | Escape sink |       +------+-------+
                               +-------------+              |
                                                            v
                                                     +--------------+
                                                     | bt_runtime   |
                                                     | one owner    |
                                                     +--------------+

application ---> mouse_registry ---> product_storage
ble_hogp -----> logitech_hidpp for the current session when capability requires it
bt_runtime ---> BTstack credential store
```

## Module responsibilities

### `domain`

Pure product types: MouseId, MouseSessionId, canonical button/motion events, profile identifiers, commands/results. No Pico SDK, BTstack or TinyUSB headers.

### `mouse_session`

Owns the single optional live mouse session. Its invariant is `ready_count <= 1`.

A stale generation callback cannot mutate a newer session.

### `ble_hogp`

Owns BLE HID discovery/client/report-map parsing for the current candidate/live session and converts transport reports into canonical mouse events.

### `bt_runtime`

Single owner of CYW43/BTstack lifecycle. There may not be competing initialization or poll owners.

### `pairing_coordinator`

Owns transaction purpose, one-winner acceptance, timeout/cancellation, saved-device search and Pair New replacement sequencing.

It is the only layer allowed to decide when the current session must be disconnected before another candidate may become ready.

### `mouse_registry`

Owns the set of saved identities, names, confirmed profile kinds and persistent product metadata. Multiple saved records are allowed; saved is not synonymous with connected.

### `profiles`

Owns profile tables and global Custom template/draft semantics.

### `remap`

Maps canonical physical mouse-button events to canonical target mouse buttons or synthetic Escape intent. Pure/host-testable.

### `output_state`

Owns held Mouse-button state, synthetic Escape held state, and bounded relative motion/wheel/pan accumulation for the one live session.

It is not a cross-mouse aggregator.

### `logitech_hidpp`

Optional vendor backend bound to the current mouse session. No UI or USB ownership.

### `usb_hid`

Sole TinyUSB device/report owner. Emits Mouse reports plus the minimal Keyboard report state needed for synthetic Escape.

### `product_storage`

Owns versioned/integrity-protected product persistence and power-loss-safe commits. It does not own BTstack security credentials.

### `interaction`

Converts HAT physical transitions into release-triggered semantic commands.

### `ui_projector`

Converts application state to canonical screen models. Async connection/disconnection events redraw state without waiting for another HAT event.

### `renderer`

Owns text/color/pixel rendering only. It receives screen models; it does not decide connection or profile behavior.

### `hat`

Owns GPIO/debounce/control-state adaptation.

### `application`

Composition and orchestration only. It may not become a second raw owner of BTstack, TinyUSB, SPI/GPIO or flash serialization.

## Unified HOME resolver

HOME entry is modeled as one application decision:

```text
resolve_home():
  if registry.empty():
    show searching-first
    ensure first-search active
  else if mouse_session.ready():
    show home-connected
    ensure no saved-search transaction is running
  else:
    show home-searching
    start/restart bounded saved-search transaction
```

A live `MouseDisconnected` event while saved records remain feeds the same resolver. This avoids a separate reconnect state machine with different UX semantics.

A saved-search timeout projects `home-retry` and stops searching until the user retries or HOME is entered again through navigation.

## Pair New replacement sequencing

Pair New never creates overlap:

```text
request Pair New
 -> cancel incompatible search transaction
 -> if live mouse exists:
      freeze new input from that session
      release held output
      disconnect session
      clear live slot
 -> begin new-only discovery
 -> first accepted unsaved candidate becomes sole live session
 -> stop transaction
```

If Pair New fails or is canceled after the old mouse was disconnected, the old mouse remains only as a saved record. Normal HOME entry can search saved devices again.

## Runtime topology

The starting runtime topology should follow the physically accepted BLE-only G06 approach unless measurements require a documented change. The later BLU2USB Classic-Keyboard multicore architecture is not imported merely because it exists historically.

There is no requirement for simultaneous HIDS client contexts.

Regardless of core placement:

- USB servicing must remain bounded/responsive;
- Bluetooth callbacks must not run unbounded UI/storage work;
- flash writes must be coordinated with radio/USB safety requirements;
- queue overflow must fail release-safe.

## Architecture guards

Static/host checks should reject:

- textual `.c` includes;
- BTstack headers outside Bluetooth/runtime adapters;
- TinyUSB ownership outside `usb_hid`;
- raw SPI/GPIO outside renderer/HAT platform adapters;
- raw flash erase/program outside storage infrastructure;
- UI code calling BTstack/TinyUSB/flash directly;
- transport report structs in profiles/remap/UI;
- Bluetooth Keyboard/Composite product modules;
- a second BT runtime owner;
- more than one ready mouse session;
- simultaneous-HIDS/multi-mouse aggregation structures that contradict the product contract;
- diagnostic CDC in production descriptors.
