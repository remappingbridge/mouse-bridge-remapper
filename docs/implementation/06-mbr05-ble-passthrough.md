# MBR-05 — BLE Mouse passthrough, clean rebuild

Status: implementation candidate; operator physical acceptance pending.
Predecessor: `8bebe26ff7554ffa811a58ea024cae5fe80a4a84` (rebuilt MBR-00..04,
explicitly accepted by the operator on 2026-09-20). Branch: `mbr/mbr-05-ble-mouse`.
This record supersedes pre-reset MBR-05 implementation claims.

## Implemented behavior

- BLE discovery, NoInputNoOutput security, bonding through SDK BTstack TLV,
  GAP name read, HIDS Report Protocol; Mouse Report Map required before readiness.
- Five canonical buttons, signed packed X/Y, wheel/pan, multiple report IDs;
  exact-length framing accepts raw payload or the one duplicated ID prepended by
  pinned BTstack (including ID zero). Unknown IDs/malformed packets fail closed.
- Keyboard/other standard non-Mouse application collections and multiple HIDS
  instances are rejected. Vendor page 0xFF00 is allowed without requiring HID++.
- One authoritative RAM Mouse session, separate monotonically generated radio
  and application identities; stale report/disconnect/transaction rejection.
- Physical-button owners and target refcounts; duplicate edges are idempotent.
  All accumulators saturate at ±32767. USB consumes deltas only after acceptance;
  equal-state motion can coalesce behind the in-flight entry, never across an edge.
- Runtime queue: 64 slots/63 usable, async-context lock; overflow latches, discards
  uncertain events, releases Mouse/Escape and disconnects. USB edge overflow,
  malformed reports, HAT overflow and USB lifecycle change also release safely.
- One core-0 CYW43/BTstack owner, inherited threadsafe-background context. UI first
  frame finishes before radio start; display work then stays at two rows/iteration.
  Lock only affects UI/backlight; passthrough remains active.
- UI amendment: HOME name first 15 supported characters + conditional ` MOUSE`,
  gray/white HOME actions, full dark-magenta searching-first/first-mouse-connected/
  learn-the-keys. General saved/removal name projection remains 21 characters.
- Qualification executable retains the accepted RAM fixtures and has no radio.

## Migration manifest

All inherited source was inspected at immutable BLU2USB G06
`7eee024ad4ee726c5a85ffa2f32b9f47187878af` before adaptation.

| G06 source | MBR destination/adaptation |
| --- | --- |
| `src/ble_hogp/ble_hogp.c` | `src/ble_hogp/ble_hogp.c`: bounded descriptor/bit parser; session IDs, stricter composite/map rejection, 64-bit sums |
| `src/ble_hogp/ble_hogp_pico.c` | `src/ble_hogp/ble_hogp_pico.c`: security/HIDS discovery and callbacks; explicit transaction/handle/CID guards, name read, accepted RAM identities |
| `src/bt_runtime/bt_runtime_pico.c` | `src/bt_runtime/bt_runtime_pico.c`: core-0 threadsafe-background initialization, locked command/queue facade |
| `include/btstack_config.h` | `include/btstack_config.h`: BLE-only fixed-capacity configuration; diagnostic print definitions removed |
| `src/bt_runtime/g05_hog_host.gatt` | `src/bt_runtime/mbr_hog_host.gatt`: product GAP name |

No G07 implementation or Bluetooth Keyboard/Classic transport was imported.

## Gate boundaries and recovery

This gate registers product identities/names in RAM. Bluetooth bonds alone persist
in SDK-owned banks (0x3FD000..0x3FEFFF on the 4 MiB Pico 2 W); no product-storage
schema has been introduced. On reboot the product registry starts empty and the
first-Mouse flow runs again. Do not claim reboot saved-registry parity yet.
Same-boot reconnect uses accepted identities, resolving list and whitelist.
Existing unrelated bonds never become product records merely because they exist.

Profiles/Custom application, durable registry, HID++ diversion, full Pair New
replacement and removal remain MBR-06/07/08. A healthy current mouse is kept alive
while Pair New counts down; no replacement is promoted by this gate. Unimplemented
profile/removal requests are rejected without success confirmation. Generic BLE
passthrough does not require any Logitech backend. Lift-specific Forward behavior
is not claimed before MBR-06.

USB identity/descriptors remain byte-for-byte MBR-04. Reflash accepted MBR-04 UF2
for rollback; it does not consume BLE credentials. No automatic bond erase is
performed. For a fresh physical pair, use an available mouse pairing channel;
reboot registry persistence is outside this candidate's acceptance scenarios.

## Verification

Host: core/UX/renderer/USB/HOGP/output/bridge/qualification + architecture and
canonical screens. Parser cases cover packed signed 12-bit fields, all five
buttons, duplicate IDs (also ID zero per SDK), multiple IDs, truncation, malformed
map, keyboard/composite rejection and failed sinks. Bridge tests cover stale
transactions/sessions, locked forwarding, held disconnect, reconnect generation,
USB overflow/lifecycle cleanup and runtime overflow. Predecessor tests remain.

CI uses exact PR HEAD (not its synthetic merge ref), pinned SDK 2.2.0 SHA
`a1438dff1d38bd9c65dbd693f0e5db4b9ae91779`, BTstack submodule
`501e6d2b86e6c92bfb9c390bcf55709938e25ac1`, GCC package
`15:13.2.rel1-2`, `pico2_w`, Release. It builds and verifies both UF2s.
Final commit/run/artifact/hash evidence is recorded in the planner execution
`mouse-bridge-remapper/executions/rebuild-mbr-05/candidate-evidence.md`.
Only the operator may close physical MBR-05. MBR-06 remains blocked until then.
