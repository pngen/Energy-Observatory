#pragma once

// Energy Observatory — version macros and library metadata.
// This is part of the Energy Observatory evidence and attribution runtime.

#define EOBSV_VERSION_MAJOR 1
#define EOBSV_VERSION_MINOR 0
#define EOBSV_VERSION_PATCH 0

#define EOBSV_VERSION_STRING "1.0.0"

// The version as a single comparable integer: major*10000 + minor*100 + patch.
#define EOBSV_VERSION_NUM   (EOBSV_VERSION_MAJOR * 10000 + EOBSV_VERSION_MINOR * 100 + EOBSV_VERSION_PATCH)

namespace eobsv {

// Human-readable version string ("1.0.0").
const char* version_string() noexcept;

}  // namespace eobsv
