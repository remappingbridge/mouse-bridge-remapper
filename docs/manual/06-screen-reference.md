# Canonical screen reference

This document is the user-facing screen inventory. Parenthetical annotations used during planning are not rendered. Dynamic fields are described below their screen.

Unless explicitly stated otherwise, rows must fit the 21-character semantic width.

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

Shown on startup when saved mice exist but none is ready yet and a bounded saved search is active. Once one saved mouse connects, automatic search stops; the product does not automatically seek a second mouse.

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

Shown after a saved-device search expires without a successful connection.

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

Existing connected mice keep working. If several unsaved candidates are waiting, only the first valid candidate is accepted; the Pair New search then stops.

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

Exactly one connected mouse:

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

Two or more connected mice replace line 2 with the count:

```text
MOUSE CONNECTED
2 DEVICES CONNECTED
 REMAPPED TO ESCAPE
 SAVED DEVICES
 LEARN THE KEYS

JOY UP / DOWN: SELECT
JOY PRESS: ACCESS
KEY X: HELP TO REMOVE
```

Count format is `<N> DEVICES CONNECTED`, from 2 through 999. `999 DEVICES CONNECTED` is the maximum 21-character form.

Profile summary values are currently:

- `NO REMAP PASSTHROUGH`
- `REMAPPED TO STANDARD`
- `REMAPPED TO ESCAPE`
- `REMAPPED TO CUSTOM`

Which individual mouse is the profile-editing target when more than one mouse is connected remains an explicit product decision; implementation must not guess.

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

The wording of `CURRENTLY CONNECTED MOUSE` requires normalization for the multi-mouse state before final renderer acceptance.

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

`MIDDLE IS FORWARD` normalizes the planning typo `FORWARED` without changing behavior.

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

The five mapping rows are dynamic draft values.

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

One mouse is shown per page. If the mouse on the page is currently connected, **its name on line 2 is cyan**. Several pages may independently have cyan names when several mice are connected.

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

## Screens intentionally absent

There are no product screens for:

- Pair Keyboard
- Keyboard Saved
- Pair Composite
- Composite Saved
- Other Devices

Escape output is not a reason to restore any of those screens.
