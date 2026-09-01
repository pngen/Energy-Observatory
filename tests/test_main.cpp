#include "framework.hpp"

#include <cstdio>
#include <exception>

int main() {
  std::size_t passed = 0;
  std::size_t failed = 0;
  for (auto& tc : eotest::registry()) {
    try {
      tc.fn();
      std::printf("[PASS] %s\n", tc.name.c_str());
      ++passed;
    } catch (const ::eotest::AssertionFailure& e) {
      std::printf("[FAIL] %s\n       %s\n", tc.name.c_str(), e.message.c_str());
      ++failed;
    } catch (const std::exception& e) {
      std::printf("[FAIL] %s\n       unexpected exception: %s\n", tc.name.c_str(),
                  e.what());
      ++failed;
    } catch (...) {
      std::printf("[FAIL] %s\n       unknown exception\n", tc.name.c_str());
      ++failed;
    }
  }
  std::printf("\n=== %zu passed, %zu failed, %zu total ===\n", passed, failed,
              eotest::registry().size());
  return failed == 0 ? 0 : 1;
}
