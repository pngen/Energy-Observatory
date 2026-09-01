#include "framework.hpp"
#include "test_common.hpp"

#include "eobsv/eobsv.hpp"

EOBSV_TEST(observation_valid_power) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  auto prov = eotest::make_provenance(eobsv::DeviceId(1), eotest::src(9),
                                      eobsv::SourceGeneration(2),
                                      eobsv::CoordinatorEpoch(3),
                                      eobsv::ObservationKind::Measured, tp);
  eobsv::PowerObservation obs(eobsv::ObservationId(100),
                              eobsv::ObservationGeneration(1),
                              eobsv::Watts(75.0), prov);
  EOBSV_ASSERT_TRUE(obs.id().value() == 100);
  EOBSV_ASSERT_TRUE(obs.generation().value() == 1);
  EOBSV_ASSERT_EQ(obs.value().value(), 75.0);
  EOBSV_ASSERT_TRUE(obs.kind() == eobsv::ObservationKind::Measured);
}

EOBSV_TEST(observation_rejects_null_id) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  auto prov = eotest::make_provenance(eobsv::DeviceId(1), eotest::src(9),
                                      eobsv::SourceGeneration(2),
                                      eobsv::CoordinatorEpoch(3),
                                      eobsv::ObservationKind::Measured, tp);
  EOBSV_ASSERT_THROWS(
      eobsv::PowerObservation(eobsv::ObservationId(0), eobsv::ObservationGeneration(1),
                              eobsv::Watts(75.0), prov),
      eobsv::InvalidIdentity);
}

EOBSV_TEST(observation_rejects_null_generation) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  auto prov = eotest::make_provenance(eobsv::DeviceId(1), eotest::src(9),
                                      eobsv::SourceGeneration(2),
                                      eobsv::CoordinatorEpoch(3),
                                      eobsv::ObservationKind::Measured, tp);
  EOBSV_ASSERT_THROWS(
      eobsv::PowerObservation(eobsv::ObservationId(100), eobsv::ObservationGeneration(0),
                              eobsv::Watts(75.0), prov),
      eobsv::InvalidIdentity);
}

EOBSV_TEST(observation_rejects_negative_value) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  auto prov = eotest::make_provenance(eobsv::DeviceId(1), eotest::src(9),
                                      eobsv::SourceGeneration(2),
                                      eobsv::CoordinatorEpoch(3),
                                      eobsv::ObservationKind::Measured, tp);
  EOBSV_ASSERT_THROWS(
      eobsv::PowerObservation(eobsv::ObservationId(100), eobsv::ObservationGeneration(1),
                              eobsv::Watts(-1.0), prov),
      eobsv::InvalidQuantity);
}

EOBSV_TEST(observation_derived_carries_inputs) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  eobsv::SourceRef ref{eotest::src(9), eobsv::SourceGeneration(2), "aggregator"};
  eobsv::Provenance prov{eobsv::DeviceId(1), ref, eobsv::CoordinatorEpoch(3),
                         eobsv::ObservationKind::Derived, eobsv::Confidence::Medium};
  prov.at(tp);
  prov.add_input(eobsv::ObservationId(1));
  prov.add_input(eobsv::ObservationId(2));
  eobsv::EnergyObservation obs(eobsv::ObservationId(200), eobsv::ObservationGeneration(1),
                               eobsv::Millijoules(5000.0), prov);
  EOBSV_ASSERT_TRUE(obs.kind() == eobsv::ObservationKind::Derived);
  EOBSV_ASSERT_TRUE(obs.provenance().inputs().size() == 2);
}
