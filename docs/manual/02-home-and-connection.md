# Home and connection lifecycle

Mouse Bridge Remapper keeps many saved mice but allows **only one live mouse connection at a time**.

The HOME family is resolved from two facts only:

1. whether at least one mouse is saved;
2. whether one mouse is currently connected.

That produces one clean rule:

```text
no saved mouse
  -> searching-first

saved mouse exists + one connected mouse
  -> home-connected

saved mouse exists + no connected mouse
  -> home-searching + start bounded saved-device search
```

There is no multi-connected HOME state.

## HOME with a connected mouse

When one mouse is connected, HOME shows:

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

Line 2 is the connected mouse's display name. The remap summary belongs to that same mouse, so there is no profile-target ambiguity.

The summary always reflects the **confirmed** active profile. It must not switch optimistically when Apply is merely requested.

## What happens when the connected mouse is turned off

If the connected mouse powers off, leaves range, loses its BLE session or otherwise disconnects:

1. held Mouse/Escape output from that session is released safely;
2. the live connection slot becomes empty;
3. because saved mice still exist, HOME enters `home-searching`;
4. entering `home-searching` starts a bounded saved-device search automatically;
5. if any saved mouse reconnects successfully, it becomes the only connected mouse and HOME becomes `home-connected`;
6. if no saved mouse is found before the timeout, HOME becomes `home-retry` / `DEVICE NOT FOUND`.

This is the same saved-search flow used at startup. Disconnect does not have a separate reconnection subsystem with different semantics.

## Entering HOME with no current connection

Whenever navigation returns to HOME and saved mice exist but no mouse is connected, `home-searching` is entered and a saved-device search begins automatically.

Therefore the user does not need to press Retry merely because they navigated back to HOME. `KEY A: RETRY SEARCH` is needed only after a previous bounded search has already expired into `DEVICE NOT FOUND`.

## Saved-device search

While the bounded search is active:

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

The search may consider multiple saved identities, but **only the first saved mouse that successfully reaches ready state is accepted**. Search stops immediately after that success because the live connection capacity is one.

`KEY B: CANCEL SEARCH` cancels only the current search transaction. It does not remove any saved mouse.

## Device not found

If the search interval expires without a ready saved mouse, HOME becomes:

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

`KEY A: RETRY SEARCH` starts a fresh bounded saved-device search.

## Pair New replaces the live connection

`PAIR NEW MOUSE` is an explicit request to connect an unsaved mouse.

If a mouse is connected when Pair New begins:

1. stop accepting new input from the current session;
2. release any held remapped Mouse button or synthetic Escape owned by it;
3. disconnect that mouse cleanly;
4. keep its Saved Devices record and bond unless the user later removes it;
5. start the Pair New search;
6. accept at most one valid unsaved mouse;
7. when the new mouse reaches ready state, it becomes the sole connected mouse.

The previous mouse is therefore **replaced as the live connection**, not deleted.

If Pair New fails or is canceled, the previous mouse remains saved but disconnected. Pair New itself does not silently reconnect it. When the user returns to HOME with no connection, the normal `home-searching` rule starts saved search automatically.

## Why this model is intentionally single-connection

The single live slot removes unnecessary ambiguity and runtime complexity:

- HOME always refers to exactly one connected mouse;
- remapper actions always target that mouse;
- only one Saved Devices page can be cyan/CONNECTED at a time;
- no cross-mouse held-button aggregation is required;
- no simultaneous HIDS-session capacity claim is required;
- Pair New has clear replacement semantics.

Multiple **saved** mice remain fully supported.
