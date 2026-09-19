# Mouse remapping

Mouse Bridge Remapper stores a confirmed remapping profile for each saved mouse. Profiles affect button functions only. Relative X/Y movement, vertical wheel and horizontal pan continue to pass through unchanged.

## Passthrough

Passthrough preserves the original five mouse-button meanings:

| Physical source | Host output |
|---|---|
| Left | Left |
| Right | Right |
| Middle | Middle |
| Forward | Forward |
| Backward | Backward |

Passthrough is the safe/default profile for a newly saved mouse unless a later explicit product rule changes that default.

## Default / Standard Remap

The supplied UI uses both `DEFAULT REMAP` and `STANDARD REMAP` wording for the same mapping. Until the display vocabulary is normalized, they refer to one profile with this exact behavior:

| Physical source | Host output |
|---|---|
| Left | Forward |
| Right | Backward |
| Middle | Middle |
| Forward | Left |
| Backward | Right |

The dedicated active/apply pages use the Standard wording; the Mouse Options menu currently uses `DEFAULT REMAP`.

## Escape Remap

Escape is intentionally retained even though Bluetooth Keyboard pairing is outside the project.

Exact mapping:

| Physical source | Host output |
|---|---|
| Left | USB Keyboard Escape |
| Right | Mouse Backward |
| Middle | Mouse Forward |
| Forward | Mouse Left |
| Backward | Mouse Right |

Press, hold and release must be preserved. Holding the mapped physical button means Escape ownership remains held until release; disconnect, profile change or removal must never leave Escape stuck.

The USB Keyboard capability exists solely as an output sink for synthetic Escape. There is no Bluetooth Keyboard discovery, pairing, saved-device type or keyboard input path.

## Custom Remap

Custom Remap edits these five sources:

- Left
- Right
- Middle
- Forward
- Backward

Each source may become one of:

- Left
- Right
- Middle
- Escape
- Forward
- Backward

Changing a per-source `WILL BECOME` page updates the Custom draft and returns to the editor. The returned row must immediately show the new draft choice.

`KEY A: APPLY CUSTOM` applies the complete draft. The UI may show the profile as active only after the runtime mapping and required persistent write have succeeded.

The inherited product model uses one global Custom template. If several saved mice use Custom, the consequences of later editing that global template must remain explicit and tested; the firmware may not silently fork per-mouse custom tables without a documented product change.

## Applying a profile safely

A profile transition follows this semantic order:

1. user requests Apply on release;
2. validate the target profile/mapping;
3. release stale ownership produced by the old mapping for that mouse;
4. update runtime mapping and any vendor-specific behavior;
5. persist the confirmed profile state;
6. publish success to the UI.

The UI must not display a false success state if runtime or persistence fails.

## Logitech Lift / HID++

For supported Logitech mice, the product may automatically use Logitech HID++ to preserve correct Forward down/hold/up semantics when Forward is remapped.

This is an implementation backend, not a user-selectable mode. Unsupported mice must continue working through ordinary HID behavior. Returning to Passthrough must remove any no-longer-needed Forward diversion.

## Concurrent mice

Each mouse retains its own profile kind. Remapping one mouse must not release or rewrite another mouse's held output or profile state.

The UI rule for choosing a profile-editing target when more than one mouse is connected is still an explicit open product decision; see [Open product decisions](../architecture/09-open-decisions.md).
