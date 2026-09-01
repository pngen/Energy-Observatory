# Contributing

Thanks for contributing to Energy Observatory. This is an evidence and
attribution runtime; the bar for a change is that it makes the record set more
trustworthy, more replayable, or easier to use.

## Ground rules

- Preserve the hard distinctions. Do not collapse measured into derived,
  device-level into workload-attributed, idle into active, or shared into
  exclusively attributable.
- Do not silently coerce. A value that cannot be reconciled must fail loudly.
- Keep units and identities strongly typed. Two different units or two
  different entity identities must never mix at the type level.
- Keep C++20 and compile clean with warnings treated as errors.

## Workflow

1. Fork and create a feature branch.
2. Add a focused test for any behavioural change.
3. Build and run the suite:

       cmake -S . -B build -G "Visual Studio 17 2022" -A x64
       cmake --build build --config Release
       ctest --test-dir build -C Release

4. Optionally validate under AddressSanitizer:

       cmake -S . -B build-asan -G "Visual Studio 17 2022" -A x64 -DEOBSV_ENABLE_SANITIZERS=ON
       cmake --build build-asan --config Debug --target eobsv_tests

5. Commit with a clear message. Do not add Co-authored-by trailers unless you
   mean it; this project does not add them by default.

## Style

- 2-space indent, 100-column soft limit.
- Headers under include/eobsv; source under src; tests under tests.
- Prefer constexpr for pure value types where it does not require throwing.
- Validate on construction and on an explicit validate().

## Reporting issues

Include the version, the compiler, the API surface used, and a minimal example.
For validation concerns, show the value that was rejected.
