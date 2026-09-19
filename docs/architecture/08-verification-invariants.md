# Verification and invariants

This document defines the product properties that implementation and tests must preserve. It does not schedule implementation work.

## Architecture invariants

1. Exactly one Bluetooth runtime owner exists.
2. BLE report layouts do not cross the canonical input boundary.
3. TinyUSB device/report ownership exists only in the USB module.
4. UI does not call BTstack, TinyUSB or flash primitives directly.
5. Product persistence is separate from BT credentials.
6. No Bluetooth Keyboard/Composite product module or UI flow is introduced.
7. Synthetic Escape is output-only and cannot become an excuse for Bluetooth keyboard input support.
8. At most one Mouse session may be ready/connected at any time.
9. Multiple saved mice are allowed and remain independent persistent records.

## Pairing/search invariants

1. First-mouse search continues until one valid first mouse is accepted.
2. Every discovery transaction accepts at most one winner.
3. If multiple candidates satisfy a transaction, the first valid accepted candidate wins and that transaction stops.
4. Saved-device search is bounded.
5. Whenever HOME is entered with saved mice and no connected mouse, saved-device search starts automatically.
6. If saved-device search expires without a connection, HOME becomes `DEVICE NOT FOUND`.
7. If the current mouse disconnects while saved mice remain, HOME enters `home-searching` and starts the same bounded saved-device search automatically.
8. Pair New disconnects the current live mouse before a new candidate may become ready.
9. Pair New failure/cancel does not delete the previous mouse's saved record or bond.
10. Pair New does not silently reconnect the previous mouse; ordinary HOME resolution performs saved search when appropriate.

## Single-session HID invariants

1. The current session has explicit held-state ownership for Mouse buttons and synthetic Escape.
2. Two physical source buttons from the same mouse mapping to the same output cannot cause premature release.
3. Disconnect/replacement/removal releases all held output from the outgoing session before it is discarded.
4. Profile transition clears stale held state from the old mapping before the new mapping becomes authoritative.
5. Duplicate button transitions are idempotent.
6. Queue/parser failure is release-safe.
7. Movement/wheel/pan remain transient and are consumed only according to the USB backpressure contract.
8. Stale callbacks from an old session generation cannot affect the replacement session.

## UI invariants

1. `home-connected` always represents exactly one connected mouse.
2. HOME line 2 shows that mouse's name; there is no multi-device count form.
3. The remap summary and remapper actions refer to that same connected mouse.
4. In Saved Devices, only the connected mouse's name may be cyan; at most one page is cyan at a time.
5. Selected/pressed white overrides current/connected cyan.
6. Async connect/disconnect/search-timeout events redraw relevant UI without requiring another HAT event.
7. Actions execute on release.
8. Help uses its own interaction ownership.
9. Removed Keyboard/Composite pages cannot reappear in the screen registry.
10. No simultaneous-mouse focus/selector state is present.

## Profile invariants

1. Preset mappings equal the documented tables.
2. Movement/wheel/pan pass through all profiles.
3. Escape profile produces standard USB Escape press/hold/release.
4. Custom allows Escape as a target.
5. Apply success is shown only after runtime and persistence confirmation.
6. Applying another preset does not silently destroy saved Custom draft/template state.
7. Logitech HID++ failure does not break generic HID mouse input.
8. Logitech Forward diversion, when used, preserves physical hold semantics.

## Persistence invariants

1. Saved identities/profile state survive reboot.
2. Multiple saved mice may persist even though only one can be connected.
3. Custom template survives reboot.
4. Accepted unapplied Custom draft state survives reboot if the inherited behavior is retained.
5. Corrupt/torn newest product generation falls back to a previous valid generation.
6. Product-state storage cannot overwrite BLE credentials.
7. Removing a mouse removes its product association and matching credential state transactionally/recoverably.
8. Pair New disconnecting a mouse does not remove its saved product state.
9. Removing the last saved mouse returns to first-mouse search.

## USB invariants

1. USB identity is fixed from boot.
2. Bluetooth activity/profile changes do not force re-enumeration.
3. Mouse output and minimal synthetic-Escape Keyboard output coexist under the fixed descriptor.
4. Production USB has no diagnostic CDC interface.
5. No Bluetooth transport handle/report layout reaches USB report construction.

## Regression lessons from BLU2USB G06

The new implementation must preserve the lessons behind accepted fixes, not merely approximate their final appearance:

- release safety on disconnect/overflow;
- Report-ID framing normalization;
- connection-state-driven UI redraw;
- Custom draft immediate reflection;
- success only after persistence confirmation;
- power-loss-safe product state;
- bounded bonded reconnect with fallback;
- Logitech Forward held-state correction;
- release-triggered HAT actions;
- accepted pixel relocation and color priority.

## Evidence discipline

A compiled artifact proves compilation only. Hardware behavior such as pairing, saved reconnect, Pair New replacement, held-button cleanup, display placement and timeout transitions requires appropriate physical evidence before it can be described as validated.

There is no simultaneous-mouse capacity qualification requirement because simultaneous live mouse connections are outside the product contract.
