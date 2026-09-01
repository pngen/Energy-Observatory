#pragma once

#include <optional>
#include <string_view>
#include <vector>

#include "eobsv/errors.hpp"
#include "eobsv/generations.hpp"
#include "eobsv/ids.hpp"
#include "eobsv/provenance.hpp"
#include "eobsv/units.hpp"

namespace eobsv {

// ---------------------------------------------------------------------------
// Energy scope.
//   * DeviceLevel         energy consumed by the device (whole-device total,
//                         idle/background, device-level counters).
//   * WorkloadAttributed  energy attributed to a logical workload (and
//                         optionally a request / attempt / execution / phase).
// ---------------------------------------------------------------------------
enum class EnergyScope : std::uint8_t { DeviceLevel, WorkloadAttributed };

constexpr const char* energy_scope_name(EnergyScope s) noexcept {
  switch (s) {
    case EnergyScope::DeviceLevel:         return "DEVICE";
    case EnergyScope::WorkloadAttributed:  return "WORKLOAD";
  }
  return "?";
}

// ---------------------------------------------------------------------------
// Eligibility: was the energy exclusively caused by and attributable to the
// described work, or was the device shared with other work during the interval
// such that attribution is approximate?
// ---------------------------------------------------------------------------
enum class EnergyEligibility : std::uint8_t {
  ExclusivelyAttributable,  // the energy belongs solely to the described work
  Shared,                   // the device was shared; attribution is approximate
  Unattributable            // no meaningful attribution exists (device-level)
};

constexpr const char* energy_eligibility_name(EnergyEligibility e) noexcept {
  switch (e) {
    case EnergyEligibility::ExclusivelyAttributable: return "EXCLUSIVE";
    case EnergyEligibility::Shared:                  return "SHARED";
    case EnergyEligibility::Unattributable:          return "UNATTRIBUTABLE";
  }
  return "?";
}

// ---------------------------------------------------------------------------
// Energy component: a category describing what the energy paid for. Separation
// of "useful work" from "overhead", "waste", "retry", "recovery", "transfer",
// "memory" and "residency" is exactly what Energy Observatory is for.
// ---------------------------------------------------------------------------
enum class EnergyComponent : std::uint8_t {
  Execution,        // compute / active work
  Transfer,         // data movement and communication
  Memory,           // memory subsystem
  Retry,            // re-running work after a failure
  Recovery,         // restoring state after a failure
  Residency,        // power held while resident but not computing
  IdleBackground,   // idle / background device energy
  Overhead,         // launch / framework / scheduling overhead
  Waste,            // energy spent on work that produced no useful output
  UsefulWork        // the energy that directly produced useful output
};

constexpr const char* energy_component_name(EnergyComponent c) noexcept {
  switch (c) {
    case EnergyComponent::Execution:      return "EXECUTION";
    case EnergyComponent::Transfer:       return "TRANSFER";
    case EnergyComponent::Memory:         return "MEMORY";
    case EnergyComponent::Retry:          return "RETRY";
    case EnergyComponent::Recovery:       return "RECOVERY";
    case EnergyComponent::Residency:      return "RESIDENCY";
    case EnergyComponent::IdleBackground: return "IDLE_BACKGROUND";
    case EnergyComponent::Overhead:       return "OVERHEAD";
    case EnergyComponent::Waste:          return "WASTE";
    case EnergyComponent::UsefulWork:     return "USEFUL_WORK";
  }
  return "?";
}

inline void check_energy_component(EnergyComponent c) {
  switch (c) {
    case EnergyComponent::Execution:
    case EnergyComponent::Transfer:
    case EnergyComponent::Memory:
    case EnergyComponent::Retry:
    case EnergyComponent::Recovery:
    case EnergyComponent::Residency:
    case EnergyComponent::IdleBackground:
    case EnergyComponent::Overhead:
    case EnergyComponent::Waste:
    case EnergyComponent::UsefulWork:
      return;
  }
  throw InvalidEnum("invalid EnergyComponent value");
}

// ---------------------------------------------------------------------------
// EnergyAttribution.
//
// Logical work identity (workload, request) is tracked separately from the
// physical execution attempts (attempt, execution, phase) so that retries and
// recovery are first-class, replayable facts and pre-failure work is
// distinguishable from the retry/recovery work that followed it.
// ---------------------------------------------------------------------------
struct EnergyAttribution {
  EnergyScope scope = EnergyScope::WorkloadAttributed;
  WorkloadId workload;
  std::optional<RequestId> request;
  std::optional<AttemptId> attempt;
  std::optional<ExecutionId> execution;
  std::optional<PhaseId> phase;
  EnergyEligibility eligibility = EnergyEligibility::ExclusivelyAttributable;
  bool is_retry = false;     // this attempt is a retry of prior failed work
  bool is_recovery = false;  // this energy went to recovery
  bool pre_failure = false;  // this energy was consumed before a failure

  void validate() const {
    if (scope == EnergyScope::WorkloadAttributed && !workload.has_value()) {
      throw InvalidAttribution("workload-attributed energy requires a workload id");
    }
    if (scope == EnergyScope::DeviceLevel && workload.has_value()) {
      throw InvalidAttribution("device-level energy cannot carry a workload id");
    }
    // Physical chain consistency: phase implies execution implies attempt.
    if (phase.has_value() && !execution.has_value()) {
      throw InvalidAttribution("phase is set without an execution");
    }
    if (execution.has_value() && !attempt.has_value()) {
      throw InvalidAttribution("execution is set without an attempt");
    }
    if ((is_retry || is_recovery || pre_failure) && !attempt.has_value()) {
      throw InvalidAttribution("retry/recovery/pre-failure flag set without an attempt");
    }
  }

  bool workload_attributed() const noexcept { return scope == EnergyScope::WorkloadAttributed; }
  bool concrete_execution() const noexcept { return execution.has_value(); }
};

// ---------------------------------------------------------------------------
// EnergyRecord.
//
// One atomic, replayable energy fact. It distinguishes:
//   * instantaneous power (power)  vs  integrated energy (energy)
//   * wall-clock duration (wall_duration) vs device-active duration
//     (active_duration)
//   * where it came from and what produced it (provenance)
// ---------------------------------------------------------------------------
class EnergyRecord {
 public:
  EnergyRecord(EnergyRecordId id, EnergyGeneration generation, PolicyGeneration policy,
               EnergyAttribution attribution, Millijoules energy, Provenance provenance)
      : id_(id), generation_(generation), policy_(policy),
        attribution_(std::move(attribution)), energy_(energy),
        provenance_(std::move(provenance)) {
    if (!id_.has_value()) throw InvalidIdentity("energy record id is null");
    if (!generation_.has_value()) throw InvalidIdentity("energy generation is null");
    if (!policy_.has_value()) throw InvalidIdentity("policy generation is null");
    attribution_.validate();
    provenance_.validate();
    validate();
  }

  // Fluid setters for optional facets.
  EnergyRecord& power(Watts w) { power_ = w; return *this; }
  EnergyRecord& component(EnergyComponent c) {
    check_energy_component(c);
    component_ = c;
    return *this;
  }
  EnergyRecord& active_duration(Milliseconds d) { active_duration_ = d; return *this; }
  EnergyRecord& wall_duration(Milliseconds d) { wall_duration_ = d; return *this; }
  EnergyRecord& utilization(Utilization u) { utilization_ = u; return *this; }
  EnergyRecord& worker(WorkerId w) { worker_ = w; return *this; }
  EnergyRecord& worker_boot(WorkerBootId b) { worker_boot_ = b; return *this; }
  EnergyRecord& device_generation(DeviceGeneration g) { device_generation_ = g; return *this; }

  void validate() const {
    if (active_duration_.has_value() && wall_duration_.has_value()) {
      if (active_duration_->value() > wall_duration_->value()) {
        throw InvalidTime("device-active duration exceeds wall-clock duration");
      }
    }
    if (energy_.value() < 0) throw InvalidQuantity("energy record is negative");
  }

  EnergyRecordId id() const noexcept { return id_; }
  EnergyGeneration generation() const noexcept { return generation_; }
  PolicyGeneration policy() const noexcept { return policy_; }
  const EnergyAttribution& attribution() const noexcept { return attribution_; }
  Millijoules energy() const noexcept { return energy_; }
  const std::optional<Watts>& power() const noexcept { return power_; }
  const std::optional<EnergyComponent>& component() const noexcept { return component_; }
  const std::optional<Milliseconds>& active_duration() const noexcept { return active_duration_; }
  const std::optional<Milliseconds>& wall_duration() const noexcept { return wall_duration_; }
  const std::optional<Utilization>& utilization() const noexcept { return utilization_; }
  const std::optional<WorkerId>& worker() const noexcept { return worker_; }
  const std::optional<WorkerBootId>& worker_boot() const noexcept { return worker_boot_; }
  const std::optional<DeviceGeneration>& device_generation() const noexcept { return device_generation_; }
  const Provenance& provenance() const noexcept { return provenance_; }

 private:
  EnergyRecordId id_;
  EnergyGeneration generation_;
  PolicyGeneration policy_;
  EnergyAttribution attribution_;
  Millijoules energy_;
  Provenance provenance_;
  std::optional<Watts> power_;
  std::optional<EnergyComponent> component_;
  std::optional<Milliseconds> active_duration_;
  std::optional<Milliseconds> wall_duration_;
  std::optional<Utilization> utilization_;
  std::optional<WorkerId> worker_;
  std::optional<WorkerBootId> worker_boot_;
  std::optional<DeviceGeneration> device_generation_;
};

}  // namespace eobsv
