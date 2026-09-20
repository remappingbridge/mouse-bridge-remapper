# MBR-01 implementation scaffold

Status: implementation scaffold introduced by `mbr-01`.

This gate establishes compileable ownership and dependency boundaries only. It does **not** claim that UI, USB HID behavior, BLE pairing/forwarding, remapping, persistence, HID++, renderer/HAT behavior, saved-device lifecycle or Pair New behavior are implemented.

## Frozen module graph

The build materializes these conceptual modules:

- domain
- mouse_registry
- mouse_session
- output_state
- profiles
- remap
- pairing_coordinator
- bt_runtime
- ble_hogp
- logitech_hidpp
- product_storage
- usb_hid
- interaction
- ui_projector
- renderer
- hat
- app

The exact dependency facade is `cmake/MouseBridgeRemapperModules.cmake` and is checked by `tests/test_architecture.py`.

## Structural runtime invariant

`mouse_session` owns exactly one authoritative slot. `pairing_coordinator` may represent a separate non-authoritative replacement candidate. The candidate does not increase the authoritative ready count.

This encodes the MBR-00 rule that there may never be two authoritative ready mice.

## Ownership guards

Automated checks reject:

- superseded Classic-HID/keyboard-transport/multi-Mouse architecture;
- raw Bluetooth runtime primitives outside `bt_runtime`, `ble_hogp`, `logitech_hidpp`;
- TinyUSB ownership outside `usb_hid`;
- GPIO/SPI ownership outside `hat`/`renderer`;
- raw flash ownership outside `product_storage`;
- raw platform/transport primitives in host-pure modules or `app`;
- textual `.c` includes and transport/HAL macro interception;
- forced USB re-enumeration and production diagnostic CDC/stdio;
- production board other than `pico2_w`.

## Build baseline

Starting inputs are intentionally inherited from accepted BLU2USB G06 build evidence:

- Ubuntu 24.04 CI runner;
- Pico SDK 2.2.0;
- ARM GCC package `15:13.2.rel1-2` / upstream 13.2.Rel1.

The scaffold builds both host tests and a Pico 2 W UF2. The UF2 is structural build evidence only and is not a usable-feature or physical-acceptance claim.
