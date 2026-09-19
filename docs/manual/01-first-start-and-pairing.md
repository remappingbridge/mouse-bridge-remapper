# First start and pairing

Status: **FROZEN BY MBR-00**.

This chapter defines how Mouse Bridge Remapper discovers, reconnects and replaces its single authoritative Mouse connection.

## First mouse

When the product starts with **no saved Mouse**, the first screen is `SEARCHING FIRST MOUSE`.

The product immediately searches for a valid BLE HOGP Mouse. The first-Mouse search is logically continuous: it uses restartable **8-second discovery cycles** until one Mouse is successfully authenticated, classified as a Mouse, persisted and made ready.

If several unsaved mice are waiting, only the first valid candidate accepted becomes the first Mouse. The transaction then stops.

`SEARCHING FIRST MOUSE` is didactic while search runs. Its HAT labels provide visual press/release feedback only; they do not navigate, cancel or lock.

## First successful connection

After the first Mouse is persisted and ready, the product shows `FIRST MOUSE CONNECTED` before ordinary HOME use.

This screen must not appear merely because an advertisement was seen or a connection handle was allocated.

On this instructional screen, Key B locks the display, Key X unlocks it while locked, and Key Y opens HOME. Other displayed controls are didactic visual feedback only. Actions happen on release.

## Startup and HOME when saved mice exist

The same HOME resolver is used at startup and later navigation:

```text
no saved mice
  -> searching-first

saved mice + one live Mouse
  -> home-connected

saved mice + no live Mouse
  -> home-searching + automatic 8-second saved-device search
```

The first saved Mouse that reaches ready state becomes the sole live Mouse and saved search stops. If none is found before timeout, HOME becomes `DEVICE NOT FOUND`.

## Reconnection after the connected Mouse is turned off

If the live Mouse is powered off, leaves range or otherwise disconnects, held Mouse/Escape output from that session is released and the live slot becomes empty.

If HOME is visible, the HOME resolver immediately enters `SEARCHING SAVED MOUSE` and starts the bounded saved search. If the disconnect occurs on another screen, connection truth still updates immediately; the next access to HOME resolves from the now-empty live slot and starts the same saved search.

If no saved Mouse reconnects during the 8-second window, the flow ends at `DEVICE NOT FOUND`.

## Pair New

`PAIR NEW MOUSE` searches for one **unsaved** BLE HOGP Mouse for **15 seconds**.

Pair New is a replacement operation, but it does **not** disconnect the current Mouse merely because search began. If a Mouse is already live, it remains connected, authoritative and usable while Pair New discovers and qualifies an unsaved replacement candidate.

A Mouse already present in Saved Devices is not a valid Pair New winner. Such a candidate is ignored for Pair New acceptance and the same 15-second new-only search continues.

When an unsaved candidate is fully qualified as replacement-ready, the product performs one controlled handoff:

1. stop accepting new input from the old live session;
2. release all held Mouse-button and synthetic-Escape output belonging to it;
3. disconnect and clear the old live session while preserving its saved record and bond;
4. persist/confirm the new Mouse as required;
5. promote the new candidate as the **only** authoritative ready Mouse;
6. stop Pair New.

At no point are two product-ready/authoritative mice allowed.

If Pair New expires or is canceled **before handoff**, the existing live Mouse remains connected and unchanged. No saved record is deleted.

## Connecting a saved Mouse instead of a new one

Pair New never changes purpose into saved-device reconnect.

The Pair New Help text instructs the user to turn off/unplug the currently connected Mouse and press Back until HOME reaches `SEARCHING SAVED MOUSE`. This works because HOME with saved mice and no live connection automatically starts the normal saved-device search.

Thus:

- Pair New = search for an unsaved Mouse;
- HOME search = reconnect a saved Mouse.

## One winner and one authoritative connection

Every discovery transaction accepts at most one winner. Product-visible runtime always satisfies:

```text
ready_authoritative_mouse_count <= 1
```

A replacement candidate may have transient transport state while it is being qualified, but it cannot become the product's live/authoritative Mouse until the previous live session has completed the handoff cleanup.
