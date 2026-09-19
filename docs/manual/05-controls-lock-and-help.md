# Controls, lock and help

Mouse Bridge Remapper uses the Waveshare Pico-LCD-1.3 HAT joystick and keys as its local control surface.

## Release-triggered actions

Navigation, Apply, Cancel, Retry, Remove, Lock and access actions execute on **release**, not on the initial physical press.

Where the screen exposes pressed-state feedback, pressing a visible control makes that label white while held. Releasing restores its semantic resting color and then performs the action.

This rule prevents accidental double execution and is inherited from the physically accepted BLU2USB interaction behavior.

## Joystick

Where an option list is present:

- `JOY UP` selects the previous option;
- `JOY DOWN` selects the next option;
- selection wraps when the screen contract declares list navigation;
- `JOY PRESS` accesses the selected option.

Saved Devices uses `JOY RIGHT\LEFT: PAGE` to move through mouse pages.

Some profile pages use `JOY LEFT` as Back. The Escape-active page currently declares `JOY LEFT: GO TO HOME`; that literal exception remains part of the current screen specification until the product vocabulary/navigation contract is explicitly normalized.

## Key B

Most screens use Key B as Back or Cancel. Cancel means leaving the pending operation without committing it.

The implementation must use the canonical screen transition table rather than infer behavior from the English word alone.

## Key X and Help

Contextual Help is generally entered using Key X on screens that advertise it.

A Help page owns the interaction while visible. Its footer is:

```text
ANY KEY: BACK
```

Any HAT control returns from Help instead of performing its normal underlying action.

## Key Y and Lock

Screens that explicitly expose `KEY Y: LOCK` may lock the display on Key Y release.

Lock is a presentation feature only:

- Bluetooth stays active;
- all connected mice continue forwarding input;
- remapping remains active;
- reconnect/session bookkeeping continues;
- USB output remains active.

The first complete HAT interaction used to unlock is consumed by the unlock operation and must not also perform a navigation/action event.

The new screen set does not declare Lock on every page. The implementation must follow the per-screen canonical contract and must not restore old hidden Lock controls merely because BLU2USB once had them.

## Learn The Keys

`PRESS TO LEARN KEYS` is a didactic page. Other than explicitly documented lock/home behavior, its HAT controls demonstrate where the controls are by changing their visual state while pressed rather than performing normal menu navigation.

It is never the first screen shown at startup. First startup uses `SEARCHING FIRST MOUSE` instead.

## Searching First Mouse controls

The first-mouse search page is also didactic. Its displayed key/joystick labels react visually to presses but do not navigate away or trigger unrelated actions while the first-pair search is running.

## Selection and colors

The inherited color priority is:

- connected/current/applied positive state: cyan;
- selected or visibly pressed actionable text: white;
- **white selection/press has priority over cyan**;
- when selection moves away from a still-current cyan item, it returns to cyan.

This same priority applies to connected Saved Devices names if a future screen makes such a row selectable.
