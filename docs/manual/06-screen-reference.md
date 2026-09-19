# Canonical screen reference

This document is the user-facing screen inventory. Parenthetical annotations used during planning are not rendered. Dynamic fields are described below their screen.

Unless explicitly stated otherwise, rows must fit the 21-character semantic width.

The product may have many saved mice, but at most one mouse may be connected at any time.

## searching-first

```text
SEARCHING FIRST MOUSE
PRESS TO LEARN KEYS
WHILE WAIT CONNECTION
       JOY UP
  JOY    JOY    JOY
  LEFT  PRESS  RIGHT
      JOY DOWN
 KEY A         KEY X
 KEY B         KEY Y
```

Shown when no mouse is saved. Search continues until one valid first mouse is accepted. If several candidates wait to pair, only the first valid one is accepted and search stops.

Coordinates: `JOY UP` starts column 8; the three `JOY` tokens start columns 3/10/17; `LEFT`/`PRESS`/`RIGHT` start 3/9/16; `JOY DOWN` starts 7; `KEY A`/`KEY X` start 2/16; `KEY B`/`KEY Y` start 2/16.

## first-mouse-connected

```text
FIRST MOUSE CONNECTED
       JOY UP
  JOY    JOY    JOY
  LEFT  PRESS  RIGHT
      JOY DOWN
               KEY A
LOCK SCREEN    KEY B
 AND UNLOCK    KEY X
  OPEN HOME -> KEY Y
```

Shown only after the first mouse has been successfully connected and saved.

## home-searching

```text
SEARCHING SAVED MOUSE
 PAIR NEW MOUSE
 SAVED DEVICES
 LEARN THE KEYS

KEY B: CANCEL SEARCH
JOY UP / DOWN: SELECT
JOY PRESS: ACCESS
KEY X: HELP
```

Shown whenever HOME is entered while at least one mouse is saved and no mouse is connected. Entering this screen automatically starts a bounded saved-device search.

The first saved mouse that successfully reaches ready state becomes the sole connected mouse and the search stops.

If the previously connected mouse is powered off or otherwise disconnects, the product returns to this same HOME search flow automatically.

## home-searching-help

```text
HOME SEARCHING HELP
UNLESS IT IS CANCELED
THE SEARCH WILL TAKE
A FEW SECONDS AND
WILL BE TRIGGERED
EVERY TIME YOU ACCESS
THIS SCREEN.

ANY KEY: BACK
```

## home-retry

```text
DEVICE NOT FOUND
 PAIR NEW MOUSE
 SAVED DEVICES
 LEARN THE KEYS

KEY A: RETRY SEARCH
JOY UP / DOWN: SELECT
JOY PRESS: ACCESS
KEY X: HELP
```

Shown after a bounded saved-device search expires without any saved mouse successfully connecting.

`KEY A: RETRY SEARCH` starts a new saved-device search.

## home-retry-help

```text
HOME RETRY HELP
THE MATCHING ATTEMPT
TOOK PLACE ONLY FOR
DEVICES ALREADY SAVED
IN THE PREFERENCES,
BUT NOT FOR DEVICES
THAT WERE NOT SAVED.

ANY KEY: BACK
```

## pair-new

```text
PAIR NEW MOUSE
TRYING TO CONNECT
A NEW MOUSE THAT
IS NOT LISTED
IN SAVED DEVICES

KEY B: CANCEL
KEY X: HELP
KEY Y: LOCK
```

Entering Pair New is a live-connection replacement operation. If a mouse is connected, the product first releases its held output and disconnects it cleanly while keeping it saved. Then Pair New searches for an unsaved mouse.

If several unsaved candidates are waiting, only the first valid candidate is accepted. When successful, that mouse becomes the sole connected mouse and the search stops.

If Pair New fails or is canceled, the old mouse remains saved but is not silently reconnected inside this screen. Returning to HOME with no connection starts `home-searching` automatically.

## help-pair-new

```text
PAIR NEW DEVICE HELP
IF YOU'D LIKE TO TRY
CONNECTING A DEVICE
THAT ALREADY HAS A
SAVED DEVICE, SIMPLY
RETURN TO THE
PREVIOUS SCREEN.

ANY KEY: BACK
```

## retry-pair-new

```text
PAIR NEW MOUSE
NO NEW MOUSE OUTSIDE
THE LIST OF SAVED
DEVICES WAS FOUND

KEY A: RETRY NEW PAIR
KEY B: BACK TRY SAVED
KEY X: HELP
KEY Y: LOCK
```

Returning to the saved HOME flow with no connected mouse causes `home-searching` to start automatically.

## help-retry-pair-new

```text
DEVICE NOT FOUND HELP
THE MATCHING ATTEMPT
TOOK PLACE ONLY FOR
DEVICES NOT SAVED IN
THE PREFERENCES, BUT
NOT FOR DEVICES
ALREADY SAVED.

ANY KEY: BACK
```

## home-connected

```text
MOUSE CONNECTED
LOGITECH LIFT
 REMAPPED TO ESCAPE
 SAVED DEVICES
 LEARN THE KEYS

JOY UP / DOWN: SELECT
JOY PRESS: ACCESS
KEY X: HELP TO REMOVE
```

Line 2 is always the name of the single connected mouse. The remap summary belongs to that same mouse.

Profile summary values are currently:

- `NO REMAP PASSTHROUGH`
- `REMAPPED TO STANDARD`
- `REMAPPED TO ESCAPE`
- `REMAPPED TO CUSTOM`

There is no multi-device count form and no profile-target ambiguity because concurrent mouse connections are not allowed.

## help-home-connected

```text
HOME CONNECTED HELP
TO DISCONNECT THE
CURRENTLY CONNECTED
MOUSE, NAVIGATE TO:
SAVED DEVICES >
(MOUSE PAGE) > REMOVE
DEVICE > REMOVE

ANY KEY: BACK
```

## remapper-options

```text
MOUSE OPTIONS
 PASSTHROUGH
 DEFAULT REMAP
 ESCAPE REMAP
 CUSTOM REMAP

JOY PRESS: ACCESS
JOY LEFT: BACK
KEY X: HELP
```

All remapper actions target the single currently connected mouse.

`DEFAULT REMAP` is the menu label for the profile whose dedicated pages currently use `STANDARD REMAP`.

## help-remapper-options

```text
REMAPPER OPTIONS HELP
CHOOSE FROM THE
OPTIONS TO CHANGE THE
FUNCTIONS OF THE
MOUSE BUTTONS.
PASSTHROUGH IS THE
DEFAULT OPTION.

ANY KEY: BACK
```

## passthrough-active

```text
PASSTHROUGH ACTIVE
ORIGINAL MOUSE
BUTTONS POSITION
ARE ACTIVE NOW



KEY B: BACK
KEY Y: LOCK
```

## passthrough-not-active

```text
APPLY PASSTHROUGH
ORIGINAL MOUSE
BUTTONS POSITION
ARE NOT ACTIVE


KEY A: APPLY
KEY B: CANCEL
KEY Y: LOCK
```

## standard-not-active

```text
APPLY STANDARD REMAP
FORWARD IS LEFT
LEFT IS FORWARD
BACKWARD IS RIGHT
RIGHT IS BACKWARD

KEY A: APPLY
KEY B: CANCEL
KEY Y: LOCK
```

Middle remains Middle.

## standard-active

```text
STANDARD REMAP ACTIVE
FORWARD IS LEFT
LEFT IS FORWARD
BACKWARD IS RIGHT
RIGHT IS BACKWARD


KEY B: BACK
KEY Y: LOCK
```

## escape-not-active

```text
APPLY ESCAPE REMAP
FORWARD IS LEFT
BACKWARD IS RIGHT
LEFT IS ESCAPE
RIGHT IS BACKWARD
MIDDLE IS FORWARD

KEY A: APPLY
KEY B: CANCEL
```

## escape-active

```text
ESCAPE APPLIED ACTIVE
FORWARD IS LEFT
BACKWARD IS RIGHT
LEFT IS ESCAPE
RIGHT IS BACKWARD
MIDDLE IS FORWARD

KEY B: BACK
JOY LEFT: GO TO HOME
```

Escape is emitted through the synthetic USB Keyboard output exception; no Bluetooth Keyboard is involved.

## custom-edit

```text
EDIT CUSTOM REMAP
 LEFT IS LEFT
 RIGHT IS RIGHT
 MIDDLE IS MIDDLE
 FORWARD IS FORWARD
 BACKWARD IS BACKWARD

JOY PRESS: ACCESS
KEY A: APPLY CUSTOM
```

The five mapping rows are dynamic draft values for the single connected mouse's Custom profile selection. The Custom template itself remains global unless explicitly changed by the product contract.

## left

```text
LEFT WILL BECOME
 LEFT
 RIGHT
 MIDDLE
 ESCAPE
 FORWARD
 BACKWARD

KEY A: APPLY AND BACK
```

## right

```text
RIGHT WILL BECOME
 LEFT
 RIGHT
 MIDDLE
 ESCAPE
 FORWARD
 BACKWARD

KEY A: APPLY AND BACK
```

## middle

```text
MIDDLE WILL BECOME
 LEFT
 RIGHT
 MIDDLE
 ESCAPE
 FORWARD
 BACKWARD

KEY A: APPLY AND BACK
```

## forward

```text
FORWARD WILL BECOME
 LEFT
 RIGHT
 MIDDLE
 ESCAPE
 FORWARD
 BACKWARD

KEY A: APPLY AND BACK
```

## backward

```text
BACKWARD WILL BECOME
 LEFT
 RIGHT
 MIDDLE
 ESCAPE
 FORWARD
 BACKWARD

KEY A: APPLY AND BACK
```

## saved-devices

Example:

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

One saved mouse is shown per page. If the page represents the single currently connected mouse, **its name on line 2 is cyan**. At most one page can have a cyan connected name.

## remove-this

```text
REMOVE THIS MOUSE
LOGITECH LIFT

PAIRING AND MAPPINGS
WILL BE DELETED

KEY A: REMOVE
KEY B: CANCEL
KEY X: HELP
```

The mouse name is dynamic.

If the removed mouse is the connected mouse, its held output is released and its live session is closed before removal commits.

## help-remove-this

```text
REMOVE MOUSE HELP
COMPLETELY REMOVE THE
AUTOMATIC CONNECTION
WHEN TURNING ON THE
DEVICE AND DELETE ITS
BUTTON REMAPPING
PROFILE.

ANY KEY: BACK
```

## learn-the-keys

```text
PRESS TO LEARN KEYS
       JOY UP
  JOY    JOY    JOY
  LEFT  PRESS  RIGHT
      JOY DOWN
               KEY A
LOCK SCREEN    KEY B
 AND UNLOCK    KEY X
  OPEN HOME -> KEY Y
```

This page is never the boot root. Coordinates follow the new layout: `JOY UP` column 8; three `JOY` labels 3/10/17; `LEFT`/`PRESS`/`RIGHT` 3/9/16; `JOY DOWN` 7; right-side `KEY A/B/X` starts column 16; `LOCK SCREEN` begins column 1; `AND UNLOCK` begins column 2; `OPEN HOME -> KEY Y` begins column 3.

Opening HOME from Learn follows the same root resolver: if a mouse is connected, show `home-connected`; if saved mice exist but none is connected, enter `home-searching` and start saved search; if none are saved, enter `searching-first`.

## Screens intentionally absent

There are no product screens for:

- Pair Keyboard
- Keyboard Saved
- Pair Composite
- Composite Saved
- Other Devices

Escape output is not a reason to restore any of those screens.
