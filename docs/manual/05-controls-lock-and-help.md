# Controls, lock and help

Status: **FROZEN BY MBR-00**.

Mouse Bridge Remapper uses the Waveshare Pico-LCD-1.3 HAT joystick and keys as its local control surface.

## Release-triggered actions

Navigation, Apply, Cancel, Retry, Remove, Lock and access actions execute on **release**, not on initial press.

Where a visible control supports press feedback, its label becomes white while held and returns to its resting semantic color on release before the action is processed.

## Option lists

Where declared by the screen reference:

- `JOY UP` selects previous;
- `JOY DOWN` selects next;
- selection wraps;
- `JOY PRESS` accesses selected option.

Saved Devices uses `JOY RIGHT\LEFT: PAGE` with page wrap.

## Connected HOME options

When `home-connected` is visible, `JOY UP/DOWN` selects among four options:

1. `PAIR NEW MOUSE` — starts the 15-second new-only Pair New search;
2. the current remap summary — opens `remapper-options`;
3. `SAVED DEVICES`;
4. `LEARN THE KEYS`.

The current Mouse remains connected while Pair New is being qualified.

## Back and HOME

Key B normally means the exact Back/Cancel transition frozen for the current screen.

`JOY LEFT: GO TO HOME` on `escape-active` is an intentional exception introduced by the newer screen contract. It invokes the unified HOME resolver directly rather than one-level Back.

No other screen gains a hidden Go-To-Home shortcut by analogy.

## Help

Key X opens contextual Help only on screens that explicitly advertise it.

A Help page owns every HAT input while visible. `ANY KEY: BACK` consumes the interaction and returns to its owning screen. It cannot simultaneously perform the control's normal underlying action.

The literal Pair New Help screens are frozen in `06-screen-reference.md`.

## Ordinary Lock

Only screens that explicitly display `KEY Y: LOCK` have the ordinary Key-Y lock command. There are **no hidden lock controls** on pages where Lock is omitted.

Ordinary Lock affects presentation only. Bluetooth, the current Mouse, Pair New/search transactions, remapping and USB output continue.

The first complete HAT interaction while ordinarily locked unlocks the display and is consumed; it must not also activate a screen action.

## Instructional First Connected / Learn controls

`first-mouse-connected` and `learn-the-keys` deliberately teach controls with these special semantics:

- joystick directions, joystick press and Key A: visual feedback only;
- Key B release: lock the display;
- while that instructional page is locked, Key X release: unlock and consume the interaction;
- Key Y release: invoke HOME resolver.

These special B/X/Y meanings come from the visible instructional text and do not redefine Key B/X/Y globally.

## Searching First Mouse

`searching-first` is fully didactic while automatic first search runs. Every shown HAT label may provide pressed visual feedback, but no control navigates, locks, cancels or changes search purpose.

## Color priority

- connected/current/applied/success: cyan;
- selected or visibly pressed actionable text: white;
- ordinary options: light gray;
- static/body text: off-white yellow;
- title: magenta.

White selected/pressed state has priority over cyan. When selection moves away from an item that is still current, cyan returns.
