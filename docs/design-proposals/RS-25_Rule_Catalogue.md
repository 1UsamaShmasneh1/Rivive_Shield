# RS-25: Deterministic Diagnosis & Recovery Rule Catalogue (P3-R2 Baseline)

- **Card:** RS-25 Prepare predefined diagnosis and recovery rules
- **Owner:** Shehab Shehab
- **Reviewer:** Yazan Ganem
- **Contract Baseline:** P3-R2 (`docs/design-proposals/Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp`)
- **Consumed By:** `DiagnosticAgent::propose` / `DiagnosticAgent::evaluateCompiledRules` and `RuleAudit` (`RS-12`)

---

## 1. Contract & Safety Boundaries
- **Compiled-in deterministic rules only:** No external AI provider, local LLM, runtime rule script, or invented SQL rule engine.
- **Allowed `RepairPlan` actions (`ActionId`):**
  1. `ActionId::StartStoppedOwnedTarget`
  2. `ActionId::ForceStopOwnedTarget`
- **Forbidden actions:** Booking restoration, booking cancellation, unowned process termination, or automatic start after a force-stop without independent SQLite verification and a **new** explicit operator `Approval`.
- **Approved 9 `IncidentStatus` states:**
  `Detected`, `Diagnosing`, `AwaitingApproval`, `Executing`, `Verifying`, `Recovered`, `Declined`, `Unresolved`, `NeedsReview`.
- **Single-incident & serialization invariant:** Maintain at most one active incident per managed target, aggregate observations with provenance, and never close an incident with unresolved side effects.

---

## 2. Deterministic Rule Definitions

| Rule ID | `IncidentKind` | Required Evidence (`EvidenceKind` & Provenance) | Strict Exclusions | Bounded `RepairPlan` Proposal |
| :--- | :--- | :--- | :--- | :--- |
| **RS25-R1** | `ProcessExited` | `EvidenceKind::Process` showing the owned target (`ProcessState::Exited`) with valid `ProcessIdentity` provenance matching `TargetConfiguration`. | Process is `Running` or `NotStarted`; operator-initiated stop; unowned PID; stale or missing observation; time/size limit exceeded. | `ActionId::StartStoppedOwnedTarget` (single bounded start; requires explicit operator `Approval` before execution). |
| **RS25-R2** | `HttpErrorResponse` | `EvidenceKind::Http` (`HttpOutcome::Completed`, repeated target HTTP 5xx / service failure on configured `loopbackPort`) **AND** `EvidenceKind::Process` confirming owned target is `ProcessState::Running` with valid `Attribution` / `FlowKey` provenance. | HTTP-only symptom without process ownership proof; single transient error; `HttpOutcome::Incomplete` or `ObservationLimitExceeded`; healthy HTTP 2xx response. | `ActionId::ForceStopOwnedTarget` (separate plan requiring explicit force-stop `Approval`, followed by `DatabaseCheck` verification and a **new** start `Approval`). |
| **RS25-R3** | `DatabaseCheckFailed` | `EvidenceKind::Database` where `DatabaseCheck` reports `CheckOutcome::Failed` on an owned `configuredDatabaseKey` with confirmed provenance. | Unowned database path; `CheckOutcome::Passed`; `CheckOutcome::Inconclusive` or `NotRun`; any booking data modification/cancellation. | No automatic destructive repair (`std::nullopt` plan); escalates to `IncidentStatus::NeedsReview` for operator inspection. |

---

## 3. Safe No-Match & Inconclusive Evidence Policy

| Input / Evidence Condition | `DiagnosisResult.proposedPlan` | Target `IncidentStatus` | `RuleAudit` Recording |
| :--- | :--- | :--- | :--- |
| **Healthy inputs** (`ProcessState::Running`, HTTP 200, `CheckOutcome::Passed`) | `std::nullopt` (None) | No repair authorized | `matchedRuleId = std::nullopt`, `rulesetVersion` recorded |
| **Missing / incomplete evidence** (`HttpOutcome::Incomplete`, `ProcessState::Unknown`, `Attribution::Unknown`) | `std::nullopt` (None) | `IncidentStatus::NeedsReview` | `matchedRuleId = std::nullopt`, `evidenceRefs` logged |
| **Time / size limit exceeded** (`IncidentKind::ObservationLimitExceeded`, `HttpOutcome::ObservationLimitExceeded`) | `std::nullopt` (None) — treated as inconclusive, never automatic termination authority | `IncidentStatus::NeedsReview` | `matchedRuleId = std::nullopt`, `evidenceRefs` logged |
| **Contradictory evidence** (e.g., `ProcessState::Exited` alongside `HttpOutcome::Completed` 200 OK on same target) | `std::nullopt` (None) | `IncidentStatus::NeedsReview` | `matchedRuleId = std::nullopt`, `evidenceRefs` logged |
| **Unsupported fault (`IncidentKind::Unknown`)** | `std::nullopt` (None) | `IncidentStatus::Unresolved` / `NeedsReview` | `matchedRuleId = std::nullopt`, `evidenceRefs` logged |

---

## 4. Force-Stop & Recovery State Sequence
1. `Detected` $\rightarrow$ `Diagnosing` (`DiagnosticAgent::propose` evaluates compiled rules and emits `RuleAudit`).
2. If a bounded plan is proposed (`StartStoppedOwnedTarget` or `ForceStopOwnedTarget`): `Diagnosing` $\rightarrow$ `AwaitingApproval`.
   - If operator rejects (`ApprovalDecision::Reject`): `AwaitingApproval` $\rightarrow$ `Declined`.
   - If operator approves (`ApprovalDecision::Approve`): `AwaitingApproval` $\rightarrow$ `Executing` $\rightarrow$ `Verifying`.
3. **Force-stop rule (`ActionId::ForceStopOwnedTarget`):**
   - Requires explicit force-stop approval $\rightarrow$ confirmed process stop $\rightarrow$ read-only SQLite verification (`DatabaseCheck`: `integrity == Passed`, `foreignKeys == Passed`, `writersConfirmedStopped == true`) $\rightarrow$ **new** separate `StartStoppedOwnedTarget` plan and **new** operator `Approval`.
   - If `DatabaseCheck` is `Failed`, `Inconclusive`, or `NotRun`: target stays stopped and transitions to `IncidentStatus::NeedsReview` / `Unresolved` (automatic start is strictly forbidden).
