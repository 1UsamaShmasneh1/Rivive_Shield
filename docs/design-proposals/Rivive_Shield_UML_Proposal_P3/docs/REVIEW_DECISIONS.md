# Review decisions and evidence limits

## Newly approved scope decisions (user confirmation relayed by coordinating thread)
- Force-stop requires approval of the concrete action first, followed by a stopped-writer SQLite check, then a new separate approval before starting. A target that is already stopped also requires approval before start. A successful force-stop alone is never Recovered.
- Booking cancellation is deferred from this milestone. Prevention of overlapping bookings remains required. This approval does not authorize deleting historical P2 records.
- Local HTTP/1.1 GET/POST with serial requests is the initial scope; other/unsupported traffic remains Unknown. Actual React browser integration must be tested before acceptance.

- One active incident per target aggregates observations; no simultaneous actions and no reuse of an old approval for a new incident.
- Booking exposes list rooms, list bookings and create booking. Identical requests using the same ID do not create a second booking. JSON schema and status mapping remain open; changed content with the same ID is now approved to be rejected.

- Separate booking and incident databases are approved. Missing/corrupt existing databases must not cause silent replacement creation. Database verification runs after all writers stop. Journal/synchronous settings remain open.

- Atomic booking creation is approved: overlap check, request-ID/result persistence and deduplication in one write transaction, explicit rollback on failure, rejection of same ID with different data, and concurrency/process-stop tests.

- Flexible experimental limits are approved. Limit exceedance or insufficient evidence yields Unknown plus a warning, never automatic force-stop. Observation timeouts are determined by measurement, not arbitrary averages. Exact numbers remain open.

- The nine IncidentStatus names and their basic meaning are approved: Detected, Diagnosing, AwaitingApproval, Executing, Verifying, Recovered, NeedsReview, Declined, Unresolved. Exact transitions remain under review. Closing an incident with unresolved side effects is forbidden.

- Approved routes: GET /api/rooms, GET /api/bookings, POST /api/bookings. JSON/status/time-wire details are not approved by implication.

- Store times in UTC and display in the client device timezone; start must precede end and adjacent bookings are permitted. Exact wire format remains open.

- Approved desktop scope: target status, metadata-only activity, current incident/evidence/approvals, incident history. Monitoring/database I/O run in the background; closing desktop does not kill booking.

These confirmations approve scope, not every new class/signature or the full implementation baseline. No publication has occurred.

## Remaining review decisions
1. Review and accept exact public/private declarations and an exhaustive state-transition table, including closed-incident/manual-action behavior.
2. Set numeric resource limits and observation thresholds, queue length/retention, SQLite journal/synchronous policy, and repository dependency versions. Proposed starter choices are in `REMAINING_DECISIONS_AR.md`; they are not accepted defaults yet.
3. Review browser REST routes/status mapping, JSON schema, build/package steps, UI event binding and resources.
4. Mac adapters remain pending Ameen results and implementation checks. `MacConnectionObserver` may return Unsupported; it must not synthesize attribution.
5. Oracle remains later work requiring reviewed remote management/authentication/transport/database-verification design. Local adapters do not support that automatically.
6. All assignments/reviewers and RS-31 addition are user-approved; see ../APPROVED_ASSIGNMENTS_AR.md. Apply that approved allocation with final card inventory, without treating application to Trello as already completed.

## Evidence distinctions
- Windows test logs show observed feasibility, not code merged into the application.
- Current TCP table snapshots prove current endpoint ownership only during query interval. `SnapshotCandidate` is deliberately not `ProvenPacketOwner`.
- A late/no response is an observation, not proof of hang, cause, or permission to kill.
- A process exit code does not prove user intent. Record whether this controller requested termination.
- Process-crash SQLite tests are not power-loss tests. Integrity/FK checks are not booking-business-rule proofs.
- No payload is logged or replayed. Temporary parsing memory does contain payload bytes; never advertise payload-free capture.
- Mac v2 test bundle has no native Mac results yet. SKIP is not PASS.

## Intended implementation patterns (not extra framework requirements)
Ports/adapters for OS/library boundaries; repository for SQLite; observer/callback queues for captured metadata; pure state machine for lifecycle; command allowlist for actions; presenter/view split for desktop. Do not add a pattern merely for its name.
