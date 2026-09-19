# MBR-00 frozen decisions

Status: **NO OPEN PRODUCT DECISION BLOCKS MBR-01 THROUGH MBR-07**.

MBR-00 closed the implementation-facing ambiguities that existed before the contract freeze.

## Frozen vocabulary

- canonical profile: `STANDARD`, visible as `STANDARD REMAP`;
- `DEFAULT REMAP` is historical alias only;
- didactic title: `PRESS TO LEARN KEYS`;
- canonical first-search screen ID: `searching-first`;
- disconnected Saved Devices status: `STATUS: DISCONNECTED`.

## Frozen timing

- FIRST_MOUSE finite discovery cycle: 8 seconds, automatically repeated while no Mouse is saved;
- SEARCH_SAVED: 8 seconds;
- PAIR_NEW: 15 seconds.

## Frozen Pair New behavior

- current healthy Mouse stays connected during new-only search;
- saved candidates are ignored as Pair New winners;
- first qualified unsaved candidate becomes replacement-ready;
- old Mouse is released/disconnected only at handoff;
- old saved record/bond remains;
- timeout/cancel before handoff keeps old Mouse live;
- Pair New never silently changes into saved search;
- to reconnect a saved Mouse, unplug current Mouse and Back until HOME resolves to SEARCHING, matching the frozen Help text.

## Frozen navigation/controls

- `KEY B: BACK TRY SAVED` invokes the ordinary HOME resolver; with no live Mouse this produces `home-searching`, with a live Mouse it produces `home-connected`;
- `JOY LEFT: GO TO HOME` on `escape-active` is intentional and directly invokes HOME resolver;
- no hidden Lock controls are inherited;
- ordinary `KEY Y: LOCK` exists only where visibly declared;
- `first-mouse-connected` and `learn-the-keys`: Key B Lock, Key X Unlock while locked, Key Y HOME; joystick and Key A are didactic;
- `searching-first`: all HAT controls are didactic only.

## Frozen dynamic-name policy

- persist full normalized available name within schema limits;
- render first 21 supported characters;
- no ellipsis/scrolling;
- fallback `UNKNOWN MOUSE`.

## Frozen transport

Mouse transport is BLE HOGP only. Bluetooth Classic Mouse is not included.

## Frozen USB development identity

- VID `0xCAFE`;
- PID `0x4011`;
- bcdDevice `0x0100`;
- manufacturer `tiagooliveirajs`;
- product `Mouse Bridge Remapper`;
- no serial string;
- HID Mouse interface 0;
- minimal synthetic-Escape HID Keyboard interface 1;
- no CDC/debug interface.

`0xCAFE` is a project/development convention, not a claim of commercial USB-IF vendor allocation. A future commercial distribution identity would require an explicit product/release contract change before changing accepted USB identity.

## Frozen typo normalization

Canonical screen reference corrects obvious transcription errors without changing behavior, including:

- `T0` -> `TO`;
- `FORWARED` -> `FORWARD`;
- `kEY` -> `KEY`;
- `DEFAULT OPTIONS` -> `DEFAULT OPTION`.

## Former multi-Mouse decisions are closed by removal

Do not reintroduce:

- more than one authoritative ready Mouse;
- `N DEVICES CONNECTED`;
- count 999;
- cross-Mouse held aggregation;
- multi-live-Mouse focus selection;
- simultaneous-HIDS capacity qualification.

## Change rule

Any future change to the above is a product-contract change. It must update documentation/planning before implementation and must identify affected predecessor evidence.
