#include "framework.hpp"

#include "eobsv/eobsv.hpp"

#include <limits>

EOBSV_TEST(unit_watts_valid) {
  eobsv::Watts w(100.0);
  EOBSV_ASSERT_EQ(w.value(), 100.0);
}

EOBSV_TEST(unit_joules_valid) {
  eobsv::Joules j(2.5);
  EOBSV_ASSERT_EQ(j.value(), 2.5);
}

EOBSV_TEST(unit_rejects_negative_power) {
  EOBSV_ASSERT_THROWS(eobsv::Watts(-1.0), eobsv::InvalidQuantity);
}

EOBSV_TEST(unit_rejects_negative_energy) {
  EOBSV_ASSERT_THROWS(eobsv::Joules(-0.001), eobsv::InvalidQuantity);
}

EOBSV_TEST(unit_allows_celsius_negative) {
  eobsv::Celsius c(-40.0);
  EOBSV_ASSERT_EQ(c.value(), -40.0);
}

EOBSV_TEST(unit_rejects_negative_kelvin) {
  EOBSV_ASSERT_THROWS(eobsv::Kelvin(-1.0), eobsv::InvalidQuantity);
}

EOBSV_TEST(unit_rejects_nan) {
  const double nan = std::numeric_limits<double>::quiet_NaN();
  EOBSV_ASSERT_THROWS(eobsv::Watts(nan), eobsv::InvalidQuantity);
}

EOBSV_TEST(unit_rejects_infinity) {
  const double inf = std::numeric_limits<double>::infinity();
  EOBSV_ASSERT_THROWS(eobsv::Joules(inf), eobsv::InvalidQuantity);
  EOBSV_ASSERT_THROWS(eobsv::Joules(-inf), eobsv::InvalidQuantity);
}

EOBSV_TEST(unit_utilization_bounds) {
  eobsv::Utilization ok(0.0);
  eobsv::Utilization full(100.0);
  EOBSV_ASSERT_EQ(full.value(), 100.0);
  EOBSV_ASSERT_THROWS(eobsv::Utilization(101.0), eobsv::InvalidQuantity);
  EOBSV_ASSERT_THROWS(eobsv::Utilization(-0.5), eobsv::InvalidQuantity);
}

EOBSV_TEST(unit_bytes_non_negative) {
  eobsv::Bytes b(1024);
  EOBSV_ASSERT_EQ(b.value(), 1024);
  EOBSV_ASSERT_THROWS(eobsv::Bytes(-1), eobsv::InvalidQuantity);
}

EOBSV_TEST(unit_tokens_operations) {
  eobsv::Tokens t(8000);
  eobsv::Operations o(42);
  EOBSV_ASSERT_EQ(t.value(), 8000);
  EOBSV_ASSERT_EQ(o.value(), 42);
}

EOBSV_TEST(unit_byte_overflow_on_add) {
  eobsv::Bytes big(std::numeric_limits<std::int64_t>::max());
  EOBSV_ASSERT_THROWS(big + eobsv::Bytes(1), eobsv::Overflow);
}

EOBSV_TEST(unit_conversion_millijoules_joules) {
  auto j = eobsv::to_joules(eobsv::Millijoules(5000.0));
  EOBSV_ASSERT_EQ(j.value(), 5.0);
  auto mj = eobsv::to_millijoules(eobsv::Joules(1.5));
  EOBSV_ASSERT_EQ(mj.value(), 1500.0);
}

EOBSV_TEST(unit_conversion_watt_hours) {
  auto j = eobsv::to_joules(eobsv::WattHours(1.0));
  EOBSV_ASSERT_EQ(j.value(), 3600.0);
  auto wh = eobsv::to_watt_hours(eobsv::Joules(3600.0));
  EOBSV_ASSERT_EQ(wh.value(), 1.0);
}

EOBSV_TEST(unit_power_times_time_is_energy) {
  eobsv::Watts w(200.0);
  eobsv::Milliseconds ms(1500.0);  // 1.5 s
  auto e = w * ms;
  EOBSV_ASSERT_EQ(e.value(), 300.0);  // J
}

EOBSV_TEST(unit_energy_over_time_is_power) {
  eobsv::Joules j(300.0);
  eobsv::Milliseconds ms(1500.0);
  auto p = j / ms;
  EOBSV_ASSERT_EQ(p.value(), 200.0);  // W
}

EOBSV_TEST(unit_addition_validates_result) {
  eobsv::Joules a(1.0), b(2.0);
  EOBSV_ASSERT_EQ((a + b).value(), 3.0);
}

EOBSV_TEST(unit_subtraction_cannot_go_negative) {
  eobsv::Joules a(1.0), b(2.0);
  EOBSV_ASSERT_THROWS(a - b, eobsv::InvalidQuantity);
}

EOBSV_TEST(units_are_distinct_types) {
  static_assert(!std::is_same_v<eobsv::Watts, eobsv::Joules>);
  static_assert(!std::is_same_v<eobsv::Joules, eobsv::Millijoules>);
  static_assert(!std::is_same_v<eobsv::Milliseconds, eobsv::Microseconds>);
}
