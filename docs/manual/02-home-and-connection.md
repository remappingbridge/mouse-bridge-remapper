# Home and connection lifecycle

Status: **FROZEN BY MBR-00**.

Mouse Bridge Remapper may keep many saved mice but has exactly one authoritative live Mouse slot.

## Unified HOME resolver

HOME depends only on saved-state and live-state:

```text
no saved Mouse
  -> searching-first

saved Mouse exists + one live Mouse
  -> home-connected

saved Mouse exists + no live Mouse
  -> home-searching + automatic saved search
```

Saved search lasts **8 seconds**. The first saved Mouse that reaches ready state wins and search stops. If none is found, HOME becomes `DEVICE NOT FOUND`.

## HOME with a connected Mouse

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

Line 2 is always the sole connected Mouse name. The remap summary belongs to that same Mouse and reflects only confirmed runtime+persistent profile state.

## Disconnect/power-off

If the current Mouse disconnects:

1. release held Mouse/Escape output from that session;
2. clear the live slot;
3. update connection truth immediately;
4. if HOME is visible, resolve HOME immediately;
5. if another page is visible, resolve HOME when HOME is next accessed.

With saved records and no live Mouse, HOME becomes `home-searching` and starts the 8-second saved search. Search expiry leads to `home-retry` / `DEVICE NOT FOUND`.

There is no separate hidden infinite reconnect state machine.

## Saved-device search

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

The search considers eligible saved identities and accepts only the first one that becomes ready. `KEY B` cancels the current search and shows `DEVICE NOT FOUND`; it never removes a saved Mouse.

## Device not found

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

`KEY A` starts another 8-second saved search.

## Pair New is replacement by handoff

`PAIR NEW MOUSE` searches for an unsaved Mouse for **15 seconds**.

If a Mouse is already connected, it remains live and usable during the Pair New search. This is intentional: merely opening Pair New must not cause needless loss of the current working connection.

An already-saved candidate is ignored for Pair New acceptance and the new-only search continues.

After an unsaved candidate has been fully qualified as replacement-ready, replacement is committed in one ordered handoff:

1. freeze new input from the old live session;
2. release its held Mouse/Escape output;
3. disconnect and clear it while keeping its saved record/bond;
4. persist/confirm the new Mouse;
5. make the new Mouse the only live/authoritative session.

If Pair New expires or is canceled before that handoff, the original live Mouse remains connected.

## Returning from Pair New to saved search

The help pages deliberately tell the user to unplug the currently connected Mouse and press Key B Back until `SEARCHING` appears when the goal is to reconnect a saved device rather than pair a new one.

`KEY B` on Pair New/retry leaves through the ordinary HOME resolver:

- current Mouse still live -> `home-connected`;
- current Mouse manually unplugged -> `home-searching` and automatic saved search.

This avoids a second special reconnect command.

## Why the model stays simple

The product has:

- many persistent saved Mouse records;
- zero or one authoritative live Mouse;
- one HOME resolver;
- one new-only Pair New transaction;
- one saved-only HOME search transaction.

There is no multi-connected HOME, cross-Mouse button aggregation, live-Mouse focus selector or simultaneous-HIDS capacity requirement.
