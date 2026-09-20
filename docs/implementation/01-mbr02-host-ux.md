# MBR-02 host-pure UX model

Status: implemented by gate `mbr-02`.

This layer turns the frozen MBR-00 screen and interaction contract into executable host-pure state. It deliberately stops before physical rendering, HAT GPIO, USB HID, Bluetooth, persistence and HID++ integration.

## Ownership

- `domain` owns shared screen/control/profile/search identifiers and frozen policy constants.
- `mouse_registry` owns the pure saved-Mouse record view used by UX tests.
- `mouse_session` retains the single authoritative live slot.
- `profiles` owns canonical profile vocabulary and the global Custom draft.
- `pairing_coordinator` owns semantic search transaction IDs, purposes, durations and the non-authoritative Pair New candidate.
- `interaction` owns press/release and presentation lock state.
- `ui_projector` owns the semantic screen model: exact rows, dynamic fields, selection/current colors and didactic press spans.
- `app` orchestrates the pure UX model and emits semantic effects. It does not call Bluetooth, storage, USB or hardware APIs.

## Semantic effects

The host-pure UX may request only semantic effects:

- start/cancel FIRST_MOUSE, SEARCH_SAVED or PAIR_NEW;
- apply a confirmed-profile candidate;
- save one Custom draft source target;
- remove one saved Mouse.

Later gates provide the runtime/storage implementations and feed confirmations/failures back into this model. UI success is therefore never optimistic.

## Search and stale-event rules

Frozen durations are compile-time policy values:

- FIRST_MOUSE: 8000 ms, restarted after expiry while registry is empty;
- SEARCH_SAVED: 8000 ms;
- PAIR_NEW: 15000 ms.

Every search has a transaction ID. Pair New candidates additionally carry transaction and session identity. Expiry/candidate events with stale IDs are ignored. Mouse disconnect events only affect the current authoritative session when the exact session generation matches.

## Pair New projection

A current authoritative Mouse and one replacement candidate may coexist in state, but the candidate is explicitly non-authoritative. Candidate qualification never increases `mbr_mouse_session_ready_count()` above one. Authority moves only through the handoff-committed semantic event.

Saved candidates cannot win PAIR_NEW. Timeout/cancel before handoff preserves the current authoritative Mouse. If that Mouse disconnects while Pair New owns presentation, connection truth changes without stealing the Pair New page; the next HOME resolution starts saved search.

## Connected HOME amendment

The MBR-02 UX amendment makes `home-connected` a direct visible entry point to Pair New. The connected Mouse name is projected as the title. The four options are Pair New, current remap summary, Saved Devices and Learn the Keys. Selection 0 starts the existing 15-second Pair New transaction without changing the current authoritative session.

## Screen model

`mbr_screen_id_t` contains exactly the 30 canonical MBR screens from `docs/manual/06-screen-reference.md`. There are no Keyboard, Composite, Other Devices or multi-connected states.

The projector preserves:

- exact canonical row literals;
- 21-character semantic width;
- first-21-supported-character Mouse names with `UNKNOWN MOUSE` fallback;
- canonical `STANDARD` profile vocabulary;
- connected/current/applied cyan;
- selected/visibly pressed white overriding cyan;
- exact Saved Devices status text;
- exact Pair New Help text;
- frozen didactic token columns;
- immediate Custom-draft reprojection;
- the intentional Escape `JOY LEFT: GO TO HOME` shortcut.

## Interaction model

Actions execute only on a matching release after press. Help consumes the complete interaction. Ordinary Lock exists only on screens that visibly advertise it; the first complete interaction while locked unlocks and is consumed. `searching-first` is fully didactic. `first-mouse-connected` and `learn-the-keys` retain their special instructional B/X/Y semantics.

## Verification

Host acceptance is split into:

- `ux_golden_contract`: every canonical screen, exact rows, width, name policy, removed-state inventory, didactic columns and color priority;
- `ux_behavior_contract`: release semantics, Help/Lock consumption, HOME/search timing, stale IDs, Pair New candidate/handoff/timeout behavior, profile confirmation, Escape exception, Custom draft and removal confirmation;
- predecessor `bootstrap_contract`;
- predecessor `architecture_contract`.

The Pico build remains a structural predecessor regression only. MBR-02 makes no claim that the produced UF2 renders these screens or performs real Bluetooth/USB behavior.
