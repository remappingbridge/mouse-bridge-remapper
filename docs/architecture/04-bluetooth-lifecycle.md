# Bluetooth lifecycle and pairing

Status: **FROZEN BY MBR-00**.

## Transport scope

Production Mouse transport is **BLE HOGP only**. Bluetooth Classic Mouse, Bluetooth Keyboard input and Bluetooth Composite product support are out of scope.

Exactly one module owns CYW43/BTstack initialization, lifecycle and run-loop integration.

## Session identities

The product has zero or one authoritative ready `MouseSession`. A generation/token prevents late callbacks from an old/replaced session from mutating a newer session.

During Pair New, implementation may create one non-authoritative candidate transport/HIDS context for qualification if required. It cannot forward authoritative product Mouse input or be counted as the live Mouse before handoff.

## FIRST_MOUSE

Precondition: registry empty.

```text
start 8-second discovery cycle
 -> unsaved candidate
 -> authenticate/security
 -> inspect Report Map and classify Mouse
 -> persist product state
 -> promote to authoritative ready Mouse
 -> stop transaction
```

No success in the cycle -> start another 8-second cycle automatically. Logical first search is therefore continuous.

First valid accepted candidate wins.

## SEARCH_SAVED

Precondition: saved mice exist and no authoritative Mouse is live.

Entering HOME starts one **8-second** saved-search transaction. Use bond/IRK/resolving/accept-list facilities as appropriate to reconnect known peers. First saved Mouse reaching ready state wins and ends the transaction.

Timeout/cancel publishes the `home-retry` / `DEVICE NOT FOUND` state.

## Disconnect recovery

On authoritative session disconnect:

1. stop/ignore further events for the old generation;
2. release every held Mouse/Escape output belonging to it;
3. clear the authoritative live slot;
4. publish `MouseDisconnected`;
5. if HOME owns presentation, invoke HOME resolver immediately;
6. otherwise keep connection truth accurate and invoke HOME resolver when HOME is next accessed.

There is no hidden infinite reconnect loop outside HOME policy.

## PAIR_NEW

Pair New is a **15-second** new-only transaction.

If an authoritative Mouse is already live, it stays connected and usable while discovery/qualification runs.

Candidate rules:

- must classify as compatible BLE HOGP Mouse;
- must be unsaved under product identity rules;
- an already-saved candidate is ignored as a Pair New winner and search continues;
- first unsaved candidate that completes qualification wins the candidate race.

Candidate qualification ends at a non-authoritative `REPLACEMENT_READY` state. Only then is handoff committed:

1. mark old authoritative session as closing and reject new input from it;
2. release all held Mouse/Escape output from old session;
3. disconnect/clear old authoritative session while preserving its saved record/bond;
4. persist/verify new product identity/profile defaults as required;
5. promote the replacement candidate to authoritative ready state;
6. publish `MouseReady` for the new generation;
7. stop Pair New.

At all times:

```text
authoritative_ready_mouse_count <= 1
```

If Pair New expires/cancels before handoff, candidate state is cleaned up and the original authoritative Mouse remains connected.

If the user manually powers off/unplugs the current Mouse while Pair New/help is active, ordinary disconnect cleanup clears the live slot. Pair New remains new-only; it does not turn into saved search. When HOME is later reached, HOME resolver starts SEARCH_SAVED.

## Cancel

Cancel only cancels its transaction. It never deletes a saved Mouse. Candidate transport state must be cleaned so stale completion cannot later become authoritative.

## Candidate classification

Advertisement appearance is insufficient. HOGP Report Map/capabilities are inspected before Mouse acceptance. Keyboard-only HID candidates are rejected.

Duplicate Report-ID framing accepted by the G06 compatibility path is normalized inside the adapter. Malformed/truncated/incompatible frames are rejected before canonical event emission.

## Bonded reconnect

BTstack owns security credentials. Saved search reuses valid bond/IRK/LTK state; Logitech Lift and similar devices should reconnect without requiring fresh pairing mode when credentials remain valid.

The 8-second saved-search window prevents an absent peer from blocking HOME indefinitely.

## Semantic events

Examples:

```text
MouseReady(mouse_id, session_id)
MouseDisconnected(mouse_id, session_id, reason)
SavedSearchExpired(transaction_id)
PairNewCandidateReady(transaction_id, candidate_session_id)
PairNewExpired(transaction_id)
```

Session/transaction identities make stale events ignorable.

## Failure safety

Security/HIDS/parser/report failures return to coordinator policy and never trap USB/UI servicing. Disconnect, handoff, removal, profile change or queue-continuity loss releases held state before authoritative session disposal.

## MBR-05 adapter boundary

The clean candidate uses BLE-only central security (NoInputNoOutput, Secure
Connections + bonding), GAP Device Name lookup, one HIDS instance in Report
Protocol, and Report Map classification before readiness. Non-Mouse standard
application collections are rejected, including Keyboard/Composite; vendor
usage page 0xFF00 remains optional input with no HID++ dependency. Known explicit
non-Mouse HID appearances are filtered before connection.

HCI handles, HIDS CIDs, transport generations and search transaction tokens guard
callbacks. Repeated FIRST cycles and current-boot saved reconnect use the existing
8-second application deadlines. Saved reconnect uses accepted RAM identities and
the controller resolving list/whitelist. Bond identity is read from the LE device
DB after security; RPA addresses are not used as the saved product identity when
an identity is available. Credentials use SDK 2.2.0 BTstack TLV flash banks (Pico
2 W offsets 0x3FD000..0x3FEFFF, 8192 bytes). Product records remain RAM only in this
gate; durable registry and reboot reconnect qualification are MBR-06/07.

A healthy mouse remains active when Pair New is opened, but simultaneous candidate
qualification/replacement is not implemented until MBR-07. This candidate times
out that search without disconnecting the existing mouse. MBR-05 physical closure
covers fresh pair and same-boot reconnect, not full replacement or power-cycle
product-state restoration.
