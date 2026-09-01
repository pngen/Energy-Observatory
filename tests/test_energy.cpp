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

eobsv::EnergyAttribution attrib(eobsv::WorkloadId w) {
  eobsv::EnergyAttribution a;
  a.workload = w;
  a.request = eobsv::RequestId(10);
  a.attempt = eobsv::AttemptId(20);
  a.execution = eobsv::ExecutionId(30);
  a.phase = eobsv::PhaseId(40);
  return a;
}
}  // namespace

EOBSV_TEST(energy_record_valid_workload) {
  auto p = make_prov();
  eobsv::EnergyRecord rec(eobsv::EnergyRecordId(1), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), attrib(eobsv::WorkloadId(5)),
                          eobsv::Millijoules(12000.0), p);
  rec.power(eobsv::Watts(120.0));
  rec.component(eobsv::EnergyComponent::Execution);
  rec.active_duration(eobsv::Milliseconds(1000.0));
  rec.wall_duration(eobsv::Milliseconds(1200.0));
  EOBSV_ASSERT_EQ(rec.energy().value(), 12000.0);
  EOBSV_ASSERT_TRUE(rec.attribution().workload_attributed());
  EOBSV_ASSERT_TRUE(rec.attribution().concrete_execution());
  EOBSV_ASSERT_TRUE(rec.component() == eobsv::EnergyComponent::Execution);
}

EOBSV_TEST(energy_record_device_level) {
  eobsv::EnergyAttribution a;
  a.scope = eobsv::EnergyScope::DeviceLevel;
  a.eligibility = eobsv::EnergyEligibility::Unattributable;
  auto p = make_prov();
  eobsv::EnergyRecord rec(eobsv::EnergyRecordId(2), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), a,
                          eobsv::Millijoules(4000.0), p);
  EOBSV_ASSERT_FALSE(rec.attribution().workload_attributed());
}

EOBSV_TEST(energy_record_requires_workload_when_attributed) {
  eobsv::EnergyAttribution a;  // scope defaults to WorkloadAttributed
  auto p = make_prov();
  EOBSV_ASSERT_THROWS(
      eobsv::EnergyRecord(eobsv::EnergyRecordId(3), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), a, eobsv::Millijoules(1.0), p),
      eobsv::InvalidAttribution);
}

EOBSV_TEST(energy_record_phase_needs_execution) {
  auto a = attrib(eobsv::WorkloadId(5));
  a.execution.reset();
  auto p = make_prov();
  EOBSV_ASSERT_THROWS(
      eobsv::EnergyRecord(eobsv::EnergyRecordId(4), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), a, eobsv::Millijoules(1.0), p),
      eobsv::InvalidAttribution);
}

EOBSV_TEST(energy_record_retry_needs_attempt) {
  auto a = attrib(eobsv::WorkloadId(5));
  a.is_retry = true;
  a.attempt.reset();
  auto p = make_prov();
  EOBSV_ASSERT_THROWS(
      eobsv::EnergyRecord(eobsv::EnergyRecordId(5), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), a, eobsv::Millijoules(1.0), p),
      eobsv::InvalidAttribution);
}

EOBSV_TEST(energy_record_active_le_wall) {
  auto p = make_prov();
  eobsv::EnergyRecord rec(eobsv::EnergyRecordId(6), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), attrib(eobsv::WorkloadId(5)),
                          eobsv::Millijoules(1.0), p);
  rec.active_duration(eobsv::Milliseconds(1500.0));
  rec.wall_duration(eobsv::Milliseconds(1000.0));
  EOBSV_ASSERT_THROWS(rec.validate(), eobsv::InvalidTime);
}

EOBSV_TEST(energy_record_rejects_null_id) {
  auto p = make_prov();
  EOBSV_ASSERT_THROWS(
      eobsv::EnergyRecord(eobsv::EnergyRecordId(0), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), attrib(eobsv::WorkloadId(5)),
                          eobsv::Millijoules(1.0), p),
      eobsv::InvalidIdentity);
}

EOBSV_TEST(energy_record_retry_and_recovery_flags) {
  auto a = attrib(eobsv::WorkloadId(5));
  a.is_retry = true;
  a.is_recovery = true;
  a.pre_failure = true;
  auto p = make_prov();
  eobsv::EnergyRecord rec(eobsv::EnergyRecordId(7), eobsv::EnergyGeneration(1),
                          eobsv::PolicyGeneration(1), a, eobsv::Millijoules(900.0), p);
  EOBSV_ASSERT_TRUE(rec.attribution().is_retry);
}
