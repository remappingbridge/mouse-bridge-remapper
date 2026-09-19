# Bluetooth lifecycle and pairing

## Transport scope

The specified Mouse transport is BLE HOGP, inherited from the accepted BLU2USB G06 mouse path.

Bluetooth Classic Keyboard and BLE/Classic Keyboard product paths are absent. Bluetooth Composite is not a product device type.

## Single BT runtime owner

Exactly one module owns CYW43/BTstack initialization, lifecycle and run-loop integration.

Only one Mouse HOGP session may be ready at a time. Session state includes connection/security phase, HIDS client context, Report Map, parsed mouse fields, subscriptions, identity correlation, timers/retries and optional HID++ state.

A session generation/token is retained so late callbacks from an old disconnected session cannot affect its replacement.

## Transaction purposes

Discovery is governed by explicit transaction purpose.

### First mouse

Precondition: no saved mouse exists.

```text
finite discovery cycle
 -> candidate
 -> authenticate
 -> inspect/classify Mouse capability
 -> persist product identity/state
 -> reach ready session
 -> stop first-search transaction
```

If no candidate succeeds, start another finite cycle. Logical first search therefore continues until a first mouse is accepted.

If several valid unsaved candidates are waiting, the first accepted candidate wins. Later candidates from the same transaction are canceled/ignored.

### Saved search

Precondition: saved mice exist and no mouse is currently connected.

Entering HOME under that condition automatically starts a bounded saved-device search.

The coordinator may try eligible saved identities using bond/resolving/accept-list facilities as appropriate. The first saved mouse that reaches ready state wins, occupies the single live slot and ends the search.

If the bounded window expires with no ready mouse, publish `home-retry` / `DEVICE NOT FOUND`.

### Disconnect recovery

When the current mouse disconnects unexpectedly:

1. release all held Mouse/Escape output for that session;
2. clear the live slot;
3. publish `MouseDisconnected`;
4. resolve HOME;
5. if saved mice remain, enter `home-searching` and automatically start the bounded saved-device search.

There is no separate infinite reconnect loop hidden behind `home-connected`.

### Pair New

Pair New is manually initiated and accepts only an unsaved valid Mouse.

If a mouse is currently connected, Pair New first performs a replacement teardown:

1. stop accepting new events from the current session;
2. release held output;
3. disconnect the current HOGP session;
4. clear the live slot;
5. keep the mouse saved and keep its bond;
6. begin new-only discovery.

The first valid unsaved mouse that completes acceptance wins, is persisted, becomes the sole ready session and ends Pair New.

If Pair New times out or is canceled, the previous mouse remains saved but disconnected. Pair New does not silently reconnect it. Returning to HOME with no connection starts the normal saved-device search.

### Cancel

Cancel affects only the current search/pair transaction. It never deletes a saved mouse.

Any candidate session owned by the canceled transaction must be cleaned up so it cannot become ready later through a stale completion.

## Candidate classification

Advertisement appearance alone is insufficient. The HOGP path obtains and inspects Report Map/capabilities before accepting a peer as Mouse.

Keyboard-only HID candidates are rejected.

Transport framing is normalized inside the adapter. Duplicated Report-ID framing, malformed lengths and incompatible field layouts are rejected before canonical event emission.

## Bonded reconnect

BTstack remains owner of BLE security credentials. Existing bond/IRK/LTK state should be reused for saved reconnection so devices such as Logitech Lift can reconnect after Pico power loss without fresh pairing mode.

Saved search is bounded. An absent peer cannot trap HOME forever.

## Asynchronous UI events

Bluetooth publishes semantic events such as:

```text
MouseReady(mouse_id, session_id)
MouseDisconnected(mouse_id, session_id, reason)
SearchExpired(transaction_id)
PairNewExpired(transaction_id)
```

Transaction/session IDs prevent late completion from an old or canceled operation from mutating current UI/runtime state.

## Failure safety

Connection/security/HIDS/report failures return to coordinator policy rather than blocking the product loop.

Disconnect, replacement, removal, parser failure or queue continuity loss while a button/Escape is held must release that held state before the session is discarded.

## Hard invariant

A second candidate may be discovering or connecting transiently only if it cannot become authoritative or ready while the current live session still exists. Product-visible ready state always satisfies:

```text
ready_mouse_count <= 1
```
