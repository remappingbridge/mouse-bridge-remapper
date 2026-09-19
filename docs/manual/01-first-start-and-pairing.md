# First start and pairing

This chapter defines how Mouse Bridge Remapper discovers, reconnects and replaces its single live mouse connection.

## First mouse

When the product starts with **no saved mouse**, the first screen is `SEARCHING FIRST MOUSE`.

The product immediately begins searching for a valid BLE HOGP mouse. The first-mouse search is logically continuous: internally it may use finite scan/connect cycles, but it keeps restarting them until one mouse is successfully found, authenticated, classified as a mouse, saved and ready.

If several unsaved mice are waiting to pair, **only the first valid mouse found is paired**. As soon as that mouse is accepted, the search ends.

The `SEARCHING FIRST MOUSE` screen is didactic while search runs. HAT controls shown on the screen only provide their declared pressed/released visual feedback; they do not navigate away from the search.

## First successful connection

After the first mouse is successfully persisted and ready, the product shows `FIRST MOUSE CONNECTED` before normal HOME use.

This success screen must not be shown merely because an advertisement was seen or a connection handle was allocated. The mouse must have completed the accepted connection/classification/persistence path.

## Startup when saved mice exist

When one or more mice are already saved, startup resolves HOME as follows:

- if one saved mouse becomes connected, HOME becomes `MOUSE CONNECTED`;
- while no mouse is connected, HOME enters `SEARCHING SAVED MOUSE` and automatically starts a bounded search;
- the first saved mouse that successfully reaches ready state wins and the search stops;
- if none is found before timeout, HOME becomes `DEVICE NOT FOUND`.

Only one mouse may be connected at a time.

## Reconnection after the connected mouse is turned off

If the live mouse is powered off, leaves range or otherwise disconnects, the product clears that live session and returns to the same HOME rule used at startup.

Because saved mice still exist and none is connected, HOME automatically enters `SEARCHING SAVED MOUSE` and starts a bounded saved-device search. If no saved device is found, the flow ends at `DEVICE NOT FOUND` after the configured search interval.

## Pairing a new mouse

Use `PAIR NEW MOUSE` to add an unsaved mouse.

Pair New is also a **live-connection replacement** operation.

If a mouse is currently connected when Pair New begins:

1. its held Mouse/Escape output is released;
2. its BLE session is disconnected cleanly;
3. its Saved Devices record remains intact;
4. the product starts searching for an unsaved mouse;
5. the first valid unsaved mouse accepted becomes the sole connected mouse.

If several unsaved mice are waiting, only the first valid candidate is accepted and the Pair New search stops.

If Pair New fails or is canceled, no saved record is deleted. The previously connected mouse remains saved but is no longer live. Returning to HOME with no connection starts the normal saved-device search automatically.

## Saved search vs Pair New

The product intentionally distinguishes two transaction purposes:

- **saved-device search** tries to reconnect any eligible mouse already known to the product;
- **Pair New** tries to add one mouse that is not already saved.

A transaction never silently changes purpose.

## One result and one live connection

Every discovery transaction has a single-winner rule:

1. discover candidates;
2. validate/classify them;
3. accept the first candidate satisfying the transaction;
4. finish persistence/connection;
5. stop the transaction.

And at all times:

> at most one mouse may be in the ready/connected state.
