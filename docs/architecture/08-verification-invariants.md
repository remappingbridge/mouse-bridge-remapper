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
8. Runtime connection truth is a set of sessions, not one global active mouse.

## Pairing/search invariants

1. First-mouse search continues until one valid first mouse is accepted.
2. Every search/pair transaction accepts at most one winning new/target mouse.
3. If multiple candidates are waiting, first valid accepted candidate wins and that transaction stops.
4. Saved-device startup search is bounded.
5. As soon as one saved mouse becomes ready, automatic saved search stops.
6. With one or more mice already connected, the product does not automatically search merely to add another mouse.
7. Pair New is explicit and does not disconnect existing mice.
8. Canceling/failed Pair New preserves existing saved/connected mice.

## Multi-mouse HID invariants

1. Each mouse/session has independent source identity.
2. One mouse release cannot release another mouse's held host button.
3. One mouse disconnect cannot release another mouse's ownership.
4. Profile transition cleans only stale ownership belonging to the affected mouse/source.
5. Duplicate button transitions are idempotent.
6. Queue/parser failure is release-safe.
7. Movement/wheel/pan from several mice can coexist without converting transient deltas into held state.
8. Synthetic Escape uses source-aware ownership and cannot become stuck after disconnect/profile change/removal.

## UI invariants

1. With one connected mouse, HOME line 2 shows its name.
2. With two or more connected mice, HOME line 2 is exactly `<N> DEVICES CONNECTED`.
3. Count cannot exceed 999; `999 DEVICES CONNECTED` fits 21 columns.
4. Connected Saved Devices mouse name on line 2 is cyan.
5. Each Saved Devices page derives connection state independently.
6. Selected/pressed white overrides current/connected cyan.
7. Async connect/disconnect redraws relevant UI without requiring another HAT event.
8. Actions execute on release.
9. Help uses its own interaction ownership.
10. Removed Keyboard/Composite pages cannot reappear in the screen registry.

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
2. Custom template survives reboot.
3. Accepted unapplied Custom draft state survives reboot if the inherited behavior is retained.
4. Corrupt/torn newest product generation falls back to a previous valid generation.
5. Product-state storage cannot overwrite BLE credentials.
6. Removing a mouse removes its product association and matching credential state transactionally/recoverably.
7. Removing the last saved mouse returns to first-mouse search.

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

A compiled artifact proves compilation only. Hardware behavior such as simultaneous connections, reconnect, button hold semantics and display placement requires appropriate physical evidence before it can be described as validated.

The product maximum of 999 is a specification bound; release evidence must separately state the largest simultaneous BLE count actually validated on hardware.
