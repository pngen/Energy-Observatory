# Changelog

All notable changes to Energy Observatory are recorded here.
This project adheres to [Semantic Versioning](https://semver.org/).

## [1.0.0] - 2026-01-01

Initial production release. Energy Observatory is the evidence and attribution
layer adjacent to Power Governor: it makes accelerator energy behaviour a
first-class, replayable systems record.

### Added

- Strong typed identities: DeviceId, WorkerId, WorkerBootId, ObservationId,
  WorkloadId, RequestId, AttemptId, ExecutionId, PhaseId, EnergyRecordId,
  SourceId.
- Strong typed generations: DeviceGeneration, CoordinatorEpoch,
  ObservationGeneration, EnergyGeneration, PolicyGeneration, SourceGeneration.
- Typed units with validation: Watts, Joules, Millijoules, WattHours,
  Milliseconds, Microseconds, Bytes, Tokens, Operations, Utilization, Celsius,
  Kelvin, Clocks. Conversion functions and power * time = energy operators.
- Validation that rejects NaN, infinities, negative values where impossible,
  out-of-range utilisation, overflow, malformed units, invalid enum values,
  impossible timestamps, and nonsensical intervals.
- Provenance-aware observation model: ObservationKind (MEASURED / DERIVED /
  ESTIMATED / SYNTHETIC / UNKNOWN), Confidence, TimePoint, Interval, SourceRef,
  Provenance, and Observation<QuantityT>.
- Energy attribution model: EnergyScope, EnergyEligibility, EnergyComponent,
  EnergyAttribution, EnergyRecord.
- Aggregation and reconciliation: ComponentTotals, partition_by_component,
  WorkloadEnergySummary, Evidence staleness.
- CMake build system with warnings-as-errors, AddressSanitizer support, install
  and package (CPack) rules, and an exported CMake package.
- Dependency-free test harness with a comprehensive suite (65 tests).
- Example: examples/record_energy.cpp.
- Documentation: README, docs/DISTINCTIONS.md, docs/PROVENANCE.md,
  docs/DESIGN.md.
