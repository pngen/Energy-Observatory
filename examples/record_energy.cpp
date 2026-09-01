// Energy Observatory — usage example.
//
// This example records the energy consumed by the phases of one workload
// execution, partitions it into useful work / overhead / transfer / waste /
// retry, and prints a replayable summary with full provenance.

#include "eobsv/eobsv.hpp"

#include <cstdio>
#include <vector>

using namespace eobsv;

namespace {

constexpr DeviceId kDevice(1);
constexpr SourceId kSource(7);
constexpr SourceGeneration kSourceGen(3);
constexpr CoordinatorEpoch kEpoch(2);
constexpr PolicyGeneration kPolicy(1);
constexpr EnergyGeneration kEnergyGen(1);

Provenance measured(TimePoint ts) {
  SourceRef ref{kSource, kSourceGen, "nvml"};
  Provenance p{kDevice, ref, kEpoch, ObservationKind::Measured, Confidence::High};
  p.at(ts);
  p.producer("accelerator-backend");
  return p;
}

EnergyRecord record(EnergyRecordId id, WorkloadId w, PhaseId phase,
                    EnergyComponent component, double millijoules, TimePoint ts,
                    bool is_retry = false) {
  EnergyAttribution a;
  a.workload = w;
  a.request = RequestId(1000);
  a.attempt = AttemptId(2000);
  a.execution = ExecutionId(3000);
  a.phase = phase;
  a.is_retry = is_retry;
  EnergyRecord r(id, kEnergyGen, kPolicy, a, Millijoules(millijoules), measured(ts));
  r.component(component);
  r.active_duration(Milliseconds(900.0));
  r.wall_duration(Milliseconds(1000.0));
  return r;
}

}  // namespace

int main() {
  WorkloadId workload(42);
  const TimePoint t0 = TimePoint::from_epoch_seconds(1700000000);

  std::vector<EnergyRecord> records{
      record(EnergyRecordId(1), workload, PhaseId(1), EnergyComponent::UsefulWork, 6000.0, t0),
      record(EnergyRecordId(2), workload, PhaseId(2), EnergyComponent::Overhead, 2000.0, t0),
      record(EnergyRecordId(3), workload, PhaseId(3), EnergyComponent::Transfer, 1500.0, t0),
      record(EnergyRecordId(4), workload, PhaseId(4), EnergyComponent::Waste, 1000.0, t0),
      record(EnergyRecordId(5), workload, PhaseId(1), EnergyComponent::Retry, 900.0, t0, /*is_retry=*/true),
  };

  SourceRef agg_source{SourceId(8), SourceGeneration(1), "observatory-aggregator"};
  Provenance derived{kDevice, agg_source, kEpoch, ObservationKind::Derived, Confidence::High};
  derived.at(t0);
  derived.producer("workload-summary");
  for (std::uint64_t i = 1; i <= 5; ++i) derived.add_input(ObservationId(i));
  WorkloadEnergySummary summary(workload, records, derived);

  const auto totals = summary.partition();
  totals.reconcile();

  std::printf("Energy Observatory %s\n", version_string());
  std::printf("Workload %llu\n", workload.value());
  std::printf("  total energy        : %.0f mJ (%s, %s)\n", summary.total().value(),
              energy_scope_name(EnergyScope::WorkloadAttributed),
              energy_eligibility_name(EnergyEligibility::ExclusivelyAttributable));
  std::printf("  useful work         : %.0f mJ (%s)\n", totals.useful_work.value(),
              energy_component_name(EnergyComponent::UsefulWork));
  std::printf("  overhead            : %.0f mJ (%s)\n", totals.overhead.value(),
              energy_component_name(EnergyComponent::Overhead));
  std::printf("  transfer            : %.0f mJ (%s)\n", totals.transfer.value(),
              energy_component_name(EnergyComponent::Transfer));
  std::printf("  waste               : %.0f mJ (%s)\n", totals.waste.value(),
              energy_component_name(EnergyComponent::Waste));
  std::printf("  retry               : %.0f mJ (%s)\n", totals.retry.value(),
              energy_component_name(EnergyComponent::Retry));
  std::printf("  useful fraction     : %.2f\n", summary.useful_fraction());
  std::printf("  attempts / retries  : %zu / %zu\n", summary.attempt_count(),
              summary.retry_count());

  std::printf("\nPer-phase attribution:\n");
  for (const auto& [phase, e] : summary.by_phase()) {
    std::printf("  phase %llu -> %.0f mJ\n", phase.value(), e.value());
  }

  std::printf("\nAggregate provenance: %s (producer=%s, backend=%s)\n",
              observation_kind_name(summary.provenance().kind()),
              summary.provenance().producer().c_str(),
              summary.provenance().source().backend.c_str());

  return 0;
}
