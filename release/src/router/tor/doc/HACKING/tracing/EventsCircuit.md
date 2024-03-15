# Circuit Subsystem Trace Events

The circuit subsystem emits a series of tracing events related to a circuit
object life cycle and its state change.

This document describes each event as in what data they record and what they
represent.

## Background

There are two types of circuits: origin and OR (onion router). Both of them
are derived from a base object called a general circuit.

- Origin circuits are the ones initiated by tor itself so client or onion
  service circuits for instance.

- OR circuits are the ones going through us that we have not initiated and
  thus only seen by relays.

Many operations are done on the base (general) circuit, and some are specific
to an origin or OR. The following section describes each of them by circuit
type.

## Trace Events

For the LTTng tracer, the subsystem name of these events is: `tor_circuit`.

Also, unless specified otherwise, every event emits a common set of parameters
thus they should always be expected in the following order:

- `circ_id`: For an origin circuit, this is the global circuit identifier used
  in a cell. For an OR circuit, the value is 0.

- `purpose`: Purpose of the circuit as in what it is used for. Note that this
  can change during the lifetime of a circuit. See `CIRCUIT_PURPOSE_*` in
  `core/or/circuitlist.h` for an exhaustive list of the possible values.

- `state`: State of a circuit. This changes during the lifetime of a circuit.
  See `CIRCUIT_STATE_*` in `core/or/circuitlist.h` for an exhaustive list of
  the possible values.

Now, the tracing events.

### General Circuit (`circuit_t`)

The following events are triggered for the base circuit object and thus apply
to all types of circuits.

  * `free`: A circuit object is freed that is memory is released and not
    usable anymore. After this event, no more events will be emitted for the
    specific circuit object.

  * `mark_for_close`: A circuit object is marked for close that is scheduled
    to be closed in a later mainloop periodic event.

    Extra parameters:

    - `end_reason`: Reason why the circuit is closed. Tor often changes that
      reason to something generic sometimes in order to avoid leaking internal
      reasons to the end point. Thus, this value can be different from
      orig_close_reason.

    - `orig_close_reason`: Original reason why the circuit is closed. That
      value never changes and contains the internal reason why we close it. It
      is **never** this reason that is sent back on the circuit.

  * `change_purpose`: Purpose change.

    Extra parameters:

    (`purpose` parameter is not present)

    - `old_purpose`: Previous purpose that is no longer.

    - `new_purpose`: New purpose assigned to the circuit.

  * `change_state`: State change.

    Extra parameters:

    (`state` parameter is not present)

    - `old_state`: Previous state that is no longer.

    - `new_state`: New state assigned to the circuit.

### Origin Circuit (`origin_circuit_t`)

The following events are triggered only for origin circuits.

  * `new_origin`: New origin circuit has been created meaning it has been
    newly allocated, initialized and added to the global list.

  * `establish`: Circuit is being established. This is the initial first step
    where the path was selected and a connection to the first hop has been
    launched.

  * `cannibalized`: Circuit has been cannibalized. This happens when we have
    an already opened unused circuit (preemptive circuits) and it was picked.

  * `first_onion_skin`: First onion skin was sent that is the handshake with
    the first hop.

    Extra parameters:

    - `fingerprint`: Identity digest (RSA) of the first hop.

  * `intermediate_onion_skin`: An intermediate onion skin was sent which can
    be why any hops after the first one. There is thus `N - 1` of these events
    where `N` is the total number of hops in the path.

    Extra parameters:

    - `fingerprint`: Identity digest (RSA) of the next hop.

  * `opened`: Circuit just became opened which means that all hops down the
    path have negotiated the handshake between them and us and the circuit is
    now ready to send cells.

  * `timeout`: Circuit has timed out that is we waited too long for the
    circuit to be built.

  * `idle_timeout`: Circuit has timed out due to idleness. This is controlled
    by the MaxCircuitDirtiness parameter which is 10 min by default.

For the common use case of a 3-hop circuit, the following events should be
seen in this order:

  `new_origin` -> `establish` -> `first_onion_skin` ->
  `intermediate_onion_skin` -> `intermediate_onion_skin` -> `opened`

### OR Circuit (`or_circuit_t`)

The following events are triggered only for OR circuits. For each of them, the
`circ_id` parameter is not present since it would always be 0. The `purpose`
and `state` remain.

  * `new_or`: New OR circuit has been created meaning it has been newly
    allocated, initialized and added to the global list.
