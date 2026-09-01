#pragma once

#include <utility>

#include "eobsv/generations.hpp"
#include "eobsv/ids.hpp"
#include "eobsv/provenance.hpp"
#include "eobsv/units.hpp"

namespace eobsv {

// ---------------------------------------------------------------------------
// Observation<QuantityT>
//
// A single observed datum: a typed, validated value classified as
// MEASURED / DERIVED / ESTIMATED / SYNTHETIC / UNKNOWN and stamped with a full
// provenance record. The generation carried by an observation is the
// observation reinterpretation generation: two observations of the same id
// under different generations represent different interpretations and are not
// combined.
// ---------------------------------------------------------------------------
template <typename QuantityT>
class Observation {
 public:
  using quantity_type = QuantityT;

  Observation(ObservationId id, ObservationGeneration generation, QuantityT value,
              Provenance provenance)
      : id_(id), generation_(generation), value_(value),
        provenance_(std::move(provenance)) {
    if (!id_.has_value()) throw InvalidIdentity("observation id is null");
    if (!generation_.has_value())
      throw InvalidIdentity("observation generation is null");
    provenance_.validate();
  }

  ObservationId id() const noexcept { return id_; }
  ObservationGeneration generation() const noexcept { return generation_; }
  const QuantityT& value() const noexcept { return value_; }
  const Provenance& provenance() const noexcept { return provenance_; }

  ObservationKind kind() const noexcept { return provenance_.kind(); }
  Confidence confidence() const noexcept { return provenance_.confidence(); }

  // Wall-clock vs device-active duration distinction lives on energy records;
  // an Observation is a single quantity reading.

 private:
  ObservationId id_;
  ObservationGeneration generation_;
  QuantityT value_;
  Provenance provenance_;
};

// Convenient typed observation aliases.
using PowerObservation    = Observation<Watts>;
using EnergyObservation   = Observation<Millijoules>;
using DurationObservation = Observation<Milliseconds>;
using UtilizationObservation = Observation<Utilization>;
using TemperatureObservation = Observation<Celsius>;

}  // namespace eobsv
