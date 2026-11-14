# Explicit P2 → P3 draft delta

Baseline consulted: `rivive_shield_proposed.hpp` from the existing P2 package, SHA256 `e1ed270f5c9c89accf46a2eacd089740540d74efb57d354358b6b72c4c5cc070`. Historical P2 adoption is retained; this file does NOT revoke it silently.

| P2 concept | P3 draft decision | Reason / status |
|---|---|---|
| BookingService/BookingRepository | Retain concepts; atomic create/list/rooms only | Independent ordinary booking backend; user has now approved deferring cancellation; exact API review remains required |
| BookingStatus and cancel/restore fields | Omit from first-milestone draft | No automatic reservation alteration or verification-probe cancellation |
| RestoreBookingInvariant / RestoreInvariantArgs / BookingRepairReceipt | Remove from proposed current action allowlist | New correction focus is process/recovery; do not mutate bookings from monitor |
| DemoTargetAdapter / FixedDemoTargetAdapter | Replace responsibilities with process, capture, parser, DB verification and orchestration boundaries | No hidden service health probe / no backdoor direct booking writes |
| Monitor.poll/checkInvariant | Replace with passive bounded capture + distinct owned-process observation | Local passive HTTP requirement; no certainty from silence |
| HealthStatus Healthy | Do not infer from live connection; explicit evidence outcomes | Complete HTTP 500 is not healthy; HTTP 200 body may be incomplete |
| ProcessGeneration and OS-owned-child boundary | Retain/refine as ProcessIdentity + retained OS authority | PID alone insufficient; stale sessions invalidate approvals |
| OperatorCli | Replace main presentation with DesktopView/WxDesktopView/Presenter | Desktop app request; CLI experimental tools stay outside product contract |
| DiagnosticAgent / RuleAudit / PlanPolicy | Retain deterministic concepts | Compiled rules, no external AI provider |
| Approval / attempts / durable claim / state machine | Retain/refine | Exact action binding, durable dispatch once, uncertainty preserved |
| Automatic verification booking/cancellation | Remove from current proposed verifier | No replay/payload retention; operator-driven synthetic activity can provide independent functional evidence |
| SQLite booking and incident stores | Retain separate stores | Read-only integrity check does not grant booking mutations |
| Open conflict repair | Replaced by normal prevention + separate business tests | No restored invariant claim based on SQLite structural checks |
| Oracle component plan | Retain later goal, do NOT invent a remote agent | Local OS ownership cannot act on remote Oracle process; separate design needed |

Core retained names do NOT imply binary/source compatibility. Namespace `rivive::p3_draft` prevents accidental mix with P2. Every changed signature needs review before replacement.
