#pragma once

#include <cstdio>
#include <exception>
#include <functional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace eotest {

struct AssertionFailure {
  std::string message;
};

struct TestCase {
  std::string name;
  std::function<void()> fn;
};

inline std::vector<TestCase>& registry() {
  static std::vector<TestCase> r;
  return r;
}

struct Registrar {
  Registrar(const char* name, std::function<void()> fn) {
    registry().push_back(TestCase{name, std::move(fn)});
  }
};

}  // namespace eotest

#define EOBSV_TEST(NAME)                                                static void eotest_##NAME();                                          static ::eotest::Registrar eotest_reg_##NAME(#NAME, eotest_##NAME);   static void eotest_##NAME()

#define EOBSV_ASSERT_TRUE(COND)                                                   do {                                                                              if (!(COND)) {                                                                    std::ostringstream oss;                                                         oss << __FILE__ << ":" << __LINE__ << ": expected true: " << #COND;             throw ::eotest::AssertionFailure{oss.str()};                                  }                                                                             } while (0)

#define EOBSV_ASSERT_FALSE(COND)                                                  do {                                                                              if ((COND)) {                                                                     std::ostringstream oss;                                                         oss << __FILE__ << ":" << __LINE__ << ": expected false: " << #COND;            throw ::eotest::AssertionFailure{oss.str()};                                  }                                                                             } while (0)

#define EOBSV_ASSERT_EQ(A, B)                                                     do {                                                                              const auto& eobsv_a_ = (A);                                                     const auto& eobsv_b_ = (B);                                                     if (!(eobsv_a_ == eobsv_b_)) {                                                    std::ostringstream oss;                                                         oss << __FILE__ << ":" << __LINE__ << ": expected equality:";                    oss << " left=" << eobsv_a_ << " right=" << eobsv_b_;                           throw ::eotest::AssertionFailure{oss.str()};                                  }                                                                             } while (0)

#define EOBSV_ASSERT_THROWS(EXPR, EXTYPE)                                         do {                                                                              bool eobsv_caught_ = false;                                                     try {                                                                             (void)(EXPR);                                                                 } catch (const EXTYPE&) {                                                         eobsv_caught_ = true;                                                         } catch (...) {                                                                   std::ostringstream oss;                                                         oss << __FILE__ << ":" << __LINE__                                                  << ": wrong exception type thrown for " << #EXPR;                           throw ::eotest::AssertionFailure{oss.str()};                                  }                                                                               if (!eobsv_caught_) {                                                             std::ostringstream oss;                                                         oss << __FILE__ << ":" << __LINE__ << ": expected exception " << #EXTYPE            << " not thrown by " << #EXPR;                                              throw ::eotest::AssertionFailure{oss.str()};                                  }                                                                             } while (0)
