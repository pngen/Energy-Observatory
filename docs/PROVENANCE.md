# Provenance model

Energy Observatory is an *evidence* layer. Every number it records carries
enough provenance that a later reader (or another runtime) can judge the claim
and re-derive it. This document defines the model.

---

## Classification

Every observation is classified into exactly one category:

| Category | Meaning | Direct? |
| --- | --- | --- |
| MEASURED | read directly from a physical telemetry source | yes |
| DERIVED | computed from one or more other observations | no |
| ESTIMATED | interpolated, modelled, or inferred | no |
| SYNTHETIC | generated input / test fixture, not physical telemetry | no |
| UNKNOWN | cannot be classified | no |

A category is a **single source of truth**. is_direct() is derived from the
category and is true only for MEASURED. The runtime never coerces one category
into another: averaging a MEASURED reading produces a new DERIVED observation
that names its inputs; it is never relabelled as MEASURED.

## What every record answers

A Provenance carries enough context to answer eight questions about any number:

1. **what produced it** — producer (a module / function string)
2. **from which device / source** — device (DeviceId) and source (SourceRef)
3. **under what source generation** — source.source_generation (SourceGeneration)
4. **at what timestamp or interval** — at (TimePoint) or interval (Interval)
5. **under what coordinator epoch** — epoch (CoordinatorEpoch)
6. **what API / backend supplied it** — source.backend
7. **whether it is direct or derived** — the kind trait
8. **what confidence applies** — confidence (Confidence)

## Validation invariants

Validation is enforced on construction (for immutable fields) and on final
validate() (for fields set afterwards). The rules:

- ObservationKind and Confidence must be valid enum values
  (an out-of-range underlying value is rejected).
- For any non-UNKNOWN observation the device id, coordinator epoch, source id
  and source generation must be concrete (non-null).
- For any non-UNKNOWN observation, a timestamp or an interval must be present;
  if both are present, the timestamp must lie within the interval.
- A DERIVED observation must name at least one derivation input.
- An UNKNOWN observation may carry a null device / source / epoch.

## Sources and generations

A SourceRef ties a value to a concrete backend (for example "nvml" or
"nvidia-smi") and to a SourceGeneration. A generation change (new firmware,
new driver) means later values are not directly comparable with earlier ones and
must be treated as a distinct state.

## Time and interval

TimePoint rejects timestamps before the epoch and absurdly far in the future.
Interval rejects reversed intervals (end before start) and absurdly long ones.
This keeps the temporal context of every record sane and replayable.

## Confidence

Confidence is None / Low / Medium / High. It is a statement about how much the
recorder trusts the value; it is independent of the category (a MEASURED value
can still be Low confidence if the backend is flaky, while a DERIVED value can be
High confidence if it is over well-bounded inputs).

---

### Why this matters

Power Governor decides what work to allow, limit, defer, or reject. That
decision is only as trustworthy as the energy evidence behind it. Energy
Observatory makes that evidence **provable and repeatable**, so a later review
can reconstruct exactly how a number was obtained rather than trusting a bare
value.
