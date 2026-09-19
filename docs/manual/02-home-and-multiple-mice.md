# Home and multiple mice

Mouse Bridge Remapper may keep several mice connected at the same time. HOME reports connection state without pretending that the runtime has only one mouse.

## HOME while searching saved devices

When saved mice exist but none is currently connected, the HOME family may show `SEARCHING SAVED MOUSE` while the bounded saved-device search is active.

The menu remains available during this search:

- `PAIR NEW MOUSE`
- `SAVED DEVICES`
- `LEARN THE KEYS`

`KEY B: CANCEL SEARCH` cancels only the current saved-device search. It does not erase Saved Devices.

If the saved search expires without a connection, HOME shows `DEVICE NOT FOUND`. `KEY A: RETRY SEARCH` starts another saved-device search.

## HOME with exactly one connected mouse

When exactly one mouse is connected, the second line of `MOUSE CONNECTED` shows that mouse's display name.

Example:

```text
MOUSE CONNECTED
LOGITECH LIFT
 REMAPPED TO ESCAPE
 SAVED DEVICES
 LEARN THE KEYS

JOY UP / DOWN: SELECT
JOY PRESS: ACCESS
KEY X: HELP TO REMOVE
```

The profile summary line reflects the confirmed profile associated with the configuration target defined by the product contract. It must never show a profile merely because an Apply control was pressed; runtime and required persistence must have succeeded first.

## HOME with two or more connected mice

When more than one mouse is connected, line 2 no longer shows one mouse name. It shows the total ready connected-device count:

```text
2 DEVICES CONNECTED
3 DEVICES CONNECTED
...
999 DEVICES CONNECTED
```

Formatting is exactly:

`<decimal count> DEVICES CONNECTED`

There is no leading indentation on that line.

The product/UI maximum is **999 connected mice**. `999 DEVICES CONNECTED` is exactly 21 characters, so the fixed-width display contract is not exceeded.

The counter represents mice that are currently in the product's ready/connected state. Saved but disconnected mice are not included.

## No automatic expansion of the connection set

Once at least one mouse is connected, HOME does not automatically begin searching for another mouse. This remains true even if:

- other saved mice are nearby;
- another unsaved mouse is in pairing mode;
- the product has fewer than 999 connected mice.

The user must explicitly request another connection through the relevant pairing flow.

## Concurrent input

All ready mice continue to work simultaneously:

- movement from different mice contributes to the same host-visible pointer stream;
- wheel and horizontal pan events from different mice are forwarded;
- held buttons are tracked by source so one mouse cannot release another mouse's hold;
- disconnecting one mouse releases only that mouse's ownership;
- locking or navigating the LCD does not stop mouse forwarding.

## Product maximum vs hardware evidence

`999` is the normative product/UI upper bound. It prevents the count text from exceeding the 21-character line width.

The firmware and release documentation must separately state the **physically validated concurrent BLE capacity**. They must not imply that 999 live BLE links have been proven merely because the UI contract accepts counts through 999.

## Profile-editing target with multiple mice

The product has not yet defined which individual mouse receives a profile change initiated from the connected HOME when two or more mice are connected. The implementation must not guess that it is the first connected, last connected, last moved, or all connected mice.

Until that product decision is explicitly documented, the runtime must preserve independent per-mouse profile identities and keep the presentation target separate from the connection set. See [Open product decisions](../architecture/09-open-decisions.md).
