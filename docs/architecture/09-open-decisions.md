# Open product decisions

The documentation is authoritative even when it explicitly records an unresolved choice. These items must not be decided silently in code.

Decisions already resolved by the current product contract are **not** listed here: multiple simultaneous mice, the 999 UI/product bound, one-winner searches, no automatic second-mouse search, cyan connected names in Saved Devices, BLE HOGP Mouse scope, no Bluetooth Keyboard/Composite pairing, and synthetic Escape retention are all already frozen.

## 1. Profile target when several mice are connected

**Open and material.**

When exactly one mouse is connected, HOME can naturally open that mouse's remapper.

When two or more mice are connected, line 2 intentionally shows only `N DEVICES CONNECTED`. The current UI has no documented action that chooses which mouse receives a profile edit from HOME.

Implementation must not choose any of these implicitly:

- first connected;
- last connected;
- most recently moved;
- last paired;
- all connected mice.

Architecture therefore keeps a possible UI configuration target/focus separate from the runtime set of connected mice until a user-facing selection rule is documented.

## 2. `DEFAULT REMAP` vs `STANDARD REMAP` wording

**Open editorial/product vocabulary.**

The current Mouse Options menu says `DEFAULT REMAP`; dedicated apply/active pages and HOME summary use `STANDARD` terminology.

They currently denote the same mapping. Internal profile identity must not depend on either display string.

A future documentation edit should choose whether to normalize all user-visible wording or deliberately retain the alias.

## 3. Exact saved-search and Pair New timeouts

**Open policy constant.**

First-mouse search is logically indefinite. Saved search and Pair New are finite, but exact durations are not yet specified.

Timeouts belong to coordinator policy, not BLE parser code.

## 4. Long mouse-name presentation

**Open renderer policy.**

Dynamic names may exceed the 21-character screen width. Storage should retain the complete normalized available name, but the UI still needs one deterministic rule such as truncation, ellipsis or scrolling.

No implementation should invent a policy based on whatever string happens to fit.

## 5. Final USB identity strings and VID/PID

**Open descriptor identity.**

The USB device must be fixed from boot and expose Mouse plus minimal synthetic-Escape Keyboard capability, but the final project-specific VID/PID/manufacturer/product strings are not yet frozen.

Historical `BLU2USB` names must not become the final Mouse Bridge Remapper identity by inertia.

## 6. `FIRST MOUSE CONNECTED` control semantics

**Open interaction detail.**

The supplied screen visually associates `LOCK SCREEN`, `AND UNLOCK` and `OPEN HOME -> KEY Y` with right-side key labels, but does not fully specify the complete release actions/lifetime of the page.

The renderer may display the current documented layout, but the interaction implementation needs an explicit control table before behavior is considered final.

## 7. `KEY B: BACK TRY SAVED` side effect

**Open navigation/search detail.**

On `retry-pair-new`, it is not yet explicit whether Key B merely navigates to the prior saved-search/home screen or also immediately starts a saved-device search.

Navigation and starting a search transaction are separate commands and must not be conflated by guesswork.

## 8. Disconnected status word in Saved Devices

**Open display vocabulary.**

`STATUS: CONNECTED` is defined for a ready live session. The exact text for a saved-but-not-connected mouse is not yet frozen (`SAVED`, `DISCONNECTED`, etc.).

The underlying state is unambiguous even while the display word remains open.

## 9. Physically supportable simultaneous BLE count

**Open engineering evidence, not an open product/UI limit.**

The product limit is 999. Actual RP2350/BTstack concurrent BLE HOGP capacity must be measured and documented honestly.

If hardware cannot meet the product target, documentation must be revised explicitly rather than silently imposing a smaller limit in code while leaving the manual at 999.
