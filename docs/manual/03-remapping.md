# Mouse remapping

Status: **FROZEN BY MBR-00**.

Mouse Bridge Remapper stores one confirmed profile kind for each saved Mouse. Only the single currently connected Mouse is a runtime remap target. Relative X/Y movement, vertical wheel and horizontal pan always pass through unchanged.

## Passthrough

| Physical source | Host output |
|---|---|
| Left | Left |
| Right | Right |
| Middle | Middle |
| Forward | Forward |
| Backward | Backward |

Passthrough is the safe/default profile for a newly saved Mouse.

## Standard Remap

`STANDARD` is the canonical profile name. Historical `DEFAULT REMAP` refers to the same mapping but is no longer canonical visible wording.

| Physical source | Host output |
|---|---|
| Left | Forward |
| Right | Backward |
| Middle | Middle |
| Forward | Left |
| Backward | Right |

User-visible menu/apply/active/status text uses `STANDARD` / `STANDARD REMAP`.

## Escape Remap

| Physical source | Host output |
|---|---|
| Left | USB Keyboard Escape |
| Right | Mouse Backward |
| Middle | Mouse Forward |
| Forward | Mouse Left |
| Backward | Mouse Right |

Escape is retained deliberately. Press, hold and release are preserved, and disconnect/profile change/removal/replacement cannot leave Escape stuck.

The USB Keyboard capability is output-only for synthetic Escape. It does not create Bluetooth Keyboard discovery, pairing, saved-device state or keyboard input.

## Custom Remap

Sources:

- Left
- Right
- Middle
- Forward
- Backward

Allowed targets:

- Left
- Right
- Middle
- Escape
- Forward
- Backward

`KEY A: APPLY AND BACK` on a source editor updates and persists the Custom draft and immediately changes the row shown on return to `EDIT CUSTOM REMAP`.

`KEY A: APPLY CUSTOM` requests the full Custom profile. The UI may claim it active only after runtime mapping and persistent confirmation succeed.

The inherited G06 product model keeps one persistent **global Custom template**. Each saved Mouse stores its own profile kind; a Mouse whose profile kind is Custom uses that global template when it becomes the live Mouse. Changing the global Custom template therefore changes the template that all saved Custom-profile mice will use on their next/current activation. Per-Mouse private Custom tables are not part of the product contract.

## Safe profile transition

1. receive Apply on release;
2. validate mapping;
3. release stale held output created by the old mapping;
4. update runtime mapping and any required HID++ state;
5. persist and verify the confirmed profile state;
6. publish profile confirmation;
7. only then show active/applied success.

No optimistic success is allowed.

## Logitech HID++

For supported Logitech mice, the product may automatically use HID++ `REPROG_CONTROLS_V4` behavior to preserve correct Forward down/hold/up semantics when Forward is remapped.

This is an automatic backend, not a user-selectable mode. Unsupported/non-Logitech mice must continue through ordinary HOGP safely. Returning to Passthrough removes no-longer-needed diversion.

## Single live Mouse rule

Multiple Mouse records may be saved, but only one Mouse is authoritative/connected at a time. Remapper screens always target that Mouse, so there is no focus or multi-Mouse profile-target decision.
