# Documentation authority

Status: **MBR-00 CONTRACT FROZEN**.

## Principle

Mouse Bridge Remapper uses **documentation as product, code as consequence**.

Normative order:

1. current user manual and architecture documents in this repository;
2. explicit later product decisions recorded by updating those documents;
3. accepted historical BLU2USB behavior where current docs deliberately inherit it;
4. source code as an implementation of documentation.

Code/tests/comments/history never override current documentation by inertia.

## Historical baseline

Stable behavioral reference: BLU2USB G06 accepted SHA

`7eee024ad4ee726c5a85ffa2f32b9f47187878af`

Inherited Mouse lessons include BLE HOGP classification, release-safe cleanup, fixed USB structure, profiles/remap, persistent profile/Custom state, bounded bonded reconnect, Logitech HID++ Forward hold correction, release-triggered HAT behavior, color priority and accepted renderer geometry.

The baseline is evidence, not a tree to copy blindly.

## Current product model

- many saved Mouse records;
- zero or one authoritative connected Mouse;
- optional non-authoritative replacement candidate only during explicit Pair New qualification;
- Pair New keeps a healthy current Mouse live until replacement handoff;
- HOME with saved mice + no live Mouse starts bounded saved search;
- Bluetooth Mouse transport is BLE HOGP only.

Previously planned simultaneous-authoritative-Mouse runtime/count/focus/capacity behavior is superseded and must not be preserved as speculative future-proofing.

## Pair New authority

The latest Pair New Help/flow clarification is normative:

- healthy current Mouse remains connected during Pair New search;
- saved candidates are not accepted by Pair New;
- an unsaved candidate is qualified as non-authoritative replacement-ready;
- old Mouse is release-cleaned/disconnected only at handoff;
- old saved record/bond remains;
- timeout/cancel before handoff leaves old Mouse live;
- to reconnect a saved Mouse, user unplugs current Mouse and Backs until HOME reaches SEARCHING.

## Explicitly excluded Bluetooth behavior

No:

- Bluetooth Keyboard discovery/pairing/input;
- Bluetooth Composite Mouse+Keyboard product role;
- Bluetooth Classic Mouse;
- Keyboard/Composite saved records or UI;
- G07+ Keyboard implementation as production baseline.

## Escape exception

A Mouse button may generate standard USB Keyboard Escape through the fixed minimal output interface. This does not authorize Bluetooth Keyboard support or general keyboard remapping.

## Canonical screen authority

`docs/manual/06-screen-reference.md` is the literal screen/control contract. Exact rows, controls, coordinates, status words, profile vocabulary and Help text are implementation requirements.

## Frozen decisions

MBR-00 closed implementation-blocking product decisions. `docs/architecture/09-open-decisions.md` now records those frozen choices rather than an unresolved queue.

Any future change must update documentation first and state its architecture, persistence and verification impact before code changes.
