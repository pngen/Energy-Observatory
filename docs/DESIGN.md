# Design

Energy Observatory is a header-mostly C++20 library with a tiny compiled core.
This document describes the architecture, layering, and the invariants that
keep it honest.

---

## Layering

    include/eobsv/
      config.hpp        version macros and version_string()
      errors.hpp        domain error hierarchy
      ids.hpp            strong typed identities (Id<Tag>)
      generations.hpp    strong typed generations (Generation<Tag>)
      units.hpp          typed quantities (Quantity<unit U, Rep>) + conversions
      provenance.hpp     ObservationKind, Confidence, TimePoint, Interval,
                         SourceRef, Provenance
      observation.hpp    Observation<QuantityT> (a typed, classified datum)
      energy.hpp         EnergyScope, EnergyEligibility, EnergyComponent,
                         EnergyAttribution, EnergyRecord
      aggregate.hpp      ComponentTotals, partitioning, WorkloadEnergySummary,
                         Evidence freshness
      eobsv.hpp          public umbrella header

    src/version.cpp      the only compiled translation unit
    tests/               a dependency-free test harness and the suite
    examples/            record_energy.cpp

The library builds as a static library (default) or shared (EOBSV_BUILD_SHARED).
Most logic lives in inline headers; version.cpp is kept separate so a linking
consumer gets a real library artifact for packaging and downstream consumption.

## Core types

### Id<Tag> and Generation<Tag>

Both wrap a 64-bit value. Id reserves 0 as the null sentinel; Generation starts
its first meaningful value at 1. Each is a distinct type via its Tag so the
compiler rejects mixing identities or generations at the type level. Both are
constexpr and trivially comparable.

### Quantity<unit U, Rep>

A typed, validated quantity. The unit is a template parameter, so Joules and
Millijoules are distinct types and cannot be combined without an explicit
conversion. Construction rejects NaN, infinities, negative-where-impossible,
out-of-range utilisation, and overflow. Cross-dimension operators support
power * time = energy and energy / time = power.

### Provenance

The container of everything a reader needs to judge a number. It is built with a
core constructor (device, source, epoch, kind, confidence) and fluid setters for
the optional facets (timestamp, interval, derivation inputs, producer, api).
validate() is the single gate that enforces the invariant set; consumers call it
once the value is fully assembled.

### EnergyRecord

An atomic, replayable energy fact. It carries the energy (mJ), an optional
instantaneous power, an attribution, an optional component, active / wall
durations, and full provenance. It enforces active <= wall and the attribution
chain consistency (phase needs an execution, execution needs an attempt, retry /
recovery / pre-failure need an attempt).

### WorkloadEnergySummary

A derived aggregate over a workload's records. It is required to be classified
DERIVED (a raw measured value can never be the aggregate of others). It sums the
total, produces a reconciled component partition, provides per-request and
per-phase attribution, and counts attempts and retries.

## Invariants

The runtime's honesty stems from enforced invariants:

1. A value is always either measured or a labelled derivation.
2. A quantity's unit is explicit and two different units never mix implicitly.
3. A record either reconciles or the runtime throws.
4. A record carries device/source/generation/time/confidence.
5. An impossible physical value is rejected, never clamped, defaulted, or coerced.

## Validation philosophy

Energy Observatory *rejects* rather than *reports-and-continues*. A NaN power,
a negative energy, an out-of-range utilisation, a reversed interval, an
unknown enum value, or an overflowing count are all hard errors. This keeps the
record set trustworthy enough to act on and replayable without surprises.

## Testing

The suite is dependency-free, registers tests through a tiny static registry, and
asserts via exceptions that the harness catches and reports. The library builds
with warnings-as-errors and is validated under AddressSanitizer.

## Packaging

CMake installs the headers, the library target, and an exported CMake package
(EnergyObservatory + EObservatory::eobsv). CPack produces a ZIP (Windows) or TGZ
(POSIX) archive.
