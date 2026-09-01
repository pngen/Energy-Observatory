#pragma once

#include <map>
#include <variant>
#include <vector>

#include "eobsv/energy.hpp"
#include "eobsv/errors.hpp"
#include "eobsv/provenance.hpp"
#include "eobsv/units.hpp"

namespace eobsv {

struct ComponentTotals {
  Millijoules execution{0};
  Millijoules transfer{0};
  Millijoules memory{0};
  Millijoules retry{0};
  Millijoules recovery{0};
  Millijoules residency{0};
  Millijoules idle_background{0};
  Millijoules overhead{0};
  Millijoules waste{0};
  Millijoules useful_work{0};
  Millijoules total{0};

  void reconcile() const {
    Millijoules acc{0};
    for (const Millijoules& m : {execution, transfer, memory, retry, recovery, residency, idle_background, overhead, waste, useful_work}) {
      acc = acc + m;
    }
    if (acc.value() != total.value()) {
      throw InvalidQuantity("energy partition does not reconcile with total");
    }
  }
};

inline ComponentTotals partition_by_component(const std::vector<EnergyRecord>& records) {
  ComponentTotals totals;
  for (const auto& r : records) {
    if (!r.component().has_value()) {
      throw InvalidAttribution("record lacks an energy component; cannot partition");
    }
    const auto& e = r.energy();
    switch (*r.component()) {
      case EnergyComponent::Execution:      totals.execution = totals.execution + e; break;
      case EnergyComponent::Transfer:       totals.transfer = totals.transfer + e; break;
      case EnergyComponent::Memory:         totals.memory = totals.memory + e; break;
      case EnergyComponent::Retry:          totals.retry = totals.retry + e; break;
      case EnergyComponent::Recovery:       totals.recovery = totals.recovery + e; break;
      case EnergyComponent::Residency:      totals.residency = totals.residency + e; break;
      case EnergyComponent::IdleBackground: totals.idle_background = totals.idle_background + e; break;
      case EnergyComponent::Overhead:       totals.overhead = totals.overhead + e; break;
      case EnergyComponent::Waste:          totals.waste = totals.waste + e; break;
      case EnergyComponent::UsefulWork:     totals.useful_work = totals.useful_work + e; break;
    }
    totals.total = totals.total + e;
  }
  totals.reconcile();
  return totals;
}

inline Millijoules sum_energy(const std::vector<EnergyRecord>& records) {
  Millijoules acc{0};
  for (const auto& r : records) acc = acc + r.energy();
  return acc;
}

class WorkloadEnergySummary {
 public:
  WorkloadEnergySummary(WorkloadId workload, std::vector<EnergyRecord> records, Provenance provenance)
      : workload_(workload), records_(std::move(records)), provenance_(std::move(provenance)) {
    if (!workload_.has_value()) throw InvalidIdentity("workload id is null");
    if (provenance_.kind() != ObservationKind::Derived) {
      throw InvalidProvenance("aggregate summary must be classified DERIVED");
    }
    provenance_.validate();
    recompute();
  }

  const WorkloadId& workload() const noexcept { return workload_; }
  const std::vector<EnergyRecord>& records() const noexcept { return records_; }
  const Provenance& provenance() const noexcept { return provenance_; }
  Millijoules total() const noexcept { return total_; }
  ComponentTotals partition() const { return partition_by_component(records_); }
  double useful_fraction() const noexcept {
    const auto p = partition_by_component(records_);
    if (total_.value() == 0.0) return 0.0;
    return p.useful_work.value() / total_.value();
  }
  const std::map<RequestId, Millijoules>& by_request() const noexcept { return by_request_; }
  const std::map<PhaseId, Millijoules>& by_phase() const noexcept { return by_phase_; }
  std::size_t attempt_count() const noexcept { return attempts_; }
  std::size_t retry_count() const noexcept { return retries_; }
  Millijoules pre_failure_energy() const noexcept { return pre_failure_; }
  Millijoules retry_energy() const noexcept { return retry_; }
  Millijoules recovery_energy() const noexcept { return recovery_; }

 private:
  void recompute() {
    total_ = Millijoules{0};
    attempts_ = 0;
    retries_ = 0;
    for (const auto& r : records_) {
      total_ = total_ + r.energy();
      const auto& a = r.attribution();
      if (a.scope != EnergyScope::WorkloadAttributed) {
        throw InvalidAttribution("workload summary contains a non-workload record");
      }
      if (a.request.has_value()) by_request_[*a.request] = by_request_[*a.request] + r.energy();
      if (a.phase.has_value()) by_phase_[*a.phase] = by_phase_[*a.phase] + r.energy();
      if (a.attempt.has_value()) {
        ++attempts_;
        if (a.is_retry) { ++retries_; retry_ = retry_ + r.energy(); }
        if (a.is_recovery) recovery_ = recovery_ + r.energy();
        if (a.pre_failure) pre_failure_ = pre_failure_ + r.energy();
      }
    }
  }

  WorkloadId workload_;
  std::vector<EnergyRecord> records_;
  Provenance provenance_;
  Millijoules total_{0};
  std::map<RequestId, Millijoules> by_request_;
  std::map<PhaseId, Millijoules> by_phase_;
  std::size_t attempts_ = 0;
  std::size_t retries_ = 0;
  Millijoules pre_failure_{0};
  Millijoules retry_{0};
  Millijoules recovery_{0};
};

struct Evidence {
  TimePoint observed_at;
  bool stale_as_of(TimePoint now, Milliseconds max_age) const {
    const auto age = now.time_point() - observed_at.time_point();
    const auto age_ms = std::chrono::duration_cast<std::chrono::milliseconds>(age).count();
    return age_ms > max_age.value();
  }
};

}  // namespace eobsv
