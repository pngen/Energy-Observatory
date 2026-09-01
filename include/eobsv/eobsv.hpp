#pragma once

// Energy Observatory — public umbrella header.
//
// Energy Observatory is the evidence and attribution layer adjacent to Power
// Governor. It makes accelerator energy behaviour a first-class, replayable
// systems record: strong typed identities and generations, typed units with
// validation, a provenance-aware observation model, and workload/request/
// attempt/execution/phase energy attribution.

#include "eobsv/config.hpp"
#include "eobsv/errors.hpp"
#include "eobsv/ids.hpp"
#include "eobsv/generations.hpp"
#include "eobsv/units.hpp"
#include "eobsv/provenance.hpp"
#include "eobsv/observation.hpp"
#include "eobsv/energy.hpp"
#include "eobsv/aggregate.hpp"
