# Rivive Shield: proposed detailed C++17 contracts, revision P2

**FOR REVIEW ONLY. This is not implementation, an approved UML revision, or authority to change a main class.**

Derived from the original P1 proposal and latest user decisions. P1 historical baseline is not silently promoted to approval. See provenance_and_decisions.md.

## 1. Review boundary and change control

The source class diagram contains 16 named classes. Retained domain classes and unrelated contracts remain; exact removed, added and changed declarations are recorded in docs/P1_to_P2_header.diff. Existing untyped methods gain proposed parameter and return types. All constructors, virtual destructors, additional methods, concrete adapter classes, Booking/Monitor classes, value types, enums, aliases, private fields and visibility decisions are new proposals with the exact P1-to-P2 delta recorded separately.

The header is a declaration catalog, not a source-code starter that the team may silently complete. Before implementation, approve the revised UML and corresponding card changes. A new helper class, private field, helper method, overload, action, enum alternative, callback, service or interface also requires the same prior reviewed UML/card change. No PIMPL, opaque state container or unspecified helper-class extension point is proposed. Local variables and calls to approved standard/library APIs are implementation details, but must not become unreviewed persistent fields or new project helpers.

Standard/compiler-generated behavior is limited to ordinary value copying, moving, assignment and destruction for value structs, and language-generated special members where legal. It does not authorize custom business behavior. Interfaces have explicit virtual destructor declarations. SQLite and process-owning adapters explicitly delete copying; mutex/reference members naturally prevent inappropriate generated copies/moves. Do not add custom constructors/destructors/copy behavior without review. `sqlite3` and `std::*` types are external library types, not undisclosed project helper classes.

The inventory has two distinct layers: domain/module/service classes and plain boundary-value structs. Values carry the data needed to make approval, persistence and verification reproducible; they are not additional business modules or lifecycle controllers.

## 2. Shared representation and failure rules

- Language is C++17. `Result<T>` is `std::variant<T, Error>`; success is the T alternative. `Status` succeeds only as `std::monostate`. No numeric truthiness convention. Ordinary operational failures are returned, not thrown. Constructor/configuration failures use `SetupError`; allocation failure may still throw standard exceptions. Destructors must not throw.
- Identifier aliases are strings, not separate strong C++ types. Boundaries validate nonempty ASCII identifiers matching `[A-Za-z0-9_-]{1,64}`. An alias is not proof of validation. IDs are unique in their entity namespace; plan identity is `(incidentId, planId, revision)`. No identifiers are executable input.
- Synthetic labels are nonempty UTF-8, bounded by a reviewed limit (not yet selected); diagnostic summaries at most configured byte limits. Only fixture-approved room IDs are accepted. No real names, credentials or connection metadata enter evidence.
- Versions start at 1; 0 denotes an absent record/no current plan when a contract explicitly permits it. Increment on a successful committed change; never wrap. Overflow is `LimitExceeded`, without mutation.
- `UtcTime` is UTC milliseconds for persisted ordering/audit. `Deadline` is monotonic, local and never serialized. Persisted expiry is wall-clock metadata; after restart all pending attempts are reconciled and not replayed, even if their timestamp suggests time remains.
- All boundaries reject invalid variants, IDs, enum values decoded from storage/network, stale versions, mismatched evidence and malformed timestamps. Deserialize strictly with bounded sizes. No unchecked cast can make an unknown action valid.
- The Result success alternative does not automatically mean recovery. `ExecutionOutcome::Succeeded` means the bounded action is known to have executed, not that independent checks passed. `VerificationResult.passed` is true if and only if `outcome == Passed`.
- `Error::message`, failure observations and audit summaries must be sanitized. Errors never grant an automatic retry. `OutcomeUncertain` means stop mutation and reconcile; it is not a reason to repeat.

## 3. Ownership, processes and threading

The booking service and control program are separate processes. Stopping the demo child cannot stop Monitor, the core or the CLI. The composition root owns all concrete services and adapters; constructor references are non-owning and must outlive their consumers. Values passed into records are owned copies. Never store borrowed references to stack request objects. `Now` is an injected clock callable owned by value, not a scheduling service.

`IncidentOrchestrator` owns one optional active record and its state-machine value. Its constructing thread is the sole transition owner. `handleEvent`, `submitApproval`, `processPendingEvents`, `reconcileAfterRestart` and `currentView` run on that owner thread. Off-thread calls to those methods fail `Busy`; external callbacks may only call `enqueueEvent`, which copies the event under `queueMutex_`. Capacity is explicit and positive; no numeric default is approved. A full queue returns `LimitExceeded`; it never silently drops an accepted event. `processPendingEvents(limit)` drains at most that many events; errors remain visible.

The owner invokes bounded synchronous module methods in the initial local implementation. These methods honor explicit deadlines and finite observation/data limits. This is not a thread-pool framework. A timeout does not prove an external mutation stopped. The queue mutex must never be held during diagnosis, process, HTTP or SQLite work. This design can receive concurrent callbacks without allowing them to mutate lifecycle state.

Repositories serialize each owned SQLite connection with their own mutex. Executor uses a short single-dispatch lock plus durable CAS. The fixed-target adapter serializes process actions and its consumed-dispatch list. NeedsReview reserves the one-demo slot and blocks new mutation; it is not a terminal outcome that silently frees the slot. `loadPendingRecords` includes NeedsReview and any unresolved attempt, not only nominal in-progress states. The diagnostic agent, domain service, CLI and verifier are owner-thread confined unless the documented read-only method is called through the core's bounded flow. There is one configured demo and one nonterminal incident, not a distributed scheduler.

## 4. Booking and SQLite transaction contract

`BookingService.validate` checks IDs, allowed room, synthetic label and a positive bounded interval. `overlaps(a,b)` is exactly `a.start < b.end && b.start < a.end`; only active same-room rows conflict. Adjacent intervals are permitted. Ordering is lexicographic `(createdAt, bookingId)` with bytewise ASCII ID tie-breaking. Creation time is supplied by the service clock, never accepted from a participant request.

`createBooking` is one atomic transaction. An existing ID with identical original request returns that same row without changing state; an existing ID with different payload returns `Conflict`. This retry does not prove a fresh successful normal-use test. A new active row starts at version 1. With guard enabled, overlap detection and insert are within the same write transaction, so two simultaneous conflicting inserts cannot both commit. Guard rejection is specifically `OverlapRejected`, not a generic SQL, availability or ID-conflict error. With the intentionally vulnerable initial guard-disabled fixture, the approved logical challenge can create overlap through the normal booking API. There is no public disable-guard operation.

`cancelBooking` checks row version in the transaction. It permits participant or verification-probe cancellation only at the relevant trusted boundary. It must reject requests to assign `Active` or `CancelledConflict`; only `restoreInvariant` may assign the conflict-repair status. A successful cancellation retains all data, sets the reason/status and increments row/store versions once. Idempotent replay returns existing result only if the exact original cancellation is identifiable; otherwise stale versions fail.

`restoreInvariant(args, attemptId, now)` uses a single SQLite write transaction:

1. Check the store revision, both row versions and IDs, guard state and existence. `expectedBookingRevision` must match the plan's Version alternative exactly.
2. Independently enumerate the entire active conflict set under the transaction. It must be exactly one pair, the specified earlier/later rows, ordered by creation time then ID, same room, actually overlapping. Any additional conflict, changed record, already active guard or failed precondition rejects the operation without mutation.
3. Preserve the earlier row byte-for-byte in business fields and version. Change only the later row to `CancelledConflict` (serialized as `cancelled_conflict`), preserving its data; set the fixed audit reason identifying the earlier booking and attempt; increment that row version once.
4. Activate the overlap guard in the same transaction, advance store revision once, and persist a `BookingRepairReceipt` keyed by attempt ID containing exact arguments and before/after data. No broad reset, deletion or replacement of unaffected rows.
5. Commit all changes together. Rollback on known failure. An uncertain commit is `OutcomeUncertain`, not success or safe retry. `loadRepairReceipt` is a read-only reconciliation route; it does not execute repair.

The proposal uses separate local SQLite booking and incident stores. There is no cross-store transaction or universal exactly-once claim. Incident prepared/dispatch records close the unsafe replay path, and the logical repair receipt supports read-only reconciliation. SQLite transaction mode, synchronous durability, busy timeout, schema and migrations still require an implementation-level reviewed design/card before coding; the declarations are not evidence that SQLite is working.

## 5. Evidence, monitoring and compiled-in rules

`readHealthAndEvidence` returns bounded, timestamped observations of the fixed child and a consistent booking snapshot, even while the booking HTTP service is stopped. A positive process observation can establish `Stopped`; an HTTP timeout alone cannot. Failure observations represent failed reads, not evidence that a target is healthy. Evidence IDs are immutable: identical duplicates can be reused; the same ID with different content is `Conflict`.

`Monitor.poll` obtains real observations and calls Monitor's own independent overlap scan. It returns `DetectionNotice` only for supported independently observed failure; no failure produces an empty evidence notice that must not create an incident. The monitor's algorithm must not reuse the booking guard's boolean or call `BookingService::overlaps`, so the test can catch guard defects. `classifyChallenge` accepts the actual typed booking result, not a caller-supplied success/guard boolean. GuardRejected requires actual `OverlapRejected` plus independent unchanged/valid state; transport/SQL/input failures remain Unresolved or ObjectiveNotMet.

DiagnosticAgent contains compiled-in rules in evaluateRules. rulesetVersion identifies the code/rule revision; adding or changing a rule requires source modification, rebuild, review and tests. There is no runtime rule file, provider, credentials or network AI call. The private helper declarations are listed in the UML exactly.

Rules consume actual bounded evidence and never a challenge selector, injected fault name or expected test answer. ServiceStopped requires positive owned-process stopped evidence with matching generation, not a failed HTTP request. BookingOverlap requires the exact independently observed pair and snapshot revisions under the existing one-pair logical repair scope. Unsupported, incomplete, stale or conflicting evidence must yield Unresolved without a plan. Simultaneous stop/overlap must not pick one arbitrarily. The agent makes only bounded read-only observations, calls the independent Monitor scan where needed, constructs a typed draft, and validates any proposed plan through PlanPolicy before presenting it.

RuleAudit records rulesetVersion, optional matchedRuleId, immutable evidence references, evaluation time and bounded summary. No match is represented by an absent matchedRuleId, not fake AI success. Audit and outcome must be persisted with the incident. DiagnosticLimits bounds reads, evidence, text and overall deadline; concrete values remain open. No AI call counts or provider error types remain.

## 6. Deterministic plan and approval contract

`PlanPolicy.validate` requires:

- Only `TargetId::DemoBookingService` and the two declared actions.
- Exact action/argument/version-tag agreement: Restart uses `RestartArgs` and `ProcessGeneration`; Restore uses `RestoreInvariantArgs` and the booking Version alternative.
- Current incident and plan revision, valid expiry, existing immutable evidence IDs, a baseline booking snapshot, and independently supported cause. A stop plan needs actual stopped-process evidence, not merely an error string; a restore plan needs the exact observed conflict pair and versions.
- `VerificationRecipe` stored inside the immutable plan, with unique fresh probe IDs, fixture-approved room/time/labels and the only proposed cleanup/preservation policies. Logical repair requires both original and variant conflict probes; restart omits them. A fresh normal probe must be nonconflicting with the intended repaired state.

`Approval.matchesPlan` checks incident/plan/revision identity only. It is a necessary helper, never sufficient authorization. `revalidate` also checks the current stored plan's full typed content, the exact stored approval, expiry and current lifecycle. The caller cannot reuse an ID/revision while changing action, arguments, evidence or verification recipe. Approval's expected incident revision must match the AwaitingApproval snapshot at acceptance. After the core saves Repairing, the executor checks that the persisted decision was accepted against that preceding revision and belongs to the exact Prepared attempt; it does not incorrectly require the pre-approval revision to equal the post-transition revision.

The CLI displays action, fixed target, record IDs, effect, evidence, expected versions, verification requests and cleanup before requesting an explicit operator decision. Approval comes only from the human operator. Rejection/cancellation while awaiting approval records the decision and performs no repair. Cancellation after dispatch may have started must not claim that the action was undone.

## 7. Durable dispatch and restart sequence

1. Core accepts a new event only against its incident and current plan revision. It rejects stale/illegal events. An already accepted EventId is a no-op, never a second dispatch. Future callbacks retain the exact original incident/plan revisions and attempt identity.
2. Core receives the displayed plan's approval. After policy checks, it creates one new attempt ID, saves approval plus `Prepared` attempt plus `Repairing` status and accepted event ID atomically using `saveRecord`. No mutation before successful commit. `expectedRecordVersion=0` is insertion; later writes compare the exact loaded version. The repository returns the new committed version.
3. Executor loads the authoritative record, compares caller plan/approval with complete stored values, checks the exact attempt and command preconditions, then calls `claimDispatch` with its loaded record version. That transaction changes only the matching Prepared attempt to Dispatching, records time and increments recordVersion. A duplicate, stale, expired or uncertain claim never invokes a command. There is no lease expiry that grants a retry.
4. After a confirmed claim, invoke exactly one registered command. The adapter checks the stored dispatch receipt, exact approved action and current target version again. In-process duplicate receipts are rejected through `consumedDispatches_`, set before any possible mutation. Logical changes additionally recheck inside their own database transaction. A local restart delegates only to the injected OwnedDemoProcess under the adapter control lock. Windows uses the retained OS process handle, never a PID lookup/reopen; observation/termination must refer to that same handle, which is not closed/replaced until all possible operations on that generation finish. macOS requires a child created by this owner, no auto-reaping/SIGCHLD handler/external wait, and exclusive controlled reaping. Do not signal a reaped PID. Observation may report an exited child without reaping while a last signal decision is pending. Lost ownership or ambiguous start/stop marks uncertainty and fails closed. Both platform adapters must have one lifetime owner; a new controller never adopts an old process automatically. Logical ProcessGeneration is not OS ownership proof. Native implementation and race behavior are still untested design gates. Configured executable/arguments are immutable and never interpreted by a shell.
5. Before invocation, a normal Result error guarantees no invocation occurred. Once invocation might have begun, return `ExecutionResult` with Succeeded, KnownFailure or Uncertain. Succeeded is not Recovered. Timeout, transport loss, interrupted commit or ambiguous process start produces Uncertain.
6. **Core reloads the record after executor dispatch before saving the execution/verification outcome.** `claimDispatch` incremented recordVersion. Merge the new result into the authoritative record without dropping the claimed attempt or other persisted audit data, then CAS that reloaded version. Never save the old pre-dispatch aggregate over it.
7. A known successful action enters Verifying. A known failure enters Failed. An uncertain action enters NeedsReview; do not reissue the attempt, even if read-only evidence later looks healthy.
8. At startup, `reconcileAfterRestart` runs before accepting any new mutable workflow. Load every nonterminal/pending record, query actual target/receipt evidence, save a reconciliation result and put interrupted work in NeedsReview conservatively. Prepared also is not blindly replayed. A fresh controller session invalidates old process generations. If load/persistence fails, expose a nondurable blocked view and perform no mutation.

A persistence failure latches `repairsBlocked_`. The core reports `OperatorView.durable=false`, `repairsBlocked=true` and a visible typed failure; the persisted old state is not falsely overwritten or described as saved. The volatile NeedsReview indication is explicitly different from a durably saved state. A no-longer-running pending attempt is still not permission to repeat it. NeedsReview has no automatic retry/reopen route in this proposal; any resumed repair requires separately reviewed workflow, fresh evidence, a new displayed plan and approval.

`startForSetup` is allowed only before an active incident/attempt exists and after loading the store successfully. `stopForControlledChallenge` is a restricted lab route, never exposed to the diagnostic agent. Both check the single-demo ownership/active-record gate. A destructor releases owned resources but must not silently terminate the running target as an unapproved cleanup side effect.

## 8. Independent verification and preservation

Verification uses the exact persisted `RepairPlan.verification` recipe. `VerificationContext.baseline` must match `baselineSnapshotRef` byte-for-byte; callers cannot substitute a more favorable baseline. Adapter verification writes are restricted to the stored plan's exact probe IDs and request payloads, the current Verifying attempt and the fixed target. Cancellation may affect only rows actually created for that attempt's probes. No arbitrary booking cancellation is allowed.

The normal-use check makes a real request through the booking service, confirms the newly committed active row with an independent read, then cancels that probe with `CancelledVerificationProbe` and retains its audit row. A pre-existing identical ID is an idempotent replay, not a fresh successful test. If cleanup fails or its outcome is uncertain, recovery fails or enters NeedsReview; it cannot quietly leave an unexplained active probe. A probe HTTP timeout may follow a committed insert. Reconcile the exact approved unique probe ID with read-only observation first; never blindly resend a probe or issue speculative cleanup after an uncertain response. Preserve any created row and its audit data. If existence, status or cleanup outcome is still uncertain, enter NeedsReview and do not perform another write. A storage-failure latch prohibits even cleanup writes.

For logical repair, original and variant requests must both reach the real guarded booking service. Original repeats the later conflicting interval with a fresh probe ID; variant uses meaningfully different overlapping bounds (not just a different string). Only explicit `OverlapRejected` plus independent evidence of no inserted row counts as a guard pass. If a broken guard accepts a probe, retain failure evidence and cancel only that newly created probe; cleanup does not turn the failed guard test into a pass.

`verifyHealth` returns ServiceHealth. `verifyNormalBooking` returns NormalBooking plus ProbeCleanup. `verifyDataAndInvariant` returns DataPreservation and BookingInvariant; for logical repair it additionally returns GuardActive, OriginalConflictRejected and VariantConflictRejected, including their cleanup observations. Batch identity must match incident and attempt. The core merges evidence first and validates all evidence references.

Required check set:

- Restart: ServiceHealth, NormalBooking, DataPreservation, BookingInvariant, ProbeCleanup.
- Logical restore: all above plus GuardActive, OriginalConflictRejected, VariantConflictRejected.

Aggregate exactly one final result per required CheckId. Internal per-probe cleanup observations combine into one ProbeCleanup result, which passes only if all created probes were confirmed cancelled and retained. Missing, duplicate contradictory, unexpected, stale-attempt, unresolved-reference, Failed, Inconclusive or NotRun results cannot recover. Do not treat an empty vector or the latest batch as all checks passing. If a bounded check cannot run, record Inconclusive/NotRun and a FailureObservation.

Allowed data deltas are deterministic, not diagnosis-defined:

- Restart: every baseline booking row, its business fields, row version and guard state remains unchanged, except the exact newly added approved probe rows.
- Restore: earlier and every unaffected baseline row are unchanged. Only the specified later row has status/reason/version changed exactly as the repair contract permits; guard becomes enabled. Then only the exact approved new probe rows may be added.
- All actually created probe rows remain as cancelled verification audit rows, with their original request data and prescribed cancellation/version changes. Successfully rejected probes create no booking row. Store revision may advance for these explicit transactions; unexpected rows, missing baseline rows or arbitrary version changes fail preservation.

Finally the core saves execution, all results/evidence and `Recovered` atomically. Only a confirmed commit permits a durable Recovered view. Independent checks do not include a circular 'record saved' boolean supplied by the verifier; persistence is confirmed by the repository's actual SaveReceipt.

## 9. State-transition completion

All original edges are retained. Every accepted transition increments incident revision once; record-only dispatch claims increment recordVersion without inventing a lifecycle transition. State-machine `applyEvent` is pure and returns a candidate; the core updates its state-machine value only after the corresponding durable save succeeds.

| From | Trigger | To | Provenance |
| --- | --- | --- | --- |
| No record | Independent DetectionNotice | Detected | Original initial edge; typed event proposed |
| Detected | BeginDiagnosisNotice | Diagnosing | Original edge; typed event proposed |
| Diagnosing | Supported, validated persisted plan | AwaitingApproval | Original |
| Diagnosing | Insufficient evidence/observation unavailable/diagnosis deadline | Unresolved | Original category, explicit timeout refinement |
| AwaitingApproval | Current explicit approval + Prepared durable attempt | Repairing | Original |
| AwaitingApproval | Explicit reject/cancel | Declined | Original |
| Repairing | Known successful execution | Verifying | Original |
| Repairing | Known execution failure | Failed | Original |
| Repairing | Timeout/interruption/uncertain outcome | NeedsReview | Original |
| Verifying | Complete required checks + final commit | Recovered | Original |
| Verifying | Known failed/incomplete check | Failed | Original |
| AwaitingApproval / Verifying | Required persistence failure | Volatile NeedsReview, mutations blocked | Original dashed edge clarified: failure cannot manufacture durable state |
| AwaitingApproval | Plan expires before approval | Unresolved | NEW proposed edge; no fabricated operator rejection |
| Verifying | A verification write/cleanup may have executed, outcome uncertain | NeedsReview | NEW proposed edge |
| Detected / Diagnosing / Repairing | Required persistence failure | Volatile NeedsReview, mutations blocked | NEW explicit fail-closed completion |
| Any persisted nonterminal state after process restart | Reconciliation, no dispatch | NeedsReview | NEW conservative recovery completion |

All other transitions fail `IllegalTransition` without mutation. In particular an early approval, late diagnostic response, replayed execution result or conflicting verification batch cannot overwrite a newer result. Terminal states do not silently reopen. NeedsReview is intentionally a stop for operator assessment, not another automatic scheduler.

## 10. Ownership and dependencies

- Samy owns Booking, BookingRequest/Snapshot semantics, BookingService, the deliberately vulnerable fixture and guard rule. Usama's SQLite adapter must implement Samy's reviewed atomic contract, not invent a competing domain rule. Ameen reviews it.
- Ameen owns Evidence, Monitor, independent overlap scanning, timing and challenge-effect classification. Shehab reviews it.
- Shehab owns DiagnosticAgent, bounded observation/data contracts, PlanPolicy, OperatorCli and exact view semantics. Yazan reviews it.
- Yazan owns both RepairCommands, RepairExecutor, RecoveryVerifier, required-check aggregation test cases and failure testing. Usama reviews it; Ameen independently reviews invariant/preservation evidence.
- Usama owns Incident/Approval audit contracts, core/state machine, repositories and SQLite adapters, fixed-target adapter, native process ownership adapters, integration and later Oracle/CI feasibility. Samy reviews documented behavior; unfamiliar specialist configuration is flagged, not falsely claimed as expert-reviewed. Yazan checks the core/repair handoff.

Usama's core workload remains additional to specialist/leadership work. This proposal adds no team member, language split, production target, unrestricted SQL/shell or distributed infrastructure. Oracle Cloud and GitHub Actions remain required pre-demonstration work, not a tested deployment claim.

## 11. Verification and remaining gates

See verification.txt for checks actually run. Declaration compilation does not link or execute any method. Native Windows/MSVC and macOS/Apple Clang, SQLite integration, process safety, concurrency, bounded termination, crash recovery, preservation, approval tests and deployment have NOT been run.

Before coding, RS-06/RS-07 must review this exact revision and its full member/method delta, SQL schema/migrations, process lifetime rules, native API cleanup/error cases, fixed executable paths/arguments, fixture and limits. New helpers or members require UML/card revision first.

## 12. Deployment boundary

First implement and test everything natively and locally on Windows and macOS. The booking service/web app and controller remain separate processes. SQLite booking and incident files remain separate and local to their owning host. Later move only the backend/database and target web application to Oracle; main Rivive Shield controller remains on team computers, with presentation from Usama's device.

The local process adapter is not a remote Oracle adapter. Do not expose remote arbitrary PID, shell, executable, SQL or filesystem operations. Remote management/authentication, independent stop observation, restart authority when booking is down, receipts, incident-store placement, concurrency between controllers and Oracle OS remain open review decisions. A booking HTTP API alone cannot restart its own stopped process. Timeout is not proof of stop; read-only reconciliation precedes any reconsidered action after uncertainty. No network-shared SQLite file is proposed.

## 13. Native ownership proposal details

OwnedDemoProcess has no general target or PID setter. The composition root injects exactly one OS-specific instance; it outlives FixedDemoTargetAdapter and is not called elsewhere. All calls, including const observations, are serialized by the adapter lock. No hidden helper classes/state or new methods are authorized.

Windows processHandle_ is the retained native HANDLE represented as void* at this portable declaration boundary, not a custom opaque state container. Implementation must close any transient thread handle after launch, retain the process handle, check errors and treat interrupted launches as uncertain. macOS ownedChildPid_ is used only while exclusive unreaped ownership remains provable; childReaped_ and ownershipUncertain_ prohibit later signals. Exact native start/stop sequences and deadline behavior need review and tests before implementation. Destructors release resources and reap already-exited owned children where safe, but never silently kill a running target as cleanup.

The proposed platform classes are not proof of safe implementations. If target code can spawn descendants, process-group/job ownership requires a new reviewed UML change; this proposal authorizes only a single fixed owned process, not arbitrary process-tree termination.

ProcessStatus is deliberately separate from HealthStatus: Running proves only OS liveness, not service health. The adapter combines it with application observations; timeout alone is Unknown/Unhealthy, never Stopped. configure() is not present: configuration() returns the immutable native owner configuration for validation. FixedDemoTargetAdapter construction must compare every DemoConfiguration field with process.configuration() and reject a mismatch with SetupError before side effects. One process owner cannot be shared by multiple adapters. No runtime reconfiguration or automatic adoption is permitted.

Persist enums through reviewed stable names/codes, not C++ ordinal positions. Removing AI error alternatives changes ordinals; any future stored format must carry a reviewed schema version and reject or migrate incompatible records. No existing runtime data migration is claimed.
