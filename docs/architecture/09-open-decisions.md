# Open product decisions

The documentation is authoritative even when it explicitly records an unresolved choice. These items must not be decided silently in code.

Decisions already resolved by the current product contract are **not** listed here: single live Mouse connection, multiple saved mice, one-winner searches, automatic saved search whenever HOME has saved mice but no connection, `DEVICE NOT FOUND` after saved-search timeout, Pair New replacement semantics, cyan connected name in Saved Devices, BLE HOGP Mouse scope, no Bluetooth Keyboard/Composite pairing, and synthetic Escape retention are all frozen.

## 1. `DEFAULT REMAP` vs `STANDARD REMAP` wording

**Open editorial/product vocabulary.**

The current Mouse Options menu says `DEFAULT REMAP`; dedicated apply/active pages and HOME summary use `STANDARD` terminology.

They currently denote the same mapping. Internal profile identity must not depend on either display string.

A future documentation edit should choose whether to normalize all user-visible wording or deliberately retain the alias.

## 2. Exact saved-search and Pair New timeouts

**Open policy constant.**

First-mouse search is logically indefinite. Saved search and Pair New are finite, but exact durations are not yet specified.

Timeouts belong to coordinator policy, not BLE parser code.

## 3. Long mouse-name presentation

**Open renderer policy.**

Dynamic names may exceed the 21-character screen width. Storage should retain the complete normalized available name, but the UI still needs one deterministic rule such as truncation, ellipsis or scrolling.

No implementation should invent a policy based on whatever string happens to fit.

## 4. Final USB identity strings and VID/PID

**Open descriptor identity.**

The USB device must be fixed from boot and expose Mouse plus minimal synthetic-Escape Keyboard capability, but the final project-specific VID/PID/manufacturer/product strings are not yet frozen.

Historical `BLU2USB` names must not become the final Mouse Bridge Remapper identity by inertia.

## 5. `FIRST MOUSE CONNECTED` control semantics

**Open interaction detail.**

The supplied screen visually associates `LOCK SCREEN`, `AND UNLOCK` and `OPEN HOME -> KEY Y` with right-side key labels, but does not fully specify the complete release actions/lifetime of the page.

The renderer may display the documented layout, but the interaction implementation needs an explicit control table before behavior is considered final.

## 6. `KEY B: BACK TRY SAVED` exact navigation detail

**Open interaction detail.**

The resulting product behavior is already constrained: once navigation reaches HOME with saved mice and no live connection, `home-searching` automatically starts the bounded saved-device search.

What remains to freeze is only the exact one-step navigation destination and visible transition caused by `KEY B` on `retry-pair-new`; implementation must not introduce a second independent reconnect policy.

## 7. Disconnected status word in Saved Devices

**Open display vocabulary.**

`STATUS: CONNECTED` is defined for the one current ready session. The exact text for every other saved mouse is not yet frozen (`SAVED`, `DISCONNECTED`, etc.).

The underlying state is unambiguous even while the display word remains open.

## Explicitly closed former decisions

The following older planning questions are obsolete and must not be reintroduced:

- which mouse HOME targets when several are connected;
- how to display `N DEVICES CONNECTED`;
- simultaneous BLE Mouse capacity;
- cross-mouse button ownership;
- multi-session reconnect scheduling;
- multi-Mouse UI focus selection.

They were eliminated by the product decision that only one mouse may be connected at a time.
