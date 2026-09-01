# Distinctions

Energy Observatory is defined by the distinctions it *refuses to collapse*.
Each distinction below is mapped to a concrete modelling decision in the public
API. Source: the runtime contract in this repository.

---

## 1. Instantaneous power vs. integrated energy

A watt is a *rate*; a joule is an *amount*. They are different types:
Watts and Joules. Energy over a window is integrated, never confused with the
power at an instant.

    Watts w(120.0);                      // instantaneous
    Millijoules e(4000.0);               // integrated
    auto from_window = w * Milliseconds(500.0);  // W * ms -> J

## 2. Measured vs. derived values

Every observation is one of MEASURED / DERIVED / ESTIMATED / SYNTHETIC /
UNKNOWN. A MEASURED value that is later averaged is a new DERIVED observation
carrying its inputs; it is never relabelled as MEASURED. is_direct() is
true only for MEASURED.

## 3. Device-level vs. workload-attributed energy

EnergyScope::DeviceLevel describes whole-device energy (idle, background,
device counters). EnergyScope::WorkloadAttributed attributes energy to a
WorkloadId (and optionally a request / attempt / execution / phase). A
device-level record must not carry a workload id.

## 4. Useful work vs. overhead vs. waste

EnergyComponent classifies what energy paid for: USEFUL_WORK, OVERHEAD,
WASTE, plus EXECUTION, TRANSFER, MEMORY, RETRY, RECOVERY, RESIDENCY,
IDLE_BACKGROUND. partition_by_component sums these and **requires the
buckets to reconcile with the total**, or it throws.

## 5. Idle / background vs. active-work energy

IDLE_BACKGROUND is a distinct component, and EnergyRecord::active_duration
(the device-active time) is validated to never exceed wall_duration.

## 6. Shared vs. exclusively attributable energy

EnergyEligibility::ExclusivelyAttributable claims the energy belongs solely to
the described work. Shared marks a device that was shared during the window,
so attribution is approximate. Unattributable marks device-level energy with
no meaningful attribution.

## 7. Observed facts vs. estimates

MEASURED and DERIVED are observations over recorded telemetry. ESTIMATED
marks interpolated or modelled numbers; UNKNOWN marks an unclassifiable value.
An estimate never masquerades as a measured fact.

## 8. Wall-clock duration vs. device-active duration

EnergyRecord::wall_duration and active_duration are separate fields, and
the active duration is validated to be no greater than the wall duration.

## 9. Execution energy vs. transfer / memory / retry / recovery / residency

Each is a separate EnergyComponent. Retry and recovery work is additionally
tagged on the attribution with is_retry, is_recovery, and pre_failure
flags, so post-failure energy is distinguishable from the work that failed.

## 10. Current observations vs. stale evidence

Evidence::stale_as_of(now, max_age) distinguishes a still-current reading
from stale evidence, so a reclaim policy never acts on an outdated number.

## 11. Synthetic inputs vs. physical telemetry

SYNTHETIC marks generated test input as distinct from physical MEASURED
telemetry. A synthetic number is never presented as a physical fact.

## 12. Logical work identity vs. physical execution attempts

A WorkloadId / RequestId is the *logical* identity of requested work. An
AttemptId / ExecutionId / PhaseId is the *physical* attempt to satisfy it. A
logical request may map to many physical attempts across retries.

## 13. Pre-failure work vs. retry / recovery work

EnergyAttribution::pre_failure, is_retry, and is_recovery are independent
flags, so energy consumed before a failure is not conflated with the energy the
retry/recovery consumed afterwards.

## 14. Aggregate totals vs. per-request / per-phase attribution

WorkloadEnergySummary keeps the aggregate total *and* the
by_request / by_phase splits, and records that it is DERIVED.

## 15. Direct vs. derived

Encapsulated by ObservationKind and is_direct(). MEASURED is direct;
everything else is a derivation or an estimate.

## 16. Which API / backend supplied it

SourceRef::backend names the producer, e.g. "nvml" or "nvidia-smi". Combined
with SourceId and SourceGeneration.

## 17. What confidence applies

Confidence is None / Low / Medium / High, carried on every provenance.

## 18. Strong typed identities

Id<Tag> wraps a 64-bit value and is a distinct type per entity, so
DeviceId, WorkerId, WorkloadId, RequestId, AttemptId, ExecutionId, PhaseId,
ObservationId, EnergyRecordId and SourceId can never be mixed at compile time.

## 19. Strong typed generations

Generation<Tag> identifies the incarnation of a mutable entity. Two records
referring to the same entity under different DeviceGeneration /
ObservationGeneration / EnergyGeneration / PolicyGeneration / SourceGeneration /
CoordinatorEpoch describe different states and are not naively combined.

## 20. Replay & reconstruction

All validation is enforced on construction and on final validation:
NaN, infinities, negative-where-impossible, out-of-range utilisation, overflow,
malformed units, invalid enum values, impossible timestamps and nonsensical
intervals are rejected rather than silently corrected.

---

The invariant, stated plainly:

> Every number is either a measured fact or a labelled derivation, carries its
> unit, its device/source/generation, its time context, and its confidence —
> and either reconciles or the runtime says so loudly.
