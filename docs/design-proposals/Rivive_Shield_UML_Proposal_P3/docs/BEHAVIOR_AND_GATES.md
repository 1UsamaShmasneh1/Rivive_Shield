# Behavior, integration gates and proposed first-milestone contract

## Application boundary
Two independently running native executables: booking backend and Rivive Shield desktop. React assets are built separately and served by the booking backend. Multiple native projects can be in one Windows solution; that is not evidence `.sln` builds on Mac. Neither executable links the other's internal application services. Booking backend has no dependency on Rivive Shield or monitoring-specific health callback.

Approved API routes: `GET /api/rooms`, `GET /api/bookings`, `POST /api/bookings`. The following payload/time representation and status mapping remain proposed. POST accepts an idempotency ID, room ID, UTC start/end and synthetic label. Explicit JSON parsing dependency and schema validation are an integration decision before implementation. Use 201 for creation, 200 for identical retry, 400 malformed input, 409 overlap/conflicting ID, 503 storage busy/unavailable; never expose SQL or paths. HTTP outcomes are not used as a universal health predicate.

Booking repository uses one write transaction to check same-room half-open overlaps and insert. All writers obey this path; SQL constraints/migrations enforce uniqueness and integrity. No public guard-disable switch. Interface deliberately has no generic SQL, delete, cancel or repair call.

## Resource and threading model
External pointer fields represent library resources, not hidden project PIMPL state. Resource-owning classes are noncopyable. Constructors may fail setup with an exception at composition; setup catches it, displays a bounded error and starts no monitor/process. Ordinary operations return Result. External destructors/release operations do not terminate the booking process silently.

wxWidgets/main thread owns window creation, event callbacks, render and destruction. One background owner serializes process operations, Monitor.tick and core processing. UI submits value commands/events; workers post immutable view copies back. UI must not call SQLite, pcap, waiting or process termination directly. `view()` reads published copies under the view mutex. Queue lock is never held across I/O. Every tick/operation honors deadline and cancellation; shutdown stops capture, joins worker, then frees views/resources. Closing desktop is not implicit permission to kill booking.

Initial start requires explicit operator approval of the displayed fixed target/executable/configuration before the UI may call `requestInitialStart()`. That method queues the approved setup intention; the background thread checks no active/pending durable incident, target state and immutable configuration. It does not return proof of process startup. During an incident only approved executor actions may change process state. Settings changes apply to a new controller session only while stopped and no active incident; existing plans are invalidated, never retargeted.

## Capture / parsing
Approved: configurable experimental limits; exceedance/insufficient evidence yields Unknown and warning, not automatic process termination. Observation thresholds must be measured; proposed numeric settings are not approved defaults.
The pcap filter is generated internally from validated loopback port; users cannot supply BPF text. Windows loopback link type and Mac lo0 type are checked; unfamiliar type returns Unsupported. The capture callback offers temporary bytes only. TCP decoding consumes them synchronously and owns only bounded library reassembly buffers. A conservative accepted-byte budget may abort earlier than a true memory cap, but never permits unbounded buffering.

Missing bytes, loss statistics unavailable/drops, midstream starts, buffer limits and parse failure preserve uncertainty. A textual missing-byte marker from the reassembly library is never HTTP input. Parser request/response state uses one outstanding request per flow. GET/POST body completion is callback-based, not packet count. No next request is accepted before preceding response completion. Body/URL/header content is never persisted; only bounded metadata is emitted.

Monotonic observation clocks measure intervals. They are not timestamps of internal handler execution. Capture UTC timestamps and wall clock may be used for evidence labels, not for execution-duration deadlines. One flow may have multiple serial request/response ordinals. EOF/time limit does not manufacture completion. Ordinary missing traffic means Unknown, not healthy or crashed.

## Process authority
Start uses the fixed reviewed executable/argument vector without a shell. Windows retains the creation handle. Mac retains an unreaped owned child and exclusive reaper discipline; never signal a reaped/reused PID. `ProcessIdentity` is checked against controller session/generation, and uncertainty blocks mutation. OS privilege errors are explicit; no automatic sudo, ACL or firewall changes.

Connection snapshots are optional corroborating metadata. HTTP evidence alone is insufficient for destructive action. A stop plan may be explicitly requested by the operator for the selected owned target, but still requires a separately displayed exact plan approval. Diagnostic rules cannot invent that manual intent.

## State and approval sequence
`Detected → Diagnosing → AwaitingApproval → Executing → Verifying → Recovered` is only the successful restart path. Unknown diagnosis leads Unresolved/no plan; rejected approval leads Declined; stale/expired approval cannot execute; uncertain side effects or persistence lead NeedsReview. ForceStop applied transitions to NeedsReview with stopped-target evidence, not Recovered. A follow-up start gets a new plan revision and a new approval. No silent recovery replay.

Store `Prepared` before dispatch. `claimDispatch` performs CAS Prepared→Dispatching in the same local incident store. Reload current authoritative plan/approval and target identity before invoking once. Duplicate event IDs are idempotent; mismatched payload with same ID is a conflict. A failed commit is not success. If process invocation may have started, return Uncertain rather than retryable failure. On controller restart, old session plans and in-flight attempts are NeedsReview; do not redispatch.

## Database and verification
Approved scope: separate booking/incident files; no silent replacement for missing/corrupt existing files; verification after all writers stop. This does not approve WAL/synchronous defaults or unresolved transaction-opening mode/API details.
Booking file and incident file cannot resolve to the same file. A DatabaseCheck is bound to configured DB key and stopped process generation. It requires confirmation no writers remain. Opening is read-only without CREATE; missing/invalid/busy yields failure/inconclusive. Preserve WAL/SHM/journal files, and do not claim byte-level forensic immutability. No code in the monitor repairs booking rows.

After forced stop: verify DB while no writers exist. A future start is gated by a current sufficient check. After successful start: verify correct new process plus independent later functional test evidence. A user-generated synthetic booking/test request is permitted by normal app behavior but is not silently replayed by the verifier. Structural DB PASS alone never yields Recovered. No functional evidence means Verifying/NeedsReview, not guessed success. Booking business invariants are tested independently.

## Persistence proposal
Use normalized local tables for incidents, immutable evidence metadata, plans, approvals, attempts, execution observations, verification results and accepted event IDs, with foreign keys and unique IDs. Atomic save/CAS updates aggregate revision; a dispatch claim consumes a prepared attempt once. Exact schema and migrations are a separate reviewed artifact, not invented by each card owner. Settings are local bounded non-secret JSON or equivalent atomic file, not cloud-shared by default.

## Minimum acceptance gates before implementation completion
Syntax compilation is only the first gate. Require constructor/resource lifetime tests, no double free/copy, deterministic state transition tests, duplicate/stale approval tests, loss/partial body/500/no-response tests, POST+serial keep-alive and unsupported paths, thread cancellation/shutdown and UI reentrancy tests, SQLite rollback/commit uncertainty and missing DB tests, owned-process generation tests, platform-specific permissions tests, independent review. Mac never inherits a Windows PASS. Integration and Oracle are separate gates.

## Explicit SQLite open mode
`SqliteConfiguration.openMode` is mandatory: OpenExisting fails if absent and never creates. InitializeNew is an explicit setup choice and fails if a database already exists; no overwrite or automatic new database on corruption. Schema compatibility is verified on open. This applies independently to booking and incident stores. DB verification never uses InitializeNew.

## Exact composition order / callback lifetime
The two executable `main` functions are composition roots, not additional business-service classes. They own all objects for the entire run. Automatic local lambdas/variables are implementation details of already approved methods/composition, not a license for new persistent state or helper classes.

Desktop sequence:
1. Construct validated immutable configs, external wx application and WxDesktopView on the UI thread.
2. Construct stores and platform process adapter, connection observer, policy, diagnosis, executor, verifier and pure state machine. Build IncidentOrchestrator.
3. Declare a local `Monitor* monitorBridge = nullptr` in the composition-root scope. Create HttpSessionTracker with a lambda that calls `monitorBridge->receiveHttp(value)` only when non-null. Create TcpStreamDecoder with a lambda calling tracker.accept; create PcapLoopbackCapture. Create Monitor with these dependencies and evidence callback that enqueues an IncidentEvent through the core (IDs/revisions allocated by the core/composition, never by library callback).
4. Assign `monitorBridge = &monitor` before capture can start. Construct ControllerRuntime with SettingsRepository and then presenter with runtime. Event binding uses these retained references. No capture/readBatch/callback before all wiring succeeds.
5. UI start request is queued only after explicit displayed-target approval. Background loop performs setup gate, then monitoring and core work. All parser/capture callbacks run synchronously in this background owner, not UI.
6. Stop/join runtime before clearing monitorBridge or destroying dependencies. Destroy in reverse order. Late wx callbacks are cancelled or ignored before views are destroyed. No lambda outlives captured references.

Backend composition: construct SqliteBookingRepository (reviewed open mode), BookingService, BookingHttpServer; React compiled assets live under staticAssetRoot. Run server; explicit shutdown closes server before repository. No connection to desktop composition.

## State table for process recovery
The exhaustive technical proposed policy is `../schemas/state_policy_proposed.json`; `../STATE_AND_API_REVIEW_AR.md` is its reader-facing summary. Unlisted transitions are rejected. Do not maintain a second competing transition table here. The nine state names/basic meanings are approved, but the transition table remains under review. No incident with unresolved side effects may close. Manual stop proposals require re-evaluation and a new explicit plan, never direct execution; the exact nonexecuting-state transitions must be finalized in that table before freezing implementation.

## Verification input provenance
`RecoveryVerifier.verify` receives the approved plan snapshot and authoritative prior stopped database check explicitly, in addition to attempt/execution and later evidence. The orchestrator loads these from the same durable incident record; it must not construct a replacement check from untrusted UI data. The verifier checks plan/attempt/execution IDs, configuration revision/database identity and prior generation binding; the new process generation must match execution. Missing or mismatched provenance is inconclusive/NeedsReview, never Recovered. Stop-only outcomes do not claim recovery regardless of the supplied check.


## Newly confirmed incident and booking invariants
One configured target has at most one active incident, with incoming observations aggregated into it. No simultaneous mutation actions are allowed. Approval matches the exact incident/plan identity; historical approval is never reusable by a new incident. An unresolved or uncertain attempt cannot be bypassed by creating another incident. Exact closure rules and exhaustive transition mapping remain review gates, not inferred from the high-level approval.

The three booking functions and identical-request ID deduplication are approved scope. Atomic deduplication belongs in createIfAvailable's transaction, not a React-only precheck. Exact wire schema/statuses/time representation and payload normalization remain proposed; different content under the same ID is rejected as approved. A duplicate must never create a second booking.

## Approved atomic booking invariants; remaining technical choices
The user approved atomic creation, explicit rollback, rejection of changed data under the same key, and concurrency/process-stop tests. This does not approve journal/synchronous or numeric retry settings. In `createIfAvailable`, deduplication and overlap prevention should execute within one write transaction. Enforce a unique request/booking ID in SQL, compare stored canonical request fields on reuse, return the existing result only for the same request, and reject different content under the same ID rather than overwrite it (rejection is approved; its exact HTTP mapping remains open).

Use an explicit rollback path for every pre-commit failure; do not assume a constraint violation rolled back the whole transaction. SQLite's default ABORT conflict handling backs out the failing statement, not necessarily earlier statements in the transaction: https://www.sqlite.org/lang_conflict.html . Determine transaction state before cleanup. Treat SQLITE_BUSY/LOCKED with a bounded reviewed wait/retry policy; do not return success or retry side effects blindly. Handle commit failure/uncertain outcome without claiming a definite successful booking; an eventual same-ID retry must resolve the durable stored result without duplication.

Before integration acceptance, test concurrent same-ID requests, same ID/different content, overlapping requests with distinct IDs, non-overlapping intervals, explicit rollback after statement failure, busy/locked paths, and controlled process crashes immediately before/after commit. Assert no duplicate booking and no overlap; distinguish committed durable state from responses observed by clients. These process-crash tests are not power-loss proof. Final schema, transaction-opening mode, retry limits and wire error mapping remain open.


## Approved time semantics
Persist UTC instants; the browser displays them in the client device timezone. Require start < end and permit adjacency using half-open intervals. Exact wire serialization/precision and ambiguous local-input handling still need schema review.


## Completed UI contract routing (technical proposal)
History and settings requests are queued, never queried/saved from GUI handlers. core.processPending services bounded history requests on the owner thread and publishes historyPage/historyOffset/pending/error under viewMutex_. runtime.processSettingsRequest validates stopped/no-active-incident before calling the store, then publishes result through core.publishSettingsResult. requestSettingsSave allows at most one pending request (Busy otherwise). A successful enqueue is not a successful save. Wire presenter to runtime after constructing both; stop/join before destroying either.
