# Persistence and removal

Status: **FROZEN BY MBR-00**.

## Ownership domains

Persistent state has separate ownership domains:

1. **Bluetooth credentials** — bond/IRK/LTK/security data owned by BTstack/platform facilities.
2. **Product state** — saved Mouse identities/names, confirmed profiles, Custom state and capability/vendor metadata owned by Mouse Bridge Remapper.

Product writes must not overwrite credential storage.

## Product record

```text
header {
  schema_version
  generation
  payload_length
  integrity_check
}

saved_mice[] {
  MouseId
  full_normalized_name
  confirmed_profile
  capability_metadata
  vendor_metadata_if_needed
}

global_custom_template
custom_draft
custom_draft_dirty
```

Transient connection handles, HIDS pointers, candidate state and queue state are never persisted as product truth.

## Power-loss safety

Use two alternating generations/slots or equivalently proven strategy:

1. serialize complete new generation;
2. write non-current slot;
3. verify integrity/readback as required;
4. make it current only after successful verification;
5. boot chooses newest valid generation;
6. corrupt/torn newest falls back to previous valid generation.

No valid record -> safe defaults / empty registry.

## Boot reconstruction

Before authoritative input, restore:

- saved registry;
- each saved Mouse's confirmed profile;
- global Custom template;
- dirty/unapplied Custom draft;
- UI profile data;
- vendor metadata needed for HID++ behavior.

Then HOME resolver determines first search vs saved search.

## Saving the first/new Mouse

A candidate is not reported as saved merely because transport connected. Product identity/state must be successfully persisted at the documented transaction point.

FIRST_MOUSE has no old live session, so accepted candidate may be persisted then promoted.

PAIR_NEW differs because a healthy old Mouse may remain authoritative while candidate is qualified.

## Pair New persistence and handoff

For Pair New:

1. qualify unsaved candidate without mutating old Mouse's saved record/bond;
2. candidate reaches non-authoritative replacement-ready state;
3. freeze/release/disconnect old authoritative session while retaining its saved state/credentials;
4. persist/verify new saved Mouse state;
5. promote new candidate authoritative;
6. publish success.

If persistence fails during handoff after old session has been closed, the product must not falsely publish the candidate as connected/saved. It enters a defined recoverable no-live state; old saved record/bond remains eligible for HOME saved search.

If Pair New expires/cancels before handoff, old Mouse remains live and no product-state mutation is required for a rejected candidate.

## Profile persistence

Apply is complete only after confirmed profile state is persisted successfully.

Custom draft persists independently from last active profile. An accepted dirty/unapplied draft may survive reboot without becoming active Custom.

## Remove transaction

For target `MouseId`:

1. if target is authoritative live Mouse, stop new events;
2. release its held Mouse/Escape output;
3. disconnect/clear its live session;
4. remove product registry/profile association;
5. remove matching BT credentials;
6. persist/verify new product generation;
7. publish `RemoveConfirmed`.

Removing a disconnected saved Mouse does not disturb another current authoritative Mouse.

## Remove destination

- zero saved mice after removal -> `searching-first` + FIRST_MOUSE;
- saved mice remain -> return to valid Saved Devices page;
- if no authoritative Mouse remains, next HOME access -> `home-searching` + SEARCH_SAVED.

## Failure model

Never display successful removal while product/credential state is knowingly inconsistent. Partial failure must have deterministic recovery/reconciliation.

## Name storage/presentation boundary

Storage preserves the best complete normalized name within schema limits.

Projection renders the first 21 renderer-supported characters, no ellipsis/scrolling. If no usable name is available, render `UNKNOWN MOUSE`. Presentation truncation never changes persistent identity/name data.
