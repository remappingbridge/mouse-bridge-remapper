# Multi-mouse domain and HID aggregation

## Identity model

Persistent product identity and transient Bluetooth handles must be separate.

```text
MouseId
  persistent identity of a saved mouse

MouseSessionId
  MouseId + session generation/token

MouseSourceId
  session + canonical source stream
```

Raw HCI/HIDS connection handles remain adapter-private. Late callbacks from an old session must be unable to mutate a replacement session that reuses a low-level handle.

## Saved mouse

Conceptual record:

```text
SavedMouse {
  MouseId id;
  DisplayName name;
  ProfileKind profile;
  CapabilitySummary capabilities;
  VendorMetadata vendor;
}
```

There is no Keyboard/Composite device-type union in the saved-device domain.

## Live mouse session

Conceptual session:

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

`connected` in UI terms means a current session that reached the product's ready state. Recent motion is not a connection-state proxy.

## Canonical input boundary

BLE adapters emit canonical events only:

```text
ButtonDown(source, LEFT|RIGHT|MIDDLE|FORWARD|BACKWARD)
ButtonUp(source, LEFT|RIGHT|MIDDLE|FORWARD|BACKWARD)
Move(source, dx, dy)
Wheel(source, delta)
Pan(source, delta)
SourceGone(source/session)
```

Report IDs, offsets, raw field layouts and duplicated transport framing must be resolved before this boundary.

Malformed or truncated reports are rejected without corrupting canonical held state.

## Per-source held ownership

Every host-visible mouse button is represented as a set/refcount of owning sources.

Example:

```text
owners[LEFT] = {
  mouse_A/session_5/remapped_forward,
  mouse_B/session_2/physical_left
}
```

The host-visible Left bit remains down while that set is non-empty.

Therefore:

- release from Mouse A does not release Mouse B;
- disconnect of Mouse A removes only A-owned entries;
- profile transition on Mouse A removes only stale ownership attributable to A's old mapping;
- duplicate Down/Up transitions are idempotent;
- removing one saved mouse cannot release another mouse's held button.

Synthetic Escape uses the same ownership principle in a dedicated key ownership slot so multiple sources cannot prematurely release it.

## Relative events

X/Y movement, vertical wheel and horizontal pan are transient rather than held ownership.

The aggregator accumulates bounded signed deltas from all ready sessions. USB output may split totals into representable report-sized chunks. A chunk is consumed only when the USB report has been accepted for submission according to the TinyUSB contract.

## Ordering

Within one mouse session, canonical event order must be preserved.

Across mice, the event queue uses deterministic serialization without claiming physical simultaneity. Because held ownership is source-aware, ordinary interleaving cannot create a cross-mouse release bug.

## Disconnect and fault cleanup

A source/session cleanup is mandatory on:

- ordinary disconnect;
- parser/client fatal error;
- queue overflow that invalidates state continuity;
- explicit device removal;
- relevant profile transition;
- stale-session replacement.

Cleanup is scoped to the affected source/session.

## 999 product bound

The domain representation must be able to represent counts through 999 without UI overflow. It must also reject or gracefully refuse creation of a 1000th ready mouse rather than generating invalid display state.

This is a product upper bound, not a claim that the Bluetooth stack/hardware has been validated with 999 simultaneous connections.
