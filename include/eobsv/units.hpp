#pragma once

#include <cstdint>
#include <compare>
#include <limits>
#include <type_traits>
#include <ostream>

#include "eobsv/errors.hpp"

namespace eobsv {

// ---------------------------------------------------------------------------
// Typed units & quantities.
//
// Every physical quantity in Energy Observatory carries an explicit typed
// unit. Two quantities of different units are distinct C++ types and cannot be
// added, subtracted, or compared without an explicit conversion. Construction
// validates the value: NaN, infinities, negative values where a negative value
// is physically impossible, out-of-range utilisation percentages, and overflow
// are all *rejected* rather than silently accepted or coerced.
//
// NOTE: validation is enforced at runtime (construction throws). The value
// arithmetic and unit conversions are intentionally not constexpr because they
// reject invalid values via exceptions. Ids/generations remain constexpr.
// ---------------------------------------------------------------------------

enum class unit {
  watts,
  joules,
  millijoules,
  watt_hours,
  milliseconds,
  microseconds,
  bytes,
  tokens,
  operations,
  utilization_percent,
  celsius,
  kelvin,
  clocks
};

constexpr const char* unit_name(unit u) noexcept {
  switch (u) {
    case unit::watts:              return "W";
    case unit::joules:             return "J";
    case unit::millijoules:        return "mJ";
    case unit::watt_hours:         return "Wh";
    case unit::milliseconds:       return "ms";
    case unit::microseconds:       return "us";
    case unit::bytes:              return "B";
    case unit::tokens:             return "tok";
    case unit::operations:         return "op";
    case unit::utilization_percent:return "%";
    case unit::celsius:            return "degC";
    case unit::kelvin:             return "K";
    case unit::clocks:             return "cyc";
  }
  return "?";
}

// Is a negative value physically valid for this unit?
constexpr bool allows_negative(unit u) noexcept {
  switch (u) {
    case unit::celsius:            return true;   // Celsius can be below zero.
    default:                       return false;  // Everything else is non-negative.
  }
}

// Range check (excludes NaN/inf which are handled at runtime for floating Rep).
constexpr bool range_ok(unit u, long double v) noexcept {
  if (u == unit::utilization_percent) {
    return v >= 0.0L && v <= 100.0L;
  }
  if (allows_negative(u)) {
    return true;
  }
  return v >= 0.0L;
}

// ---------------------------------------------------------------------------
// Quantity<U, Rep>
// ---------------------------------------------------------------------------
template <unit U, typename Rep = double>
class Quantity {
  static_assert(std::is_arithmetic_v<Rep>,
                "Quantity represents arithmetic physical quantities only");

 public:
  using value_type = Rep;
  static constexpr unit unit_v = U;

  constexpr Quantity() noexcept = default;

  explicit Quantity(Rep v) : value_(v) { validate_value(v); }

  // Prevent silent truncation when constructing an integer quantity from a
  // floating-point argument (e.g. Bytes(3.7) must not silently become 3).
  template <typename F,
            typename = std::enable_if_t<std::is_floating_point_v<F> &&
                                        std::is_integral_v<Rep>>>
  Quantity(F) = delete;

  constexpr Rep value() const noexcept { return value_; }

  constexpr bool operator==(const Quantity&) const noexcept = default;
  constexpr auto operator<=>(const Quantity&) const noexcept = default;

  // Same-unit arithmetic; the result is re-validated by the constructor.
  Quantity operator+(const Quantity& rhs) const { return Quantity(checked_add(value_, rhs.value_)); }
  Quantity operator-(const Quantity& rhs) const { return Quantity(checked_sub(value_, rhs.value_)); }
  Quantity operator-() const { return Quantity(checked_neg(value_)); }
  Quantity& operator+=(const Quantity& rhs) { *this = *this + rhs; return *this; }
  Quantity& operator-=(const Quantity& rhs) { *this = *this - rhs; return *this; }

  template <typename S> requires std::is_arithmetic_v<S>
  Quantity operator*(S s) const { return Quantity(checked_mul(value_, static_cast<Rep>(s))); }
  template <typename S> requires std::is_arithmetic_v<S>
  Quantity operator/(S s) const { return Quantity(checked_div(value_, static_cast<Rep>(s))); }

  friend std::ostream& operator<<(std::ostream& os, const Quantity& q) {
    os << q.value() << " " << unit_name(U);
    return os;
  }

 private:
  static void validate_value(Rep v) {
    if constexpr (std::is_floating_point_v<Rep>) {
      if (v != v) {  // NaN
        throw InvalidQuantity("quantity is NaN");
      }
      if (v == std::numeric_limits<Rep>::infinity() ||
          v == -std::numeric_limits<Rep>::infinity()) {
        throw InvalidQuantity("quantity is infinite");
      }
    }
    if (!range_ok(U, static_cast<long double>(v))) {
      throw InvalidQuantity(std::string("quantity out of range for unit ") + unit_name(U));
    }
  }

  // Portable overflow-checked arithmetic (no GCC-specific builtins).
  static Rep checked_add(Rep a, Rep b) {
    if constexpr (std::is_integral_v<Rep>) {
      const Rep maxv = std::numeric_limits<Rep>::max();
      if (b > 0 && a > maxv - b) throw Overflow("quantity overflow (add)");
      if (b < 0 && a < std::numeric_limits<Rep>::min() - b)
        throw Overflow("quantity overflow (add)");
      const Rep r = static_cast<Rep>(a + b);
      if (!range_ok(U, static_cast<long double>(r)))
        throw InvalidQuantity("quantity out of range after addition");
      return r;
    } else {
      return a + b;
    }
  }
  static Rep checked_sub(Rep a, Rep b) {
    if constexpr (std::is_integral_v<Rep>) {
      if (a < b && std::numeric_limits<Rep>::is_signed) {
        // Range check below rejects negative results for non-negative units.
        const Rep r = static_cast<Rep>(a - b);
        if (!range_ok(U, static_cast<long double>(r)))
          throw InvalidQuantity("quantity out of range after subtraction");
        return r;
      }
      const Rep r = static_cast<Rep>(a - b);
      if (!range_ok(U, static_cast<long double>(r)))
        throw InvalidQuantity("quantity out of range after subtraction");
      return r;
    } else {
      return a - b;
    }
  }
  static Rep checked_neg(Rep a) {
    if constexpr (std::is_integral_v<Rep>) {
      const Rep r = static_cast<Rep>(-a);
      if (!range_ok(U, static_cast<long double>(r)))
        throw InvalidQuantity("quantity out of range after negation");
      return r;
    } else {
      return -a;
    }
  }
  static Rep checked_mul(Rep a, Rep b) {
    if constexpr (std::is_integral_v<Rep>) {
      if (b != 0 && a > std::numeric_limits<Rep>::max() / b)
        throw Overflow("quantity overflow (multiply)");
      const Rep r = static_cast<Rep>(a * b);
      if (!range_ok(U, static_cast<long double>(r)))
        throw InvalidQuantity("quantity out of range after multiplication");
      return r;
    } else {
      return a * b;
    }
  }
  static Rep checked_div(Rep a, Rep b) {
    if (b == 0) throw InvalidQuantity("division by zero");
    if constexpr (std::is_integral_v<Rep>) {
      const Rep r = static_cast<Rep>(a / b);
      if (!range_ok(U, static_cast<long double>(r)))
        throw InvalidQuantity("quantity out of range after division");
      return r;
    } else {
      return a / b;
    }
  }

  Rep value_{};
};

// ---------------------------------------------------------------------------
// Named quantity aliases (typed units).
// ---------------------------------------------------------------------------
using Watts        = Quantity<unit::watts>;
using Joules       = Quantity<unit::joules>;
using Millijoules  = Quantity<unit::millijoules>;
using WattHours    = Quantity<unit::watt_hours>;
using Milliseconds = Quantity<unit::milliseconds>;
using Microseconds = Quantity<unit::microseconds>;
using Bytes        = Quantity<unit::bytes, std::int64_t>;
using Tokens       = Quantity<unit::tokens, std::int64_t>;
using Operations   = Quantity<unit::operations, std::int64_t>;
using Utilization  = Quantity<unit::utilization_percent>;
using Celsius      = Quantity<unit::celsius>;
using Kelvin       = Quantity<unit::kelvin>;
using Clocks       = Quantity<unit::clocks, std::int64_t>;

// ---------------------------------------------------------------------------
// Unit conversions (explicit, never implicit).
// ---------------------------------------------------------------------------
inline Joules      to_joules(Millijoules mj)    { return Joules(mj.value() / 1000.0); }
inline Millijoules to_millijoules(Joules j)     { return Millijoules(j.value() * 1000.0); }
inline WattHours   to_watt_hours(Joules j)      { return WattHours(j.value() / 3600.0); }
inline Joules      to_joules(WattHours wh)      { return Joules(wh.value() * 3600.0); }
inline Millijoules to_millijoules(WattHours wh) { return Millijoules(wh.value() * 3600000.0); }
inline Milliseconds to_milliseconds(Microseconds us) { return Milliseconds(us.value() / 1000.0); }
inline Microseconds to_microseconds(Milliseconds ms) { return Microseconds(ms.value() * 1000.0); }
inline Celsius     to_celsius(Kelvin k)         { return Celsius(k.value() - 273.15); }
inline Kelvin      to_kelvin(Celsius c)         { return Kelvin(c.value() + 273.15); }

// ---------------------------------------------------------------------------
// Cross-dimension derived quantities.
//   power * time = energy        (watts * ms -> J, watts * us -> mJ)
//   energy / time = power        (J / ms -> W)
// ---------------------------------------------------------------------------
inline Joules operator*(Watts w, Milliseconds ms) {
  return Joules(w.value() * (ms.value() / 1000.0));
}
inline Joules operator*(Milliseconds ms, Watts w) { return w * ms; }
inline Millijoules operator*(Watts w, Microseconds us) {
  return Millijoules(w.value() * us.value());
}
inline Millijoules operator*(Microseconds us, Watts w) { return w * us; }
inline Watts operator/(Joules j, Milliseconds ms) {
  return Watts(j.value() / (ms.value() / 1000.0));
}

}  // namespace eobsv
