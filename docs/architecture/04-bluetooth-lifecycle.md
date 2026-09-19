# Bluetooth lifecycle and pairing

## Transport scope

The specified Mouse transport is BLE HOGP, inherited from the accepted BLU2USB G06 mouse path.

Bluetooth Classic Keyboard and BLE/Classic Keyboard product paths are absent. Bluetooth Composite is not a product device type.

## Single BT runtime owner

Exactly one module owns CYW43/BTstack initialization, lifecycle and run-loop integration.

Per-mouse HOGP state must be instance/session scoped:

- connection/security phase;
- HIDS client context;
- Report Map;
- parsed mouse fields;
- subscribed report state;
- candidate/identity correlation;
- timers/retries;
- optional vendor/HID++ state.

A mutable singleton parser/client representing “the mouse” is prohibited.

## Transaction purposes

Discovery is governed by explicit transaction purpose.

### First mouse

Precondition: no saved mouse exists.

```text
start finite discovery cycle
 -> candidate
 -> authenticate
 -> inspect/classify Mouse capability
 -> persist product identity/state
 -> reach ready session
 -> stop first-search transaction
```

If no candidate succeeds, restart another finite cycle. Logical first search therefore continues indefinitely without requiring an unbounded BTstack call.

If several valid unsaved candidates are waiting, the first candidate that completes acceptance wins. Later candidates from the same transaction are ignored/canceled and are not persisted.

### Saved search/reconnect

Precondition: at least one saved mouse exists and no qualifying connected mouse has already ended the automatic search.

The product starts a bounded search/reconnect attempt. The coordinator may use BTstack credential/resolving/accept-list mechanisms as appropriate to reconnect known peers.

**First successful ready saved mouse ends the automatic transaction.** The coordinator must not continue automatically connecting additional saved mice after that success.

If the bounded window expires with none ready, publish the retry state.

### Pair New

Pair New is manually initiated and runs while existing live mice remain operational.

Candidate must:

- classify as a valid Mouse;
- satisfy security/pairing requirements;
- not already be accepted as a Saved Devices entry under the product identity rules.

First successful new mouse wins, is persisted and becomes ready; then the Pair New transaction stops. Additional waiting mice require another explicit Pair New action.

### Cancel

Cancel affects only the current transaction. It does not remove saved mice and does not disconnect already-ready mice unless the canceled transaction owns a not-yet-committed candidate session that needs cleanup.

## No automatic second mouse

After any mouse is ready, no background policy may start discovery solely to fill more connection slots.

Reconnect after an unexpected disconnect may target that previously connected/saved mouse according to the documented reconnect policy, but must not turn into generic batch connection of unrelated additional saved mice.

## Candidate classification

Advertisement appearance alone is insufficient. The HOGP path obtains and inspects the Report Map/capabilities before accepting a peer as a mouse.

Keyboard-only HID candidates are not accepted as Mouse.

Transport framing is normalized inside the adapter. Duplicated Report-ID framing, malformed lengths and incompatible field layouts are rejected before canonical event emission.

## Bonded reconnect

BTstack remains owner of BLE security credentials. Existing bond/IRK/LTK state should be reused for saved reconnection so devices such as Logitech Lift can reconnect after Pico power loss without fresh pairing mode.

A reconnect attempt must be bounded. An absent peer cannot trap HOME forever.

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

Disconnect while a button or synthetic Escape is held triggers source-scoped release cleanup.
