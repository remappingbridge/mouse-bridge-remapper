# Saved devices

`SAVED DEVICES` is the persistent per-mouse view. It shows one saved mouse per page.

The product may store multiple saved mice, but **only one mouse may be connected at a time**.

## Pagination

The title is dynamic:

```text
1 OF 1
1 OF 2
2 OF 2
3 OF 4
...
```

`JOY RIGHT\LEFT: PAGE` moves between saved-mouse pages. Pagination represents saved mice, not live connections.

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

If the mouse shown on that page is the **single currently connected mouse**, its name on line 2 is **cyan**.

All other saved-mouse names use the ordinary body-text color.

Because only one mouse can be connected, at most one Saved Devices page can have a cyan mouse name at any time.

### Status

`STATUS:` reflects that specific saved mouse:

- `CONNECTED` means it owns the current ready live session;
- every other saved mouse is not connected.

The exact disconnected display word remains part of the product vocabulary that must be frozen before final renderer acceptance.

### Profile

`PROFILE:` shows the mouse's **confirmed saved profile**, not an in-progress draft or optimistic Apply result.

Each saved mouse keeps its own confirmed profile kind. The inherited Custom mapping template remains global unless the product contract is later changed.

## Opening HOME from a disconnected state

Saved Devices does not itself create parallel connections.

Whenever the user returns to HOME and saved mice exist but none is connected, HOME enters `SEARCHING SAVED MOUSE` and automatically starts a bounded search. The first saved mouse that reaches ready state becomes the sole connected mouse.

## Remove device

Selecting `REMOVE DEVICE` opens the confirmation page for that saved mouse.

```text
REMOVE THIS MOUSE
LOGITECH LIFT

PAIRING AND MAPPINGS
WILL BE DELETED

KEY A: REMOVE
KEY B: CANCEL
KEY X: HELP
```

A successful remove operation coordinates:

- release of held output if this is the currently connected mouse;
- disconnection of its live session if connected;
- deletion of its saved product record/profile association;
- deletion of its Bluetooth security relationship where applicable;
- persistence of the new state.

If the removed mouse was the **last saved mouse**, the product returns to first-mouse onboarding and automatically begins searching for a new first mouse.

If at least one saved mouse remains, the product returns to Saved Devices on a valid remaining page. Removing a disconnected saved mouse does not disturb the current live mouse.

## Help

The Remove help screen explains that removal deletes automatic reconnection and the mouse's remapping profile association.

Removal must not be reported as complete before the product reaches its documented commit point. A partially deleted record must never be projected as a successful removal.
