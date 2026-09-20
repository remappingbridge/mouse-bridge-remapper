# Canonical screen reference

Status: **FROZEN BY MBR-00; AMENDED BY MBR-02 AND HOME-SEARCHING HELP DECISION 2026-09-20**.

This document is the executable user-facing screen inventory. Parenthetical annotations from historical planning are metadata and are never rendered. Unless explicitly stated otherwise, rows fit the 21-character semantic width.

The product may have many saved mice, but at most one mouse may be connected/authoritative at any time. Actions execute on release.

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

Shown when no mouse is saved. First-Mouse search is logically continuous, implemented as restartable 8-second cycles until one valid BLE HOGP Mouse is accepted.

All displayed HAT controls are didactic only on this screen: while held, the corresponding visible label becomes white; on release it returns to its resting color. No control navigates, locks or cancels first search.

Frozen 1-based token columns: `JOY UP` 8; three `JOY` tokens 3/10/17; `LEFT`/`PRESS`/`RIGHT` 3/9/16; `JOY DOWN` 7; `KEY A`/`KEY X` 2/16; `KEY B`/`KEY Y` 2/16.

Canonical screen ID is `searching-first`. `searching-first-mouse` is a historical alias only.

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

Shown only after the first Mouse has been authenticated, classified, persisted and made ready.

Controls on this instructional screen are frozen as follows:

- joystick directions/press and Key A: visual didactic feedback only;
- Key B release: lock the LCD presentation;
- while this screen is locked, Key X release: unlock; the interaction is consumed and remains on this screen;
- Key Y release: open HOME, which resolves to `home-connected` while the Mouse remains live.

Frozen columns follow the displayed instructional geometry: `JOY UP` 8; three `JOY` 3/10/17; `LEFT`/`PRESS`/`RIGHT` 3/9/16; `JOY DOWN` 7; right-side `KEY A`, `KEY B`, `KEY X` start at 16; `LOCK SCREEN` starts 1; `AND UNLOCK` starts 2; `OPEN HOME -> KEY Y` starts 3.

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

Shown whenever HOME is entered while at least one Mouse is saved and no Mouse is connected. Entering it starts an **8-second** saved-device search automatically.

The first saved Mouse that reaches ready state becomes the sole connected Mouse and HOME immediately becomes `home-connected`. If the search expires, HOME becomes `home-retry`.

`KEY B` cancels the current search and leaves the HOME family in `home-retry`; it does not delete a saved record.

## home-searching-help

```text
HOME SEARCHING HELP
THE MATCHING ATTEMPT
TOOK PLACE ONLY FOR
DEVICES ALREADY SAVED
IN THE PREFERENCES,
BUT NOT FOR DEVICES
THAT WERE NOT SAVED.

ANY KEY: BACK
```

This literal explains that the HOME matching attempt is saved-device-only. Any HAT control returns to `home-searching` and is consumed. The automatic saved-device search remains an 8-second `SEARCH_SAVED` transaction; Pair New remains a separate 15-second unsaved-only search.

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

Shown after saved search expires or is canceled. `KEY A` starts a new 8-second saved search and enters `home-searching`.

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

`PAIR NEW` runs for **15 seconds** and searches only for an unsaved BLE HOGP Mouse.

If one Mouse is already connected, that Mouse remains the sole authoritative live Mouse and continues forwarding input while the new candidate is discovered and qualified. A saved candidate is ignored for Pair New acceptance and the same Pair New window continues.

When one unsaved candidate has been fully qualified as replacement-ready, the product performs one atomic handoff: stop old-session input, release all held Mouse/Escape output, disconnect/clear the old live session, persist/confirm the new Mouse as required, then promote the candidate as the sole ready Mouse. There is never more than one authoritative/ready Mouse.

If Pair New is canceled or times out before handoff, the current Mouse remains connected and unchanged. If it was manually unplugged during the search, no connection is fabricated; returning to HOME invokes the normal HOME resolver.

`KEY B` cancels Pair New and returns through the HOME resolver. `KEY X` opens `help-pair-new`. `KEY Y` locks presentation without stopping the current Mouse or the Pair New transaction.

## help-pair-new

```text
PAIR NEW DEVICE HELP
TO CONNECT A SAVED
DEVICE, FIRST UNPLUG
CURRENTLY CONNECTED
MOUSE AND PRESS THE
KEY B TO BACK UNTIL
SEARCHING APPEARS.

ANY KEY: BACK
```

This text is literal. `ANY KEY: BACK` returns to `pair-new` and consumes the interaction.

The instruction is consistent with the connection model: Pair New does not select already-saved Mice. To make the normal saved-device search eligible, the user disconnects the current Mouse and navigates back until HOME resolves to `home-searching`.

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

Shown after the 15-second Pair New window expires without a new candidate. If the original Mouse stayed connected, it is still the current Mouse.

`KEY A` starts another Pair New window. `KEY B` leaves Pair New through the HOME resolver: with a current Mouse it goes to `home-connected`; after the user has unplugged the current Mouse it goes to `home-searching`, which immediately starts saved search. `KEY X` opens `help-retry-pair-new`. `KEY Y` locks presentation.

## help-retry-pair-new

```text
DEVICE NOT FOUND HELP
TO CONNECT A SAVED
DEVICE, FIRST UNPLUG
CURRENTLY CONNECTED
MOUSE AND PRESS THE
KEY B TO BACK UNTIL
SEARCHING APPEARS.

ANY KEY: BACK
```

This text is literal. `ANY KEY: BACK` returns to `retry-pair-new` and consumes the interaction.

## home-connected

Flow to this screen: `searching-first`, `home-searching` or `home-retry` -> `home-connected`. The first-Mouse instructional screen may also reach HOME through its documented Key-Y action.

```text
LOGITECH LIFT
 PAIR NEW MOUSE
 REMAPPED TO ESCAPE
 SAVED DEVICES
 LEARN THE KEYS

JOY UP / DOWN: SELECT
JOY PRESS: ACCESS
KEY X: HELP TO REMOVE
```

HOME-connected uses the first **15 renderer-supported characters** of the name
(uppercase, no ellipsis), trims trailing spaces, and appends ` MOUSE` only if the
full bounded original name does not contain the standalone word `MOUSE`, ignoring
case. Word boundaries are non-ASCII-alphanumeric/non-underscore characters.
The result fits 21 display columns. A `MOUSE` word beyond column 15 also suppresses
the suffix; truncation is never expanded to preserve a word. Empty/unusable names
remain `UNKNOWN MOUSE`. Stored names are unchanged. Examples: `LIFT` → `LIFT MOUSE`,
`MOUSE GENERIC` → `MOUSE GENERIC`, `XPTO ULTRA 2714` → `XPTO ULTRA 2714 MOUSE`
(the original has 14 characters), `ABCDEFGHIJKLMNOP` → `ABCDEFGHIJKLMNO MOUSE`.
Saved Devices/removal retain their separate first-21-supported-character policy.

All HOME options (searching/retry/connected), including the remap-summary link,
use ordinary action light gray and white selection. They never use status cyan.
The current-profile highlight inside `remapper-options` remains cyan.

The four options are ordered:

1. `PAIR NEW MOUSE` -> `pair-new`;
2. the confirmed remap summary -> `remapper-options`;
3. `SAVED DEVICES` -> `saved-devices`;
4. `LEARN THE KEYS` -> `learn-the-keys`.

`PAIR NEW MOUSE` is therefore a normal visible path from `home-connected` into `pair-new`. The current Mouse remains authoritative/usable while the new-only search runs.

Profile summary values are exactly:

- `NO REMAP PASSTHROUGH`
- `REMAPPED TO STANDARD`
- `REMAPPED TO ESCAPE`
- `REMAPPED TO CUSTOM`

If the live Mouse disconnects while this HOME is visible, HOME immediately resolves to `home-searching` and starts the 8-second saved search.

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
 STANDARD REMAP
 ESCAPE REMAP
 CUSTOM REMAP

JOY PRESS: ACCESS
JOY LEFT: BACK
KEY X: HELP
```

`STANDARD REMAP` is the canonical visible name; historical `DEFAULT REMAP` is an alias only. Current confirmed profile is cyan when not selected and white while selected. `JOY LEFT` returns one logical level to `home-connected`.

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

`KEY B` returns to `remapper-options`; `KEY Y` locks.

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

Apply may enter active state only after runtime and persistence confirmation. Cancel returns to `remapper-options`.

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

There is **no hidden Key Y lock** on this page because it is not displayed.

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

`KEY B` returns to `remapper-options`. `JOY LEFT: GO TO HOME` is an intentional new-screen exception to the inherited one-level Back rule and invokes the HOME resolver directly. There is no hidden lock control.

Escape is emitted through the fixed synthetic USB Keyboard output exception; no Bluetooth Keyboard is involved.

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

The five mapping rows show the live Custom draft. Up/Down select rows, Joy Press opens the corresponding source editor, and Key A requests full Custom apply. Full success is visible only after runtime + persistence confirmation. There is no hidden Lock control.

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

On each source editor, Up/Down selects the target with wrap. `KEY A` persists the accepted draft choice and returns to `custom-edit`, which immediately reflects it. Controls not displayed are not inherited as hidden actions.

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

One saved Mouse is shown per page. The connected Mouse name on line 2 is cyan; all other names use ordinary body color. At most one page can be connected/cyan.

Status text is exactly `STATUS: CONNECTED` for the current ready Mouse and `STATUS: DISCONNECTED` for every saved Mouse without the live session.

Profile values are `PASSTHROUGH`, `STANDARD`, `ESCAPE`, or `CUSTOM`. `JOY RIGHT/LEFT` wraps pages. `JOY PRESS` on `REMOVE DEVICE` opens `remove-this`. `KEY B` invokes HOME resolver.

Saved/removal name presentation uses the first 21 supported characters, with fallback `UNKNOWN MOUSE`. It does not apply the HOME title suffix rule.

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

If the target is live, removal stops new input, releases held output and disconnects it before the removal commit. Successful removal deletes product association and matching Bluetooth credential relationship transactionally/recoverably.

If it was the last saved Mouse, successful removal enters `searching-first` and first search. Otherwise return to a valid Saved Devices page. `KEY B` cancels back to that Saved Devices page. `KEY X` opens help.

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

This screen is never a boot root. Its instructional controls are identical to `first-mouse-connected`: joystick and Key A give visual feedback only; Key B locks; while this page is locked, Key X unlocks and is consumed; Key Y invokes HOME resolver.

Frozen columns: `JOY UP` 8; three `JOY` 3/10/17; `LEFT`/`PRESS`/`RIGHT` 3/9/16; `JOY DOWN` 7; right-side `KEY A/B/X` 16; `LOCK SCREEN` 1; `AND UNLOCK` 2; `OPEN HOME -> KEY Y` 3.

## Screens intentionally absent

There are no product screens for Pair Keyboard, Keyboard Saved, Pair Composite, Composite Saved or Other Devices. There is no multi-connected Mouse count/focus screen. Synthetic Escape is not permission to restore Bluetooth Keyboard/Composite product scope.

## 2026-09-20 background clarification

`searching-first`, `first-mouse-connected` and `learn-the-keys` fill the entire
240×240 display with the same dark magenta (`MBR_DARK_MAGENTA`, RGB565 `0x0801`),
including margins and title region; no black panel or footer-only fill.
