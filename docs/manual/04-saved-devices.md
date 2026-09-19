# Saved devices

Status: **FROZEN BY MBR-00**.

`SAVED DEVICES` shows one saved Mouse per page. Multiple mice may be saved; only one may be authoritative/connected.

## Pagination

Title format is `N OF M`, for example `1 OF 1`, `2 OF 4`. `JOY RIGHT\LEFT: PAGE` wraps through saved-Mouse pages.

## Canonical page

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

## Name

The connected Mouse name is cyan. Every disconnected saved Mouse name uses ordinary body color. At most one page is cyan.

Store the full normalized available name within schema limits. Render the first 21 renderer-supported characters with no ellipsis or scrolling. If no usable name exists, render `UNKNOWN MOUSE`.

## Status

Exact values:

- current authoritative Mouse: `STATUS: CONNECTED`;
- all other saved mice: `STATUS: DISCONNECTED`.

## Profile

Exact profile values:

- `PROFILE: PASSTHROUGH`
- `PROFILE: STANDARD`
- `PROFILE: ESCAPE`
- `PROFILE: CUSTOM`

They reflect confirmed saved profile state, never an optimistic Apply request.

## Back to HOME

`KEY B` invokes the unified HOME resolver. If a live Mouse exists, HOME is `home-connected`; if saved mice exist but no live Mouse, HOME becomes `home-searching` and starts the 8-second saved search.

## Remove device

```text
REMOVE THIS MOUSE
LOGITECH LIFT

PAIRING AND MAPPINGS
WILL BE DELETED

KEY A: REMOVE
KEY B: CANCEL
KEY X: HELP
```

Successful removal coordinates product state and Bluetooth credentials. If the target is live, stop new input, release held output and disconnect/clear it before removal commits.

- last saved Mouse removed -> `searching-first` + first search;
- saved mice remain -> return to a valid Saved Devices page;
- removing a disconnected saved Mouse does not disturb the current live Mouse.

`KEY B` cancels to the saved-device page. `KEY X` opens contextual Help.

Removal cannot be shown as successful before its documented commit point.
