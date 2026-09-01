#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "eobsv/errors.hpp"
#include "eobsv/generations.hpp"
#include "eobsv/ids.hpp"
#include "eobsv/units.hpp"

namespace eobsv {

// ---------------------------------------------------------------------------
// Observation classification.
//
// Every observation must be classified into exactly one of these categories.
// A category is never silently coerced into another; a MEASURED reading that is
// later averaged becomes a DERIVED observation, never a re-labelled MEASURED
// one. This is the heart of the provenance model.
// ---------------------------------------------------------------------------
enum class ObservationKind : std::uint8_t {
  Measured,   // read directly from a physical telemetry source (direct)
  Derived,    // computed from one or more other observations
  Estimated,  // interpolated, modelled, or inferred
  Synthetic,  // generated input / test fixture, not physical telemetry
  Unknown     // cannot be classified
};

constexpr const char* observation_kind_name(ObservationKind k) noexcept {
  switch (k) {
    case ObservationKind::Measured:  return "MEASURED";
    case ObservationKind::Derived:   return "DERIVED";
    case ObservationKind::Estimated: return "ESTIMATED";
    case ObservationKind::Synthetic: return "SYNTHETIC";
    case ObservationKind::Unknown:   return "UNKNOWN";
  }
  return "UNKNOWN";
}

// True only for genuinely measured, first-hand telemetry.
constexpr bool is_direct(const ObservationKind k) noexcept {
  return k == ObservationKind::Measured;
}

// Parse a canonical name ("MEASURED" / "measured") into an ObservationKind.
// Throws InvalidEnum for anything outside the declared set so that malformed
// enum values are rejected, not silently defaulted.
inline ObservationKind parse_observation_kind(std::string_view name) {
  struct Entry { std::string_view n; ObservationKind k; };
  static constexpr Entry entries[] = {
      {"MEASURED", ObservationKind::Measured},
      {"measured", ObservationKind::Measured},
      {"DERIVED", ObservationKind::Derived},
      {"derived", ObservationKind::Derived},
      {"ESTIMATED", ObservationKind::Estimated},
      {"estimated", ObservationKind::Estimated},
      {"SYNTHETIC", ObservationKind::Synthetic},
      {"synthetic", ObservationKind::Synthetic},
      {"UNKNOWN", ObservationKind::Unknown},
      {"unknown", ObservationKind::Unknown},
  };
  for (const auto& e : entries) {
    if (name == e.n) return e.k;
  }
  throw InvalidEnum("unknown observation kind: " + std::string(name));
}

// Validate that an arbitrary enum underlying value maps to a known kind
// (guards deserialization / reflection paths).
inline void check_observation_kind(ObservationKind k) {
  switch (k) {
    case ObservationKind::Measured:
    case ObservationKind::Derived:
    case ObservationKind::Estimated:
    case ObservationKind::Synthetic:
    case ObservationKind::Unknown:
      return;
  }
  throw InvalidEnum("invalid ObservationKind value");
}

// ---------------------------------------------------------------------------
// Confidence.
// ---------------------------------------------------------------------------
enum class Confidence : std::uint8_t { None, Low, Medium, High };

constexpr const char* confidence_name(Confidence c) noexcept {
  switch (c) {
    case Confidence::None:   return "NONE";
    case Confidence::Low:    return "LOW";
    case Confidence::Medium: return "MEDIUM";
    case Confidence::High:   return "HIGH";
  }
  return "NONE";
}

inline void check_confidence(Confidence c) {
  switch (c) {
    case Confidence::None:
    case Confidence::Low:
    case Confidence::Medium:
    case Confidence::High:
      return;
  }
  throw InvalidEnum("invalid Confidence value");
}

// ---------------------------------------------------------------------------
// Time / interval.
//
// Timestamps and intervals are validated so that "impossible" wall-clock values
// (before the epoch, absurdly far in the future) and nonsensical intervals
// (reversed or absurdly long) are rejected at construction.
// ---------------------------------------------------------------------------
using SysClock = std::chrono::system_clock;

// Seconds from grid epoch to about the year 3000. Used as the upper bound for
// "plausible" accelerator telemetry timestamps.
inline constexpr std::int64_t kMaxTimestampSeconds = 32503680000LL;
// An interval longer than ~10 years is treated as nonsensical for a single
// accelerator observation.
inline constexpr std::int64_t kMaxIntervalSeconds = 315576000LL;

class TimePoint {
 public:
  TimePoint() = default;

  explicit TimePoint(SysClock::time_point tp) : tp_(tp) {
    const std::int64_t secs =
        std::chrono::duration_cast<std::chrono::seconds>(tp_.time_since_epoch()).count();
    if (secs < 0) throw InvalidTime("timestamp before epoch");
    if (secs > kMaxTimestampSeconds) throw InvalidTime("timestamp too far in the future");
  }

  static TimePoint from_epoch_seconds(std::int64_t secs) {
    return TimePoint(SysClock::time_point(std::chrono::seconds(secs)));
  }
  static TimePoint from_epoch_millis(std::int64_t ms) {
    return TimePoint(SysClock::time_point(std::chrono::milliseconds(ms)));
  }
  static TimePoint from_epoch_micros(std::int64_t us) {
    return TimePoint(SysClock::time_point(std::chrono::microseconds(us)));
  }

  SysClock::time_point time_point() const noexcept { return tp_; }
  std::int64_t seconds_since_epoch() const noexcept {
    return std::chrono::duration_cast<std::chrono::seconds>(tp_.time_since_epoch()).count();
  }
  std::int64_t milliseconds_since_epoch() const noexcept {
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp_.time_since_epoch()).count();
  }

  constexpr bool operator==(const TimePoint&) const noexcept = default;

 private:
  SysClock::time_point tp_{};
};

class Interval {
 public:
  Interval() = default;

  // Throws InvalidTime if end < start or the duration is absurd.
  Interval(TimePoint start, TimePoint end) : start_(start), end_(end) {
    validate();
  }

  TimePoint start() const noexcept { return start_; }
  TimePoint end() const noexcept { return end_; }

  Microseconds duration_us() const noexcept {
    auto d = end_.time_point() - start_.time_point();
    return Microseconds(static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(d).count()));
  }
  Milliseconds duration_ms() const noexcept {
    auto d = end_.time_point() - start_.time_point();
    return Milliseconds(static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(d).count()));
  }
  bool empty() const noexcept { return start_ == end_; }

  constexpr bool operator==(const Interval&) const noexcept = default;

  void validate() const {
    const std::int64_t secs =
        std::chrono::duration_cast<std::chrono::seconds>(end_.time_point() - start_.time_point())
            .count();
    if (secs < 0) throw InvalidTime("interval end precedes start");
    if (secs > kMaxIntervalSeconds) throw InvalidTime("interval is absurdly long");
  }

 private:
  TimePoint start_;
  TimePoint end_;
};

// ---------------------------------------------------------------------------
// SourceRef — which device source / backend supplied a value, and under which
// source generation (firmware, driver, or backend revision).
// ---------------------------------------------------------------------------
struct SourceRef {
  SourceId source_id;
  SourceGeneration source_generation;
  std::string backend;  // e.g. "nvml", "nvidia-smi", "rocm-smi", "custom"

  void validate() const {
    if (!source_id.has_value()) throw InvalidProvenance("source id is null");
    if (!source_generation.has_value())
      throw InvalidProvenance("source generation is null");
  }
};

// ---------------------------------------------------------------------------
// Provenance.
//
// Carries enough context to answer, for every number:
//   * what produced this number            -> producer
//   * from which device / source           -> device, source
//   * under what source generation         -> source.source_generation
//   * at what timestamp or interval        -> at / interval
//   * under what coordinator epoch         -> epoch
//   * what API / backend supplied it       -> source.backend
//   * whether it is direct or derived      -> trait of kind
//   * what confidence applies              -> confidence
//   * what inputs it was derived from      -> inputs (for DERIVED)
// ---------------------------------------------------------------------------
class Provenance {
 public:
  Provenance(DeviceId device, SourceRef source, CoordinatorEpoch epoch,
             ObservationKind kind, Confidence confidence)
      : device_(device), source_(std::move(source)), epoch_(epoch), kind_(kind),
        confidence_(confidence) {
    check_observation_kind(kind_);
    check_confidence(confidence_);
    // Enforce only the immutable core invariants here. Facets set afterwards
    // via the fluid setters (timestamp, interval, derivation inputs) are
    // validated by validate() once the value is fully built.
    if (kind_ != ObservationKind::Unknown) {
      if (!device_.has_value()) throw InvalidProvenance("device id is null");
      if (!epoch_.has_value()) throw InvalidProvenance("coordinator epoch is null");
      if (!source_.source_id.has_value())
        throw InvalidProvenance("source id is null");
      if (!source_.source_generation.has_value())
        throw InvalidProvenance("source generation is null");
    }
  }

  // Fluid setters for the optional facets.
  Provenance& producer(std::string v) { producer_ = std::move(v); return *this; }
  Provenance& at(TimePoint t) { at_ = t; return *this; }
  Provenance& interval(Interval i) { interval_ = i; return *this; }
  Provenance& add_input(ObservationId id) { inputs_.push_back(id); return *this; }
  Provenance& api(std::string v) { api_ = std::move(v); return *this; }

  // Full invariant validation. Called by consumers once all facets are set.
  void validate() const {
    if (kind_ != ObservationKind::Unknown) {
      source_.validate();
    }

    if (kind_ != ObservationKind::Unknown) {
      if (!device_.has_value()) throw InvalidProvenance("device id is null");
      if (!epoch_.has_value()) throw InvalidProvenance("coordinator epoch is null");

      if (!at_.has_value() && !interval_.has_value()) {
        throw InvalidProvenance(
            "observation carries neither a timestamp nor an interval");
      }
      if (at_.has_value() && interval_.has_value()) {
        const auto a = at_->milliseconds_since_epoch();
        const auto s = interval_->start().milliseconds_since_epoch();
        const auto e = interval_->end().milliseconds_since_epoch();
        if (a < s || a > e) {
          throw InvalidProvenance("timestamp lies outside the stated interval");
        }
      }
    }

    if (kind_ == ObservationKind::Derived && inputs_.empty()) {
      throw InvalidProvenance("DERIVED observation carries no derivation inputs");
    }
  }

  DeviceId device() const noexcept { return device_; }
  const SourceRef& source() const noexcept { return source_; }
  CoordinatorEpoch epoch() const noexcept { return epoch_; }
  ObservationKind kind() const noexcept { return kind_; }
  Confidence confidence() const noexcept { return confidence_; }
  const std::string& producer() const noexcept { return producer_; }
  const std::string& api() const noexcept { return api_; }
  const std::optional<TimePoint>& at() const noexcept { return at_; }
  const std::optional<Interval>& interval() const noexcept { return interval_; }
  const std::vector<ObservationId>& inputs() const noexcept { return inputs_; }

 private:
  DeviceId device_;
  SourceRef source_;
  CoordinatorEpoch epoch_;
  ObservationKind kind_;
  Confidence confidence_;
  std::string producer_;
  std::string api_;
  std::optional<TimePoint> at_;
  std::optional<Interval> interval_;
  std::vector<ObservationId> inputs_;
};

}  // namespace eobsv
