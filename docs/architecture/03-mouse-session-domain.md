# Mouse session domain

## Core simplification

Mouse Bridge Remapper supports many **saved** mice but exactly one **live connected mouse**.

The runtime therefore owns one optional connection slot instead of a set of simultaneous sessions:

```text
ConnectedMouseSlot = None | MouseSession
```

This is an intentional product constraint, not merely an implementation limitation.

## Identity model

Persistent identity and transient Bluetooth state remain separate:

```text
MouseId
  persistent identity of a saved mouse

MouseSessionId
  MouseId + session generation/token
```

Raw HCI/HIDS handles remain adapter-private. The generation/token prevents a late callback from an old disconnected session from mutating a newer session that reused a low-level handle.

## Saved mouse

Conceptual persistent record:

```text
SavedMouse {
  MouseId id;
  DisplayName name;
  ProfileKind profile;
  CapabilitySummary capabilities;
  VendorMetadata vendor;
}
```

There may be multiple `SavedMouse` records.

There is no Keyboard/Composite device-type union in the product domain.

## Live mouse session

Conceptual live state:

```text
MouseSession {
  MouseSessionId session_id;
  MouseId mouse_id;
  ConnectionPhase phase;
  ReportCapabilities reports;
  SecurityState security;
  HidppSessionState vendor;
}
```

At most one such session may be in `ready` state.

`connected` in UI terms means that the single live slot contains a current session that reached the product's ready state. Recent motion is never used as a connection proxy.

## Canonical input boundary

BLE adapters emit canonical events only:

```text
ButtonDown(LEFT|RIGHT|MIDDLE|FORWARD|BACKWARD)
ButtonUp(LEFT|RIGHT|MIDDLE|FORWARD|BACKWARD)
Move(dx, dy)
Wheel(delta)
Pan(delta)
SessionGone(session_id)
```

Report IDs, offsets, transport framing and raw HIDS structures are resolved before this boundary.

Malformed or truncated reports are rejected without corrupting held output state.

## Held-state model

Because only one mouse may be live, cross-device reference counting is unnecessary.

The remap/output layer still tracks explicit held state for:

- Left;
- Right;
- Middle;
- Forward;
- Backward;
- synthetic Escape.

Duplicate press/release transitions are idempotent.

On disconnect, replacement, removal, parser failure, queue continuity loss or a profile transition that invalidates existing mapping state, all held output attributable to the current session is released before the session is discarded.

## Relative events

X/Y movement, vertical wheel and horizontal pan are transient.

They are accumulated in bounded signed totals as needed for USB backpressure and split into report-sized chunks. A chunk is consumed only when the USB report is accepted for submission according to the TinyUSB contract.

No multi-mouse merging is required.

## Session replacement

There is exactly one supported live-session transition:

```text
None -> Mouse A
Mouse A -> None
Mouse A -> None -> Mouse B
```

The last form is used by Pair New replacement. Direct overlap `Mouse A + Mouse B` is forbidden.

A replacement transaction must complete release/disconnect cleanup of A before B becomes authoritative.

## Saved-state independence

Disconnecting a live mouse does not delete its `SavedMouse` record.

Therefore:

- a powered-off mouse can be rediscovered by the saved-device HOME search;
- Pair New may disconnect the current mouse while leaving it saved;
- removal is the only user action that deletes the saved product relationship and associated credential state.

## Invariant

At every externally observable instant:

```text
ready_live_mouse_count <= 1
```

Any code path that would make a second mouse ready before the first live session has been released and closed is an architecture violation.
