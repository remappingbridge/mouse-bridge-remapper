# Documentation authority

## Principle

Mouse Bridge Remapper uses **documentation as product, code as consequence**.

The normative order is:

1. current user manual and architecture documents in this repository;
2. explicit later product decisions recorded by updating those documents;
3. accepted historical BLU2USB behavior where this documentation deliberately inherits it;
4. source code as an implementation of the documentation.

Code, tests, comments or historical branches do not override current documentation merely because they already exist.

## Historical baseline

The stable behavioral reference is BLU2USB G06 at accepted SHA:

`7eee024ad4ee726c5a85ffa2f32b9f47187878af`

The product inherits relevant mouse behavior and bug fixes from that baseline, including:

- BLE HOGP mouse discovery/classification;
- release-safe held-state cleanup on disconnect/overflow;
- fixed USB identity independent from Bluetooth state;
- profile/remap behavior;
- persistent profile and Custom state;
- bounded bonded reconnect/fallback;
- Logitech Lift HID++ Forward hold correction;
- release-triggered HAT interaction;
- current/applied cyan with selected/pressed white priority;
- accepted renderer geometry/pixel-relocation lessons.

The baseline is not copied blindly. The current product explicitly chooses **one live mouse connection at a time**, while retaining multiple saved mice.

## Superseded complexity

Any previously planned simultaneous multi-mouse runtime is superseded.

The implementation must not preserve multi-session HIDS coordination, cross-mouse held-button aggregation, connection-count UI, profile-focus rules for multiple live mice, or simultaneous-BLE capacity gates merely because they were documented earlier.

The current model is:

- many saved mice;
- zero or one connected mouse;
- Pair New replaces the live connection;
- HOME with saved mice and no live connection automatically starts a bounded saved-device search.

## Explicitly excluded historical behavior

This product does not expose Bluetooth pairing or saved-device flows for:

- Keyboard;
- Composite Mouse+Keyboard peers as a product type.

Historical G07+ Keyboard work is not an implementation baseline for this product.

## Escape exception

Escape is an explicit exception to the mouse-only Bluetooth input scope. A mouse button may generate standard USB Keyboard Escape through a minimal firmware-owned output capability.

This exception does not authorize:

- Bluetooth Keyboard discovery;
- Bluetooth Keyboard pairing;
- keyboard saved-device records;
- keyboard input parsing;
- Pair Keyboard UI;
- Pair Composite UI;
- general keyboard remapping.

## Normative vs explanatory text

Statements using `must`, `must not`, exact screen blocks, exact mapping tables, colors, limits and state-transition requirements are normative.

Architecture diagrams and rationale explain the contract but may not weaken normative behavior.

## Updating the product

A behavior change is made by updating documentation first. The update should identify:

- changed user-visible behavior;
- architecture/invariant impact;
- migration impact on persistent state when relevant;
- required verification changes.

Only after the documentation is coherent should code be changed to match it.

## Open decisions

If documentation explicitly marks a product decision open, implementation must preserve the ability to resolve it later and must not choose a user-visible behavior silently. Current open decisions are maintained in `09-open-decisions.md`.
