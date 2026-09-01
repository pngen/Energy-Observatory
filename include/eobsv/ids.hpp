#pragma once

#include <cstdint>
#include <compare>

namespace eobsv {

// ---------------------------------------------------------------------------
// Strong typed identity.
//
// Every entity in Energy Observatory is identified by a *typed* value so that a
// DeviceId can never be silently mixed up with a WorkerId or an ExecutionId at
// the type level. The value 0 is reserved as the "null / unset" sentinel and is
// never a legitimate concrete identity; validation that an identity is concrete
// happens where the identity is semantically required.
// ---------------------------------------------------------------------------
template <typename Tag, typename Underlying = std::uint64_t>
class Id {
 public:
  using tag_type = Tag;
  using value_type = Underlying;

  constexpr Id() noexcept = default;
  constexpr explicit Id(Underlying value) noexcept : value_(value) {}

  constexpr Underlying value() const noexcept { return value_; }

  // True when this identity is concrete (non-zero / non-null).
  constexpr bool has_value() const noexcept { return value_ != Underlying{0}; }
  constexpr bool empty() const noexcept { return value_ == Underlying{0}; }
  constexpr explicit operator bool() const noexcept { return has_value(); }

  constexpr bool operator==(const Id&) const noexcept = default;
  constexpr auto operator<=>(const Id&) const noexcept = default;

 private:
  Underlying value_{0};
};

// ---------------------------------------------------------------------------
// Identity declarations. The empty tag structs exist solely to give each Id a
// distinct type; they never carry data.
// ---------------------------------------------------------------------------

#define EOBSV_DECLARE_ID(NAME, TAG) struct TAG {}; using NAME = Id<TAG>;

// Device identity (a physical or virtual accelerator / GPU).
EOBSV_DECLARE_ID(DeviceId, DeviceIdTag)

// Worker identity (a compute worker or execution context).
EOBSV_DECLARE_ID(WorkerId, WorkerIdTag)

// Worker boot session identity (one incarnation / boot of a worker).
EOBSV_DECLARE_ID(WorkerBootId, WorkerBootIdTag)

// A single observed datum.
EOBSV_DECLARE_ID(ObservationId, ObservationIdTag)

// A logical workload (a unit of requested work).
EOBSV_DECLARE_ID(WorkloadId, WorkloadIdTag)

// A logical request within a workload.
EOBSV_DECLARE_ID(RequestId, RequestIdTag)

// A physical attempt to satisfy a request (may be retried).
EOBSV_DECLARE_ID(AttemptId, AttemptIdTag)

// A concrete execution of an attempt on a worker.
EOBSV_DECLARE_ID(ExecutionId, ExecutionIdTag)

// A named phase of an execution.
EOBSV_DECLARE_ID(PhaseId, PhaseIdTag)

// A persisted energy record.
EOBSV_DECLARE_ID(EnergyRecordId, EnergyRecordIdTag)

// A provenance source (a telemetry backend, sampler, or producer).
EOBSV_DECLARE_ID(SourceId, SourceIdTag)

#undef EOBSV_DECLARE_ID

}  // namespace eobsv
