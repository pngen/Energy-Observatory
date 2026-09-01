#pragma once

#include <stdexcept>
#include <string>
#include <utility>

namespace eobsv {

// Base class for all Energy Observatory domain errors.
// Energy Observatory rejects rather than silently coerces invalid inputs, so
// validation failures surface as exceptions derived from this type.
class Error : public std::runtime_error {
 public:
  explicit Error(const char* message) : std::runtime_error(message) {}
  explicit Error(std::string message) : std::runtime_error(std::move(message)) {}
};

// A physical quantity was rejected: NaN, infinity, a negative value where a
// negative value is physically impossible, an out-of-range utilisation, or an
// overflow.
class InvalidQuantity : public Error {
 public:
  using Error::Error;
};

// A value was tagged with a malformed / unsupported / inconsistent unit.
class InvalidUnit : public Error {
 public:
  using Error::Error;
};

// An enum value outside the declared enumerator set.
class InvalidEnum : public Error {
 public:
  using Error::Error;
};

// An impossible timestamp or a nonsensical interval (reverse / zero / absurd).
class InvalidTime : public Error {
 public:
  using Error::Error;
};

// Integer overflow, or a count/clock value that cannot be represented.
class Overflow : public Error {
 public:
  using Error::Error;
};

// Generation lineage is discontinuous or mismatched across a record boundary.
class GenerationMismatch : public Error {
 public:
  using Error::Error;
};

// An identity that is required to be concrete is null/unset, or is otherwise
// invalid in its context.
class InvalidIdentity : public Error {
 public:
  using Error::Error;
};

// Provenance is internally contradictory, for example a DERIVED observation
// that names no derivation input, or SYNTHETIC data mislabelled as MEASURED.
class InvalidProvenance : public Error {
 public:
  using Error::Error;
};

// An attribution graph references an entity that does not exist or that the
// record is not allowed to reference.
class InvalidAttribution : public Error {
 public:
  using Error::Error;
};

}  // namespace eobsv
