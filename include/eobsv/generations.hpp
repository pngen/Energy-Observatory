#pragma once

#include <cstdint>
#include <compare>

namespace eobsv {

// ---------------------------------------------------------------------------
// Strong typed generation counter.
//
// A Generation identifies *which incarnation* of a mutable entity a record
// refers to. It is a monotonic counter that is advanced when the entity is
// recreated or re-licensed (a new device generation, a new source generation,
// a new coordinator epoch). Generation 0 means "none"; the first concrete
// generation is 1. Two records referring to the same entity under different
// generations describe different states and must not be naively combined.
// ---------------------------------------------------------------------------
template <typename Tag, typename Underlying = std::uint64_t>
class Generation {
 public:
  using tag_type = Tag;
  using value_type = Underlying;

  constexpr Generation() noexcept = default;

  // Passing 0 yields the "none" generation; passing any other value is
  // accepted verbatim. This constructor is deliberately non-throwing so that
  // generations remain lightweight value types.
  constexpr explicit Generation(Underlying value) noexcept : value_(value) {}

  constexpr Underlying value() const noexcept { return value_; }

  constexpr bool has_value() const noexcept { return value_ != Underlying{0}; }
  constexpr bool empty() const noexcept { return value_ == Underlying{0}; }
  constexpr explicit operator bool() const noexcept { return has_value(); }

  // Advance to the next generation (wraps to 1 on overflow of 0).
  constexpr Generation& operator++() noexcept {
    value_ = (value_ == static_cast<Underlying>(-1)) ? Underlying{1}
                                                     : static_cast<Underlying>(value_ + 1);
    return *this;
  }
  constexpr Generation operator++(int) noexcept {
    Generation copy = *this;
    ++(*this);
    return copy;
  }

  constexpr bool operator==(const Generation&) const noexcept = default;
  constexpr auto operator<=>(const Generation&) const noexcept = default;

 private:
  Underlying value_{0};
};

#define EOBSV_DECLARE_GENERATION(NAME, TAG) struct TAG {}; using NAME = Generation<TAG>;

// How many generations a device has been through.
EOBSV_DECLARE_GENERATION(DeviceGeneration, DeviceGenerationTag)

// The coordinator epoch in which an observation was made.
EOBSV_DECLARE_GENERATION(CoordinatorEpoch, CoordinatorEpochTag)

// Generation of an observation's interpretation / derivation.
EOBSV_DECLARE_GENERATION(ObservationGeneration, ObservationGenerationTag)

// Generation of the energy accounting policy in force.
EOBSV_DECLARE_GENERATION(PolicyGeneration, PolicyGenerationTag)

// Generation of the energy record format / schema.
EOBSV_DECLARE_GENERATION(EnergyGeneration, EnergyGenerationTag)

// Generation of a provenance source (device firmware / driver / backend).
EOBSV_DECLARE_GENERATION(SourceGeneration, SourceGenerationTag)

#undef EOBSV_DECLARE_GENERATION

}  // namespace eobsv
