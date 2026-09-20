# Mouse session domain

Status: **FROZEN BY MBR-00**.

## Core model

Mouse Bridge Remapper may save many mice but has at most one **authoritative ready Mouse**.

```text
AuthoritativeMouseSlot = None | MouseSession
ReplacementCandidateSlot = None | CandidateSession
```

The candidate slot is used only during explicit Pair New qualification. A candidate is not product-connected/authoritative and cannot forward authoritative USB Mouse input before promotion.

## Identity

```text
MouseId
  persistent identity of a saved Mouse

MouseSessionId
  MouseId + connection generation/token
```

Raw HCI/HIDS handles stay adapter-private. Generation prevents late callbacks from old/replaced sessions mutating newer state.

## Saved Mouse

```text
SavedMouse {
  MouseId id;
  DisplayName full_name;
  ProfileKind profile;  // PASSTHROUGH | STANDARD | ESCAPE | CUSTOM
  CapabilitySummary capabilities;
  VendorMetadata vendor;
}
```

There may be multiple saved records. There is no Keyboard/Composite product-device union.

## Authoritative session

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

UI `CONNECTED` means this slot contains a current authoritative session in ready state. Recent movement is never a connection proxy.

## Replacement candidate

A Pair New candidate may have discovery/security/HIDS state sufficient for qualification. It is explicitly non-authoritative until handoff.

It must not:

- be counted as the connected Mouse;
- drive HOME connected name/profile;
- emit authoritative Mouse output;
- mutate the current Mouse's profile/registry entry.

## Canonical boundary

BLE adapter outputs canonical events only for authoritative forwarding:

```text
ButtonDown(LEFT|RIGHT|MIDDLE|FORWARD|BACKWARD)
ButtonUp(...)
Move(dx,dy)
Wheel(delta)
Pan(delta)
SessionGone(session_id)
```

Report IDs, offsets/framing and raw HIDS structures are adapter-private. Malformed/truncated reports are rejected.

## Held state

Cross-Mouse refcounting is unnecessary because only one Mouse is authoritative.

Within the authoritative session, explicit ownership/refcounts are still required for Left/Right/Middle/Forward/Backward/Escape because two physical buttons may map to the same target.

Duplicate transitions are idempotent. Disconnect, handoff, removal, parser/queue continuity loss and profile transition release invalidated held output before session disposal/change.

## Relative events

X/Y, vertical wheel and pan are transient, bounded and USB-backpressure-aware. No cross-Mouse merging exists.

## State transitions

Ordinary transitions:

```text
None -> Mouse A authoritative
Mouse A authoritative -> None
```

Pair New:

```text
Mouse A authoritative
  + Candidate B qualifying (non-authoritative)
  -> B replacement-ready
  -> freeze/release/disconnect A
  -> promote B authoritative
```

The temporary existence of candidate B is not two connected product mice. Invariant:

```text
authoritative_ready_mouse_count <= 1
```

If Pair New fails/cancels before handoff, Candidate B is discarded and Mouse A remains authoritative.

If Mouse A is manually unplugged while Pair New is active, A follows ordinary disconnect cleanup; candidate search remains new-only until it succeeds/expires/cancels.

## Saved-state independence

Disconnect/handoff never implies deletion. The old Mouse keeps its saved record/profile/bond unless the user explicitly removes it.

Removal is the only product action that deletes the saved relationship and coordinates matching credential deletion.

## MBR-05 generation boundary

Radio attempts increment a nonzero transport generation. Application promotion
allocates an independent `MouseSessionId`; the bridge retains the explicit
transport→application mapping. Only matching ready-session reports can enter
current-session held ownership. Stale transport disconnect/input cannot release
or mutate a newer session. Disconnect/overflow clears the mapping before further
messages are processed. Physical source Up releases its captured Down target,
so duplicate Down/Up is idempotent and shared targets remain held until all current
owners release.
