# System architecture

## Design goals

The architecture must make these product properties structural rather than accidental:

- several mice may be live at once;
- one mouse cannot release another mouse's held output;
- one pairing action accepts at most one new mouse;
- automatic search stops after its first accepted connection;
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
             | renderer      | | aggregator  |<------| mouse        |
             | ST7789        | | source-own. |       | sessions     |
             +---------------+ +------+------+       +------+-------+
                                      |                     |
                                      v                     v
                               +-------------+       +--------------+
                               | usb_hid     |       | ble_hogp     |
                               | Mouse +     |       | N sessions   |
                               | Escape sink |       +------+-------+
                               +-------------+              |
                                                            v
                                                     +--------------+
                                                     | bt_runtime   |
                                                     | one owner    |
                                                     +--------------+

application ---> mouse_registry ---> product_storage
ble_hogp -----> logitech_hidpp where capability requires it
bt_runtime ---> BTstack credential store
```

## Module responsibilities

### `domain`

Pure product types: MouseId, session identity, canonical button/motion events, profile identifiers, commands/results. No Pico SDK, BTstack or TinyUSB headers.

### `mouse_sessions`

Owns the runtime set of live mouse sessions. No single global active-mouse object may substitute for this set.

### `ble_hogp`

Owns BLE HID discovery/client/report-map parsing and converts transport reports into canonical mouse events. Per-connection parser/client state is mandatory.

### `bt_runtime`

Single owner of CYW43/BTstack lifecycle. There may not be competing initialization/poll owners.

### `pairing_coordinator`

Owns search transaction purpose, single-winner acceptance, timeouts, cancellation and reconnect policy.

### `mouse_registry`

Owns saved identities, names, confirmed profile kinds and persistent product metadata. Saved is not synonymous with connected.

### `profiles`

Owns profile tables and the global Custom template/draft semantics.

### `remap`

Maps canonical physical mouse-button events to canonical target mouse buttons or synthetic Escape intent. Pure/host-testable.

### `mouse_aggregator`

Owns per-source held-state aggregation and relative motion accumulation across all live mice.

### `logitech_hidpp`

Optional vendor backend bound to a specific mouse session. No UI or USB ownership.

### `usb_hid`

Sole TinyUSB device/report owner. Emits aggregated Mouse reports plus the minimal Keyboard report state needed for synthetic Escape.

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

## Runtime topology

The starting runtime topology should follow the physically accepted BLE-only G06 approach unless measurements require a documented change. The later BLU2USB Classic-Keyboard multicore architecture is not imported merely because it exists historically.

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
- one global `active_mouse` used as the complete runtime model;
- diagnostic CDC in production descriptors.
