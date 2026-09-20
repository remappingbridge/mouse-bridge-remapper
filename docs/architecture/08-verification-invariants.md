# Verification and invariants

Status: **FROZEN BY MBR-00**.

These are mandatory implementation/test properties.

## Architecture invariants

1. Exactly one Bluetooth runtime owner exists.
2. BLE report layouts do not cross the canonical input boundary.
3. TinyUSB ownership exists only in `usb_hid`.
4. UI does not call BTstack, TinyUSB or flash primitives directly.
5. Product state is separated from BT credentials.
6. No Bluetooth Keyboard/Composite product module or UI flow exists.
7. Synthetic Escape is output-only.
8. At most one Mouse is authoritative/ready at any time.
9. Multiple saved Mouse records are allowed.
10. Speculative simultaneous-Mouse aggregation/focus/capacity infrastructure is prohibited.

## Search invariants

1. FIRST_MOUSE uses restartable 8-second cycles and is logically continuous until one valid first Mouse is accepted.
2. SEARCH_SAVED lasts 8 seconds per attempt.
3. PAIR_NEW lasts 15 seconds per attempt.
4. Every transaction accepts at most one winner.
5. HOME with saved mice and no live Mouse starts SEARCH_SAVED automatically.
6. SEARCH_SAVED expiry/cancel produces `DEVICE NOT FOUND`.
7. An authoritative disconnect while HOME is visible immediately causes saved-search HOME; otherwise HOME resolver applies when HOME is next accessed.
8. Pair New accepts only unsaved BLE HOGP Mouse candidates; already-saved candidates are ignored for Pair New acceptance.

## Pair New handoff invariants

1. Starting Pair New does not disconnect a healthy current Mouse.
2. The current Mouse remains authoritative/usable while an unsaved replacement candidate is being discovered/qualified.
3. A candidate cannot forward authoritative Mouse output before handoff.
4. Replacement handoff releases old held Mouse/Escape state before old-session disposal.
5. Old saved record/bond survives replacement.
6. New candidate becomes authoritative only after old authoritative session is cleared and required new product state is confirmed.
7. Timeout/cancel before handoff leaves the original current Mouse connected.
8. Manually unplugging the current Mouse while Pair New/help is visible updates connection truth but does not change Pair New into a saved search; HOME later resolves to saved search.

## Single-session HID invariants

1. Current session has explicit held ownership for Mouse targets and Escape.
2. Two physical source buttons mapping to the same target cannot cause premature release.
3. Disconnect/handoff/removal/profile transition/continuity loss is release-safe.
4. Duplicate Down/Up is idempotent.
5. Relative movement/wheel/pan remain transient and honor USB backpressure.
6. Stale callbacks from old generations cannot mutate the current/replacement session.

## UI invariants

1. `home-connected` represents exactly one authoritative Mouse and uses its projected name as the dynamic title.
2. `home-connected` exposes exactly four visible options in order: Pair New, current remap summary, Saved Devices, Learn the Keys.
3. Selecting Pair New from `home-connected` enters `pair-new` without disconnecting the current Mouse.
4. No `N DEVICES CONNECTED` form exists.
5. Saved Devices has at most one cyan/CONNECTED page.
6. Disconnected saved Mouse status is exactly `STATUS: DISCONNECTED`.
7. Name projection is deterministic: first 21 supported characters; fallback `UNKNOWN MOUSE`.
8. Selected/pressed white overrides cyan.
9. Async state events update relevant visible UI without waiting for unrelated input.
10. Actions execute on release.
11. Help owns/consumes all controls.
12. Hidden controls are not inferred from old BLU2USB screens.
13. Pair New Help text is the exact frozen text in the canonical screen reference.
14. `JOY LEFT: GO TO HOME` on `escape-active` is a deliberate direct-HOME exception.

## Profile invariants

1. Canonical profile names are PASSTHROUGH, STANDARD, ESCAPE, CUSTOM.
2. Standard mapping equals the documented table; historical Default is an alias only.
3. Movement/wheel/pan pass through all profiles.
4. Escape profile produces standard USB Escape press/hold/release.
5. Custom permits Escape and preserves draft separately from confirmed active profile.
6. Apply success appears only after runtime+persistence confirmation.
7. Logitech HID++ failure does not break generic HOGP input.
8. Logitech Forward diversion preserves real hold semantics.

## Persistence invariants

1. Saved identities/names/profile kinds survive reboot.
2. Global Custom template and dirty/unapplied draft survive as documented.
3. Corrupt/torn newest generation falls back to previous valid generation.
4. Product writes cannot overwrite BT credentials.
5. Removal coordinates product association and credential deletion recoverably.
6. Pair New replacement never deletes the old Mouse record/bond.
7. Removing last saved Mouse returns to first-Mouse search.

## USB invariants

1. VID/PID/string/interface contract equals MBR-00 frozen identity.
2. USB identity is fixed from boot.
3. Bluetooth/profile/UI state does not force re-enumeration.
4. Interface 0 is Mouse; interface 1 is minimal synthetic-Escape Keyboard output.
5. No production CDC/debug interface exists.
6. Bluetooth report handles/layouts never reach USB report construction.

## Inherited G06 regression lessons

Preserve:

- release safety on disconnect/overflow;
- Report-ID framing normalization;
- event-driven connection UI;
- Custom draft immediate reflection;
- success only after persistence confirmation;
- power-loss-safe product state;
- bounded bonded reconnect/fallback;
- Logitech Forward held-state correction;
- release-triggered HAT actions;
- accepted pixel relocation and color priority.

## Evidence discipline

A build proves compilation only. Physical behavior—BLE pairing/reconnect, replacement handoff, held-state cleanup, HAT/display placement, timeout behavior, HID++ and USB enumeration—requires physical evidence at the gates that explicitly require it. MBR-00 itself requires no build, UF2 or physical test.
