#include "framework.hpp"
#include "test_common.hpp"

#include "eobsv/eobsv.hpp"

EOBSV_TEST(provenance_measured_ok) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  auto p = eotest::make_provenance(eobsv::DeviceId(1), eotest::src(9),
                                   eobsv::SourceGeneration(2),
                                   eobsv::CoordinatorEpoch(3),
                                   eobsv::ObservationKind::Measured, tp);
  EOBSV_ASSERT_TRUE(p.kind() == eobsv::ObservationKind::Measured);
  EOBSV_ASSERT_TRUE(eobsv::is_direct(p.kind()));
  EOBSV_ASSERT_TRUE(p.device().value() == 1);
  EOBSV_ASSERT_TRUE(p.source().backend == "test-backend");
  EOBSV_ASSERT_TRUE(p.source().source_generation.value() == 2);
  EOBSV_ASSERT_TRUE(p.epoch().value() == 3);
}

EOBSV_TEST(provenance_derived_requires_inputs) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  // DERIVED with no derivation inputs must be rejected on final validation.
  auto p = eotest::make_provenance(eobsv::DeviceId(1), eotest::src(9),
                                   eobsv::SourceGeneration(2), eobsv::CoordinatorEpoch(3),
                                   eobsv::ObservationKind::Derived, tp);
  EOBSV_ASSERT_THROWS(p.validate(), eobsv::InvalidProvenance);
}

EOBSV_TEST(provenance_unknown_needs_no_source) {
  eobsv::SourceRef ref{eobsv::SourceId(0), eobsv::SourceGeneration(0), ""};
  eobsv::Provenance p{eobsv::DeviceId(0), ref, eobsv::CoordinatorEpoch(0),
                      eobsv::ObservationKind::Unknown, eobsv::Confidence::None};
  EOBSV_ASSERT_TRUE(p.kind() == eobsv::ObservationKind::Unknown);
}

EOBSV_TEST(provenance_measured_needs_timestamp_or_interval) {
  eobsv::SourceRef ref{eotest::src(9), eobsv::SourceGeneration(2), "nvml"};
  eobsv::Provenance p{eobsv::DeviceId(1), ref, eobsv::CoordinatorEpoch(3),
                      eobsv::ObservationKind::Measured, eobsv::Confidence::High};
  EOBSV_ASSERT_THROWS(p.validate(), eobsv::InvalidProvenance);
}

EOBSV_TEST(provenance_timestamp_outside_interval) {
  eobsv::SourceRef ref{eotest::src(9), eobsv::SourceGeneration(2), "nvml"};
  eobsv::Provenance p{eobsv::DeviceId(1), ref, eobsv::CoordinatorEpoch(3),
                      eobsv::ObservationKind::Measured, eobsv::Confidence::High};
  p.at(eobsv::TimePoint::from_epoch_seconds(2000));
  p.interval(eobsv::Interval(eobsv::TimePoint::from_epoch_seconds(1000),
                             eobsv::TimePoint::from_epoch_seconds(1500)));
  EOBSV_ASSERT_THROWS(p.validate(), eobsv::InvalidProvenance);
}

EOBSV_TEST(provenance_from_both_within_interval_ok) {
  eobsv::SourceRef ref{eotest::src(9), eobsv::SourceGeneration(2), "nvml"};
  eobsv::Provenance p{eobsv::DeviceId(1), ref, eobsv::CoordinatorEpoch(3),
                      eobsv::ObservationKind::Measured, eobsv::Confidence::High};
  p.at(eobsv::TimePoint::from_epoch_seconds(1200));
  p.interval(eobsv::Interval(eobsv::TimePoint::from_epoch_seconds(1000),
                             eobsv::TimePoint::from_epoch_seconds(1500)));
  p.validate();  // must not throw
}
