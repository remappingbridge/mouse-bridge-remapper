# First start and pairing

This chapter defines how Mouse Bridge Remapper behaves before and during mouse discovery.

## First mouse

When the product starts with **no saved mouse**, the first screen is `SEARCHING FIRST MOUSE`.

The product immediately begins searching for a valid BLE HOGP mouse. The first-mouse search is logically continuous: internally it may use finite scan/connect cycles, but it must keep restarting them until a first mouse is successfully found, authenticated, classified as a mouse, saved and ready.

If several unsaved mice are waiting to pair at the same time, **only the first valid mouse found is paired**. As soon as that mouse is accepted, the automatic search ends. The product does not continue pairing the remaining candidates.

The `SEARCHING FIRST MOUSE` screen is didactic while search runs. HAT controls shown on the screen only provide their declared pressed/released visual feedback; they do not navigate away from the search.

## First successful connection

After the first mouse is successfully persisted and ready, the product shows `FIRST MOUSE CONNECTED` before normal HOME use.

This success screen must not be shown merely because an advertisement was seen or a connection handle was allocated. The mouse must have completed the product's accepted connection/classification/persistence path.

## Startup when saved mice exist

When one or more mice are already saved and the product powers on with no live connection yet, HOME enters the saved-device search state and attempts to reconnect a saved mouse for a bounded period.

The search is not indefinite. If no qualifying saved mouse is found in the configured interval, HOME moves to the retry state and the user can explicitly retry.

Once **one saved mouse has successfully connected**, automatic search stops. The product must not continue searching automatically for a second, third or later mouse merely because saved capacity remains.

This rule exists to prevent unexpected additional connections. Extra mice are connected only through an explicit user action.

## Pairing an additional mouse

Use `PAIR NEW MOUSE` to add another mouse.

Starting Pair New does not disconnect or disable mice that are already connected. Existing mice continue forwarding input while the new pairing transaction runs.

Pair New accepts a valid mouse that is not already present in Saved Devices. If several unsaved mice are simultaneously waiting to pair, **only the first valid one found is accepted**, and that manual search stops immediately after the successful pair. To add another mouse, start Pair New again.

A failed or canceled Pair New attempt does not delete, replace or invalidate any previously saved mouse.

## Saved vs new searches

The product intentionally distinguishes two search purposes:

- **saved-device search** tries to reconnect a mouse already known to the product;
- **Pair New** tries to add a mouse that is not yet saved.

The UI help screens explain this distinction. A search transaction never silently changes purpose.

## No automatic second mouse

The following is a product invariant:

> After one or more mice are connected, Mouse Bridge Remapper does not start an automatic search merely to increase the number of connected mice.

A later mouse connection requires the user to enter Pair New or another explicit connection action defined by the manual.

## One result per search transaction

Every pairing/discovery transaction that can accept a new mouse has a single-winner rule:

1. discover candidates;
2. validate/classify them;
3. accept the first valid candidate satisfying that transaction;
4. finish persistence/connection;
5. stop that search transaction.

The product never pairs two devices from one Pair New action and never turns one first-mouse search into a batch-pair operation.
