# RS-25: Positive & Negative Rule Verification Matrix (P3-R2 Contracts)

- **Target Component:** `DiagnosticAgent::propose` (`RS-12`) consuming `RS-25_Rule_Catalogue.md`
- **Contract Header:** `docs/design-proposals/Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp`

---

## 1. Positive Test Cases (Expected Rule Match & Bounded `RepairPlan`)

| Test ID | Scenario | Input `Incident` & `Evidence` | Expected `DiagnosisResult` | Expected `IncidentStatus` & `RuleAudit` |
| :--- | :--- | :--- | :--- | :--- |
| **TC-RS25-P01** | Owned target unexpected exit (`RS25-R1`) | `kind = IncidentKind::ProcessExited`; `EvidenceKind::Process` (`ProcessState::Exited`, valid `ProcessIdentity` matching `TargetConfiguration`). | `proposedPlan.has_value() == true`; `action == ActionId::StartStoppedOwnedTarget`; `expectedProcess` matches target. | Next state: `IncidentStatus::AwaitingApproval`; `audit.matchedRuleId == "RS25-R1"`; `audit.evidenceRefs` non-empty. |
| **TC-RS25-P02** | Confirmed hung service on owned loopback port (`RS25-R2`) | `kind = IncidentKind::HttpErrorResponse`; `EvidenceKind::Http` (`HttpOutcome::Completed`, 503 on `loopbackPort`) + `EvidenceKind::Process` (`ProcessState::Running` with proven ownership). | `proposedPlan.has_value() == true`; `action == ActionId::ForceStopOwnedTarget`. | Next state: `IncidentStatus::AwaitingApproval`; `audit.matchedRuleId == "RS25-R2"`. |
| **TC-RS25-P03** | Force-stop followed by passed SQLite verification & new start approval | `ForceStopOwnedTarget` approved & confirmed stopped; `DatabaseCheck` (`integrity == Passed`, `foreignKeys == Passed`, `writersConfirmedStopped == true`); new `Incident` evaluation for stopped target. | Separate `StartStoppedOwnedTarget` plan proposed requiring a **new** `Approval`. | Transitions through `Verifying` $\rightarrow$ `AwaitingApproval` (for start) $\rightarrow$ `Recovered` only after start verification. |

---

## 2. Negative & Safety Boundary Test Cases (Expected Safe No-Match)

| Test ID | Scenario | Input `Incident` & `Evidence` | Expected `DiagnosisResult` | Expected `IncidentStatus` & `RuleAudit` |
| :--- | :--- | :--- | :--- | :--- |
| **TC-RS25-N01** | Healthy target state | `ProcessState::Running`, HTTP 200 OK, `DatabaseCheck` `Passed`. | `proposedPlan == std::nullopt` (no repair authorized). | No active recovery; `audit.matchedRuleId == std::nullopt`. |
| **TC-RS25-N02** | HTTP symptom with unknown process ownership | `kind = IncidentKind::HttpErrorResponse`; `EvidenceKind::Http` present, but `Attribution::Unknown` / missing `ProcessObservation`. | `proposedPlan == std::nullopt` (HTTP-only symptom cannot authorize force-stop). | `IncidentStatus::NeedsReview`; `audit.matchedRuleId == std::nullopt`. |
| **TC-RS25-N03** | Observation size/time limit exceeded | `kind = IncidentKind::ObservationLimitExceeded` or `HttpOutcome::ObservationLimitExceeded`. | `proposedPlan == std::nullopt` (limit exceeded is inconclusive, never termination authority). | `IncidentStatus::NeedsReview`; `audit.matchedRuleId == std::nullopt`. |
| **TC-RS25-N04** | Incomplete or missing evidence | `HttpOutcome::Incomplete` or empty `incident.evidence`. | `proposedPlan == std::nullopt`. | `IncidentStatus::NeedsReview`; `audit.matchedRuleId == std::nullopt`. |
| **TC-RS25-N05** | Contradictory evidence | `EvidenceKind::Process` (`ProcessState::Exited`) + `EvidenceKind::Http` (`HttpOutcome::Completed`, 200 OK) for same target. | `proposedPlan == std::nullopt`. | `IncidentStatus::NeedsReview`; `audit.matchedRuleId == std::nullopt`. |
| **TC-RS25-N06** | Database check failure (`RS25-R3`) | `kind = IncidentKind::DatabaseCheckFailed`; `DatabaseCheck` (`integrity == Failed`). | `proposedPlan == std::nullopt` (no auto-overwrite, no booking restore/cancel). | `IncidentStatus::NeedsReview`; `audit.matchedRuleId == "RS25-R3"`. |
| **TC-RS25-N07** | Failed/inconclusive SQLite check after force-stop | Target stopped via `ForceStopOwnedTarget`, `DatabaseCheck` returns `CheckOutcome::Failed` or `Inconclusive`. | `proposedPlan == std::nullopt` (automatic start strictly blocked). | Target stays stopped; `IncidentStatus::NeedsReview` / `Unresolved`. |
| **TC-RS25-N08** | Unsupported fault kind | `kind = IncidentKind::Unknown`. | `proposedPlan == std::nullopt`. | `IncidentStatus::Unresolved` / `NeedsReview`; `audit.matchedRuleId == std::nullopt`. |
