# Persistence and removal

## Ownership domains

Persistent state is split into two ownership domains:

1. **Bluetooth credentials** — bond/IRK/LTK/security data owned by BTstack/platform facilities.
2. **Product state** — saved mouse identity/name, confirmed profile, Custom state and capability/vendor metadata owned by Mouse Bridge Remapper.

The product must never treat them as one opaque blob or allow product writes to overwrite credential storage.

## Product record

A versioned persistent schema should contain at least:

```text
header {
  schema_version
  generation
  payload_length
  integrity_check
}

saved_mice[] {
  MouseId
  display_name
  confirmed_profile
  capability_metadata
  vendor_metadata_if_needed
}

global_custom_template
custom_draft
custom_draft_dirty
```

Transient connection handles, HIDS client pointers, queue state and BTstack internal structures are never persisted as product data.

The product may store many saved mice even though only one can be live at a time.

## Power-loss safety

Use two alternating flash generations/slots or an equivalently proven strategy:

1. serialize the new generation completely;
2. write it to the non-current slot;
3. verify integrity/readback as required;
4. consider it current only after successful completion;
5. on boot, select the newest valid generation;
6. if the newest generation is corrupt/torn, fall back to the previous valid generation.

No valid product record means safe defaults rather than undefined state.

## Boot reconstruction order

Before live mouse input becomes authoritative, boot reconstructs:

- saved-device registry;
- each saved mouse's confirmed profile;
- Custom template and unapplied draft state;
- UI profile projection data;
- Logitech vendor behavior requirements.

After storage validation, HOME resolution decides whether the product needs first-mouse search, saved-device search or can show the connected state once a session is ready.

## Saving a newly paired mouse

Pairing is transactional:

```text
discover
 -> authenticate
 -> classify as Mouse
 -> establish stable identity
 -> persist product record
 -> commit ready/saved transition
```

The UI must not report a newly saved mouse if persistence failed.

A search accepts only one winner. Other candidates from the same transaction are not partially saved.

## Profile persistence

Profile Apply is not complete until the new confirmed profile has been persisted successfully.

The accepted G06 behavior of preserving a Custom draft independently from the last actually applied profile remains the inherited default: an accepted per-source draft may survive reboot as dirty/unapplied without falsely becoming the active Custom profile.

## Pair New and saved-state preservation

Pair New may disconnect the currently connected mouse, but **disconnect is not deletion**.

The replaced mouse keeps:

- its `SavedMouse` record;
- confirmed profile;
- Custom relationship;
- Bluetooth bond/credentials.

If the new pairing fails, that previous mouse remains eligible for the ordinary saved-device search when HOME is entered without a live connection.

## Remove transaction

Removing a mouse coordinates all product/security state for that `MouseId`.

Required semantic sequence:

1. if this is the current live mouse, stop accepting new events;
2. release all held Mouse/Escape output for the live session;
3. disconnect and clear the live session if this mouse is connected;
4. remove product registry/profile association;
5. remove matching Bluetooth credentials/security relationship;
6. persist and verify the new product generation;
7. publish `RemoveConfirmed` to UI.

If the removed mouse is not currently connected, the live session of another saved mouse is not disturbed.

## Remove destination

If removal leaves zero saved mice, transition to first-mouse onboarding and begin automatic first-mouse search.

If saved mice remain, return to Saved Devices on a valid remaining page.

If removal also cleared the only live session, then the next HOME entry follows the ordinary rule: saved mice + no connection -> `home-searching` with automatic bounded saved search.

## Failure model

A removal may not be displayed as successful while product state and security state are knowingly inconsistent.

The implementation must define a recoverable state for partial failures and make reboot reconciliation deterministic. Silent half-removal is prohibited.

## Full names vs display names

Storage should preserve the best complete normalized device name available within the chosen schema limits. Renderer truncation/ellipsis, if needed, is a presentation policy and must not destroy the stored identity/name.
