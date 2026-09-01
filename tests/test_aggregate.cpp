#include "framework.hpp"
#include "test_common.hpp"

#include "eobsv/eobsv.hpp"

namespace {
eobsv::Provenance make_prov() {
  eobsv::SourceRef ref{eotest::src(9), eobsv::SourceGeneration(2), "nvml"};
  eobsv::Provenance p{eobsv::DeviceId(1), ref, eobsv::CoordinatorEpoch(3),
                      eobsv::ObservationKind::Measured, eobsv::Confidence::High};
  p.at(eobsv::TimePoint::from_epoch_seconds(1700000000));
  return p;
}

eobsv::EnergyRecord rec(std::uint64_t id, eobsv::WorkloadId w, eobsv::PhaseId phase,
                        eobsv::EnergyComponent comp, double millijoules) {
  eobsv::EnergyAttribution a;
  a.workload = w;
  a.request = eobsv::RequestId(10);
  a.attempt = eobsv::AttemptId(20);
  a.execution = eobsv::ExecutionId(30);
  a.phase = phase;
  eobsv::EnergyRecord r(eobsv::EnergyRecordId(id), eobsv::EnergyGeneration(1),
                        eobsv::PolicyGeneration(1), a,
                        eobsv::Millijoules(millijoules), make_prov());
  r.component(comp);
  return r;
}
}  // namespace

EOBSV_TEST(aggregate_partition_reconciles) {
  eobsv::WorkloadId w(1);
  std::vector<eobsv::EnergyRecord> rs{
      rec(1, w, eobsv::PhaseId(1), eobsv::EnergyComponent::UsefulWork, 6000.0),
      rec(2, w, eobsv::PhaseId(2), eobsv::EnergyComponent::Overhead, 2000.0),
      rec(3, w, eobsv::PhaseId(2), eobsv::EnergyComponent::Waste, 1000.0),
      rec(4, w, eobsv::PhaseId(1), eobsv::EnergyComponent::IdleBackground, 1000.0),
  };
  auto totals = eobsv::partition_by_component(rs);
  EOBSV_ASSERT_EQ(totals.useful_work.value(), 6000.0);
  EOBSV_ASSERT_EQ(totals.overhead.value(), 2000.0);
  EOBSV_ASSERT_EQ(totals.waste.value(), 1000.0);
  EOBSV_ASSERT_EQ(totals.idle_background.value(), 1000.0);
  EOBSV_ASSERT_EQ(totals.total.value(), 10000.0);
}

EOBSV_TEST(aggregate_partition_rejects_missing_component) {
  eobsv::WorkloadId w(1);
  auto r = rec(1, w, eobsv::PhaseId(1), eobsv::EnergyComponent::UsefulWork, 1000.0);
  // Deliberately strip the component by constructing a record without one.
  eobsv::EnergyAttribution a;
  a.workload = w;
  a.execution = eobsv::ExecutionId(30);
  a.attempt = eobsv::AttemptId(20);
  eobsv::EnergyRecord no_comp(eobsv::EnergyRecordId(2), eobsv::EnergyGeneration(1),
                              eobsv::PolicyGeneration(1), a,
                              eobsv::Millijoules(1000.0), make_prov());
  std::vector<eobsv::EnergyRecord> rs{r, no_comp};
  EOBSV_ASSERT_THROWS(eobsv::partition_by_component(rs), eobsv::InvalidAttribution);
}

EOBSV_TEST(aggregate_partition_rejects_unreconciled) {
  eobsv::WorkloadId w(1);
  auto r = rec(1, w, eobsv::PhaseId(1), eobsv::EnergyComponent::UsefulWork, 1000.0);
  auto totals = eobsv::partition_by_component(std::vector<eobsv::EnergyRecord>{r});
  // Tamper with the total to force a reconcilable mismatch.
  auto bad = totals;
  bad.total = eobsv::Millijoules(9999.0);
  EOBSV_ASSERT_THROWS(bad.reconcile(), eobsv::InvalidQuantity);
}

EOBSV_TEST(aggregate_workload_summary) {
  eobsv::WorkloadId w(1);
  std::vector<eobsv::EnergyRecord> rs{
      rec(1, w, eobsv::PhaseId(1), eobsv::EnergyComponent::UsefulWork, 6000.0),
      rec(2, w, eobsv::PhaseId(2), eobsv::EnergyComponent::Overhead, 2000.0),
      rec(3, w, eobsv::PhaseId(3), eobsv::EnergyComponent::UsefulWork, 4000.0),
  };
  eobsv::SourceRef ref{eotest::src(9), eobsv::SourceGeneration(2), "aggregator"};
  eobsv::Provenance prov{eobsv::DeviceId(1), ref, eobsv::CoordinatorEpoch(3),
                         eobsv::ObservationKind::Derived, eobsv::Confidence::High};
  prov.at(eobsv::TimePoint::from_epoch_seconds(1700000000));
  prov.add_input(eobsv::ObservationId(1));
  eobsv::WorkloadEnergySummary s(w, rs, prov);
  EOBSV_ASSERT_EQ(s.total().value(), 12000.0);
  EOBSV_ASSERT_TRUE(s.by_phase().size() == 3);
  EOBSV_ASSERT_TRUE(s.by_request().size() == 1);
  EOBSV_ASSERT_TRUE(s.attempt_count() == 3);
  double frac = s.useful_fraction();
  EOBSV_ASSERT_TRUE(frac > 0.83 && frac < 0.84);
}

EOBSV_TEST(aggregate_workload_summary_must_be_derived) {
  eobsv::WorkloadId w(1);
  std::vector<eobsv::EnergyRecord> rs{
      rec(1, w, eobsv::PhaseId(1), eobsv::EnergyComponent::UsefulWork, 1000.0),
  };
  eobsv::SourceRef ref{eotest::src(9), eobsv::SourceGeneration(2), "aggregator"};
  eobsv::Provenance prov{eobsv::DeviceId(1), ref, eobsv::CoordinatorEpoch(3),
                         eobsv::ObservationKind::Measured, eobsv::Confidence::High};
  prov.at(eobsv::TimePoint::from_epoch_seconds(1700000000));
  EOBSV_ASSERT_THROWS(eobsv::WorkloadEnergySummary(w, rs, prov),
                      eobsv::InvalidProvenance);
}

EOBSV_TEST(aggregate_evidence_staleness) {
  eobsv::Evidence ev{eobsv::TimePoint::from_epoch_seconds(1700000000)};
  auto now = eobsv::TimePoint::from_epoch_seconds(1700000100);  // 100 s later
  EOBSV_ASSERT_TRUE(ev.stale_as_of(now, eobsv::Milliseconds(5000.0)));
  EOBSV_ASSERT_FALSE(ev.stale_as_of(now, eobsv::Milliseconds(200000.0)));
}
