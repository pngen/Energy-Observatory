# Security

Energy Observatory is a *trust* component: the energy evidence it records is
used to decide what work an accelerator may do. Integrity of that evidence is a
security property, not just a correctness concern.

## Trust model

- A value is only as trustworthy as its provenance. The runtime labels every
  number as MEASURED / DERIVED / ESTIMATED / SYNTHETIC / UNKNOWN and carries the
  device, source, source generation, coordinator epoch, time context, and
  confidence that produced it.
- Impossible physical values (NaN, infinity, negative energy or power,
  out-of-range utilisation, overflow, reversed or absurd intervals) are rejected
  rather than accepted or quietly corrected. This prevents a corrupted or
  maliciously crafted reading from silently poisoning a decision.
- Aggregates must reconcile with their total or the runtime fails loudly, so a
  partial or tampered record set cannot be silently replayed as a complete one.
- A DERIVED aggregate is always stamped as such and names its inputs; it is
  never relabelled as a direct measurement.

## Reporting a vulnerability

If you find a way to make the runtime accept an untrustworthy value, produce a
non-reconciling record without an error, or otherwise undermine the evidence
guarantee, please report it privately to the maintainer before disclosing it.
Include the version, the trigger, and the effect on the evidence guarantee.
