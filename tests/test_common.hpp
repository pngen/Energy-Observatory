#pragma once

#include "eobsv/eobsv.hpp"

namespace eotest {

// A reusable provenance for tests: MEASURED, device d, source s/backend b,
// epoch e, at timestamp ts.
inline eobsv::Provenance make_provenance(eobsv::DeviceId device,
                                         eobsv::SourceId source,
                                         eobsv::SourceGeneration source_gen,
                                         eobsv::CoordinatorEpoch epoch,
                                         eobsv::ObservationKind kind,
                                         eobsv::TimePoint at) {
  eobsv::SourceRef ref{source, source_gen, "test-backend"};
  eobsv::Provenance p{device, ref, epoch, kind, eobsv::Confidence::High};
  p.at(at);
  return p;
}

inline eobsv::SourceId src(std::uint64_t v) { return eobsv::SourceId(v); }

}  // namespace eotest
