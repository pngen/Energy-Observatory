# Energy Observatory

> The evidence and attribution layer immediately adjacent to
> [Power Governor](../Power-Governor). It makes accelerator **energy behaviour**
> a first-class, **replayable systems record**: strong typed identities and
> generations, typed units with validation, a provenance-aware observation
> model, and workload/request/attempt/execution/phase energy attribution.

**Status:** production-quality open-source C++20 runtime.

---

## What it answers

| Question | Answered by |
| --- | --- |
| How much power and energy may this accelerator consume now? | **Power Governor** |
| How much energy did accelerator work **actually** consume? | **Energy Observatory** |
| Where did that energy go? | component & scope attribution |
| What useful work did it produce? | useful / overhead / waste partition |
| What evidence supports the attribution? | provenance-aware observations |
| Can that conclusion be reconstructed later? | replayable, validated, generation-stamped records |

This is **not** a dashboard, a generic telemetry wrapper, or a simple NVML
sampler. It is a library that records *facts about energy* with enough
provenance that a later reader can judge the claim and re-derive it.

## The distinctions it preserves

Energy Observatory is built around a set of hard, non-negotiable distinctions.
It refuses to collapse them:

- instantaneous **power** (W) vs. integrated **energy** (J / mJ)
- **measured** values vs. **derived** values
- device-level energy vs. **workload-attributed** energy
- **useful** work vs. **overhead** and **waste**
- **idle / background** energy vs. **active-work** energy
- **shared** energy vs. **exclusively attributable** energy
- observed **facts** vs. **estimates**
- **wall-clock** duration vs. **device-active** duration
- **execution** energy vs. **transfer / memory / retry / recovery / residency** costs
- **current** observations vs. **stale** evidence
- **synthetic** inputs vs. **physical** telemetry
- logical **work identity** vs. physical **execution attempts**
- **pre-failure** work vs. **retry / recovery** work
- aggregate **totals** vs. per-request / per-phase **attribution**

Every one of these maps to a typed identity, a typed unit, or an explicit
enumeration in the public API. See [docs/DISTINCTIONS.md](docs/DISTINCTIONS.md).

## Strong typed identities & generations

Every entity is identified by a strong, type-tagged value so a DeviceId can
never be silently confused with a WorkerId or an ExecutionId. Every mutable
entity carries a generation so two records under different generations are
recognisably different states.

| Identity | Generation |
| --- | --- |
| DeviceId | DeviceGeneration |
| WorkerId, WorkerBootId | CoordinatorEpoch |
| ObservationId | ObservationGeneration |
| WorkloadId | EnergyGeneration |
| RequestId, AttemptId, ExecutionId, PhaseId | PolicyGeneration |
| EnergyRecordId, SourceId | SourceGeneration |

## Typed units & validation

Physical quantities carry their unit in the type system and are validated on
construction: **NaN, infinities, a negative value where a negative value is
physically impossible, out-of-range utilisation, overflow, malformed units,
invalid enum values, impossible timestamps, and nonsensical intervals are all
rejected** rather than silently accepted or coerced.

Units: Watts, Joules, Millijoules, WattHours, Milliseconds, Microseconds,
Bytes, Tokens, Operations, Utilization, Celsius, Kelvin, Clocks.

## Provenance-aware observations

Every observation is classified as exactly one of:

- MEASURED — read directly from physical telemetry (direct)
- DERIVED — computed from one or more other observations
- ESTIMATED — interpolated, modelled, or inferred
- SYNTHETIC — generated input / test fixture, not physical telemetry
- UNKNOWN

A category is **never silently converted** into another. Each record carries
enough provenance to answer, for every number:

1. what produced it (producer)
2. from which device / source (device, source)
3. under what source generation (source.source_generation)
4. at what timestamp or interval (at / interval)
5. under what coordinator epoch (epoch)
6. what API / backend supplied it (source.backend)
7. whether it is direct or derived (trait of kind)
8. what confidence applies (confidence)

## Build

### MSVC / Windows

    cmake -S . -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release
    ctest --test-dir build -C Release

### Linux / macOS

    cmake -S . -B build
    cmake --build build
    ctest --test-dir build

## Usage

    #include <eobsv/eobsv.hpp>
    using namespace eobsv;

    Provenance p{DeviceId(1),
                 SourceRef{SourceId(7), SourceGeneration(3), "nvml"},
                 CoordinatorEpoch(2), ObservationKind::Measured, Confidence::High};
    p.at(TimePoint::from_epoch_seconds(1700000000));

    Observation<Watts> o{ObservationId(100), ObservationGeneration(1), Watts(75.0), p};

See [examples/record_energy.cpp](examples/record_energy.cpp) for the full example
that partitions a workload's energy into useful work / overhead / transfer /
waste / retry and prints a replayable, provenance-carrying summary.

## Packaging

    cmake -S . -B build
    cmake --build build --config Release
    cmake --install build --config Release --prefix <prefix>
    cpack -G ZIP --config build/CPackConfig.cmake

Downstream projects consume the installed package with:

    find_package(EnergyObservatory CONFIG REQUIRED)
    target_link_libraries(app PRIVATE EObservatory::eobsv)

## Options

| Option | Default | Meaning |
| --- | --- | --- |
| EOBSV_BUILD_TESTS | ON | build the test suite |
| EOBSV_BUILD_EXAMPLES | ON | build the examples |
| EOBSV_BUILD_SHARED | OFF | build a shared library |
| EOBSV_WARNINGS_AS_ERRORS | ON | treat warnings as errors |
| EOBSV_ENABLE_SANITIZERS | OFF | enable sanitizers (debug builds) |

## License

[MIT](LICENSE). No Co-authored-by trailers are added to commits.

## Provenance & design notes

- [docs/PROVENANCE.md](docs/PROVENANCE.md) — the provenance model and the
  observation classification rules.
- [docs/DISTINCTIONS.md](docs/DISTINCTIONS.md) — the full catalogue of
  distinctions the runtime refuses to collapse.
- [docs/DESIGN.md](docs/DESIGN.md) — architecture, layering, and invariants.
