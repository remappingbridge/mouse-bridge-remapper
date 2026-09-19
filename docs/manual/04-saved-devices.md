# Saved devices

`SAVED DEVICES` is the persistent per-mouse view. It shows one saved mouse per page.

## Pagination

The title is dynamic:

```text
1 OF 1
1 OF 2
2 OF 2
3 OF 4
...
```

`JOY RIGHT\LEFT: PAGE` moves between mouse pages. Pagination represents saved mice, not only currently connected mice.

## Page contents

Canonical structure:

```text
3 OF 4
LOGITECH LIFT
STATUS: CONNECTED
PROFILE: STANDARD
 REMOVE DEVICE

JOY RIGHT\LEFT: PAGE
JOY PRESS: ACCESS
KEY B: BACK
```

Line 2 is the saved mouse's display name.

### Connected-name color

If that saved mouse is **currently connected**, its name on line 2 is **cyan**.

If that saved mouse is not currently connected, the name uses the ordinary body-text color.

Connection coloring is independently derived for each saved mouse. With several connected mice, several Saved Devices pages may correctly show cyan names at the same time.

### Status

`STATUS:` reflects the current runtime state of that specific saved mouse, not a global “active mouse” flag.

At minimum:

- `CONNECTED` means a current ready session exists for that saved mouse;
- a saved mouse with no ready live session must not be falsely labeled connected.

The exact disconnected display word remains part of the product vocabulary that must be frozen before renderer implementation.

### Profile

`PROFILE:` shows the mouse's **confirmed saved profile**, not an in-progress draft or optimistic Apply result.

The profile identity is per saved mouse even though the inherited Custom mapping template is global.

## Remove device

Selecting `REMOVE DEVICE` opens the confirmation page for that mouse.

```text
REMOVE THIS MOUSE
LOGITECH LIFT

PAIRING AND MAPPINGS
WILL BE DELETED

KEY A: REMOVE
KEY B: CANCEL
KEY X: HELP
```

Removal means more than hiding the row. A successful remove operation coordinates:

- release of any held output owned by that mouse;
- disconnection of its current session, if connected;
- deletion of its saved product record/profile association;
- deletion of its Bluetooth security relationship where applicable;
- persistence of the new state.

Other connected mice keep working throughout removal except for any bounded shared-runtime interruption that the architecture explicitly proves safe.

If the removed mouse was the **last saved mouse**, the product returns to first-mouse onboarding and automatically begins searching for a new first mouse.

If at least one saved mouse remains, the product returns to Saved Devices on a valid remaining page.

## Help

The Remove help screen explains that removal deletes automatic reconnection and the mouse's remapping profile association.

Removal must not be reported as complete before the product has reached its documented commit point. A partially deleted record must never be projected as a successful removal.
