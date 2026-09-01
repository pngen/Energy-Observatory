#include "framework.hpp"

#include "eobsv/eobsv.hpp"

#include <type_traits>

EOBSV_TEST(ids_default_is_empty) {
  eobsv::DeviceId id;
  EOBSV_ASSERT_TRUE(id.empty());
  EOBSV_ASSERT_FALSE(id.has_value());
}

EOBSV_TEST(ids_concrete_has_value) {
  eobsv::DeviceId id(7);
  EOBSV_ASSERT_TRUE(id.has_value());
  EOBSV_ASSERT_FALSE(id.empty());
  EOBSV_ASSERT_TRUE(static_cast<bool>(id));
  EOBSV_ASSERT_EQ(id.value(), 7ULL);
}

EOBSV_TEST(ids_zero_is_null) {
  eobsv::WorkerId w(0);
  EOBSV_ASSERT_TRUE(w.empty());
}

EOBSV_TEST(ids_equality_and_ordering) {
  eobsv::DeviceId a(1), b(2), c(1);
  EOBSV_ASSERT_TRUE(a == c);
  EOBSV_ASSERT_TRUE(a != b);
  EOBSV_ASSERT_TRUE(a < b);
  EOBSV_ASSERT_TRUE(b > a);
}

EOBSV_TEST(ids_are_distinct_types) {
  static_assert(!std::is_same_v<eobsv::DeviceId, eobsv::WorkerId>);
  static_assert(!std::is_same_v<eobsv::RequestId, eobsv::AttemptId>);
  static_assert(!std::is_same_v<eobsv::ExecutionId, eobsv::PhaseId>);
  static_assert(!std::is_same_v<eobsv::ObservationId, eobsv::EnergyRecordId>);
  // All identities share the same underlying value type.
  static_assert(std::is_same_v<eobsv::DeviceId::value_type, std::uint64_t>);
}

EOBSV_TEST(generations_default_empty) {
  eobsv::DeviceGeneration g;
  EOBSV_ASSERT_TRUE(g.empty());
  EOBSV_ASSERT_FALSE(g.has_value());
}

EOBSV_TEST(generations_first_is_one) {
  eobsv::CoordinatorEpoch e(1);
  EOBSV_ASSERT_TRUE(e.has_value());
  EOBSV_ASSERT_EQ(e.value(), 1ULL);
}

EOBSV_TEST(generations_increment) {
  eobsv::SourceGeneration g(1);
  ++g;
  EOBSV_ASSERT_EQ(g.value(), 2ULL);
  g++;
  EOBSV_ASSERT_EQ(g.value(), 3ULL);
}

EOBSV_TEST(generations_are_distinct_types) {
  static_assert(!std::is_same_v<eobsv::DeviceGeneration, eobsv::ObservationGeneration>);
  static_assert(!std::is_same_v<eobsv::CoordinatorEpoch, eobsv::PolicyGeneration>);
}
