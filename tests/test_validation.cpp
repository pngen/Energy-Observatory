#include "framework.hpp"

#include "eobsv/eobsv.hpp"

#include <cstdint>

EOBSV_TEST(validation_parse_observation_kind_ok) {
  EOBSV_ASSERT_TRUE(eobsv::parse_observation_kind("MEASURED") ==
                    eobsv::ObservationKind::Measured);
  EOBSV_ASSERT_TRUE(eobsv::parse_observation_kind("derived") ==
                    eobsv::ObservationKind::Derived);
  EOBSV_ASSERT_TRUE(eobsv::parse_observation_kind("SYNTHETIC") ==
                    eobsv::ObservationKind::Synthetic);
}

EOBSV_TEST(validation_parse_observation_kind_bad) {
  EOBSV_ASSERT_THROWS(eobsv::parse_observation_kind("guessed"),
                      eobsv::InvalidEnum);
  EOBSV_ASSERT_THROWS(eobsv::parse_observation_kind(""), eobsv::InvalidEnum);
}

EOBSV_TEST(validation_check_observation_kind_invalid_underlying) {
  auto bad = static_cast<eobsv::ObservationKind>(99);
  EOBSV_ASSERT_THROWS(eobsv::check_observation_kind(bad), eobsv::InvalidEnum);
}

EOBSV_TEST(validation_check_confidence_invalid_underlying) {
  auto bad = static_cast<eobsv::Confidence>(55);
  EOBSV_ASSERT_THROWS(eobsv::check_confidence(bad), eobsv::InvalidEnum);
}

EOBSV_TEST(validation_timestamp_ok) {
  auto tp = eobsv::TimePoint::from_epoch_seconds(1700000000);
  EOBSV_ASSERT_EQ(tp.seconds_since_epoch(), 1700000000LL);
}

EOBSV_TEST(validation_timestamp_before_epoch) {
  EOBSV_ASSERT_THROWS(eobsv::TimePoint::from_epoch_seconds(-1), eobsv::InvalidTime);
}

EOBSV_TEST(validation_timestamp_too_far_future) {
  EOBSV_ASSERT_THROWS(eobsv::TimePoint::from_epoch_seconds(eobsv::kMaxTimestampSeconds + 1),
                      eobsv::InvalidTime);
}

EOBSV_TEST(validation_interval_ok) {
  auto s = eobsv::TimePoint::from_epoch_seconds(1000);
  auto e = eobsv::TimePoint::from_epoch_seconds(1500);
  eobsv::Interval iv(s, e);
  EOBSV_ASSERT_EQ(iv.duration_ms().value(), 500000.0);  // 500 s in ms
  EOBSV_ASSERT_FALSE(iv.empty());
}

EOBSV_TEST(validation_interval_reversed) {
  auto s = eobsv::TimePoint::from_epoch_seconds(1500);
  auto e = eobsv::TimePoint::from_epoch_seconds(1000);
  EOBSV_ASSERT_THROWS(eobsv::Interval(s, e), eobsv::InvalidTime);
}

EOBSV_TEST(validation_interval_absurd) {
  auto s = eobsv::TimePoint::from_epoch_seconds(1000);
  auto e = eobsv::TimePoint::from_epoch_seconds(1000 + eobsv::kMaxIntervalSeconds + 1);
  EOBSV_ASSERT_THROWS(eobsv::Interval(s, e), eobsv::InvalidTime);
}

EOBSV_TEST(validation_is_direct_classification) {
  EOBSV_ASSERT_TRUE(eobsv::is_direct(eobsv::ObservationKind::Measured));
  EOBSV_ASSERT_FALSE(eobsv::is_direct(eobsv::ObservationKind::Derived));
  EOBSV_ASSERT_FALSE(eobsv::is_direct(eobsv::ObservationKind::Estimated));
  EOBSV_ASSERT_FALSE(eobsv::is_direct(eobsv::ObservationKind::Synthetic));
  EOBSV_ASSERT_FALSE(eobsv::is_direct(eobsv::ObservationKind::Unknown));
}

EOBSV_TEST(validation_names) {
  EOBSV_ASSERT_TRUE(eobsv::observation_kind_name(eobsv::ObservationKind::Measured) ==
                    std::string_view("MEASURED"));
  EOBSV_ASSERT_TRUE(eobsv::unit_name(eobsv::unit::millijoules) == std::string_view("mJ"));
  EOBSV_ASSERT_TRUE(eobsv::confidence_name(eobsv::Confidence::High) ==
                    std::string_view("HIGH"));
}
