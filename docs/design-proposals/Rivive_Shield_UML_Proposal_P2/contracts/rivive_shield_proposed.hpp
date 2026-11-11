#pragma once
// PROPOSED UML CONTRACT REVISION P2. REVIEW ONLY; NOT IMPLEMENTATION OR APPROVAL.
// Derived from P1; user decisions supersede runtime AI and POSIX-only assumptions.
// Every type/signature/private field below is proposed unless noted as a retained conceptual name.
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <istream>
#include <mutex>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
#include <vector>
struct sqlite3; // External SQLite C-library type, not a new project helper.
namespace rivive {
using Version = std::uint64_t;
using UtcTime = std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
using Deadline = std::chrono::steady_clock::time_point;
using Now = std::function<UtcTime()>;
using BookingId = std::string;
using RoomId = std::string;
using EvidenceId = std::string;
using IncidentId = std::string;
using PlanId = std::string;
using ApprovalId = std::string;
using AttemptId = std::string;
using EventId = std::string;
using SessionId = std::string;
enum class ErrorCode { InvalidArgument, NotFound, StaleVersion, IllegalTransition, DuplicateEvent, Conflict, OverlapRejected, Busy, WrongTarget, UnsupportedAction, UnsupportedEvidence, ApprovalRequired, ApprovalMismatch, Expired, PreconditionFailed, LimitExceeded, TargetUnavailable, StorageUnavailable, OutcomeUncertain, CorruptRecord, InternalError };
enum class TargetId { DemoBookingService };
enum class ProbeCleanupPolicy { CancelCreatedProbesRetainRows };
enum class DataPreservationPolicy { ExactBaselineExceptApprovedDeltas };
enum class ActionId { RestartDemoService, RestoreBookingInvariant };
enum class BookingStatus { Active, CancelledByParticipant, CancelledConflict, CancelledVerificationProbe };
enum class ProcessStatus { Running, Stopped, Unknown };
enum class HealthStatus { Healthy, Stopped, Unhealthy, Unknown };
enum class EvidenceSource { Monitor, DiagnosticTool, Verifier, Reconciliation };
enum class IncidentStatus { Detected, Diagnosing, AwaitingApproval, Repairing, Verifying, Recovered, Unresolved, Declined, Failed, NeedsReview };
enum class CauseState { Unresolved, ServiceStopped, BookingOverlap };
enum class ApprovalDecision { Approve, Reject, Cancel };
enum class AttemptStatus { Prepared, Dispatching, Succeeded, Failed, Uncertain };
enum class ExecutionOutcome { Succeeded, KnownFailure, Uncertain };
enum class CheckId { ServiceHealth, NormalBooking, DataPreservation, BookingInvariant, GuardActive, OriginalConflictRejected, VariantConflictRejected, ProbeCleanup };
enum class CheckOutcome { Passed, Failed, Inconclusive, NotRun };
enum class TimeoutStage { Diagnosis, Approval, Execution, Verification };
enum class EventDisposition { Applied, Duplicate, Stale, Rejected, PersistenceBlocked };
enum class ChallengeOutcome { VerifiedEffect, GuardRejected, ObjectiveNotMet, Unresolved };

struct RepairPlan;

// NEW PROPOSED TYPE. Owner: All owners; Usama coordinates.
// Messages are bounded and sanitized; no credentials, shell output or connection metadata.
struct Error {
ErrorCode code;
    std::string message;
    std::vector<EvidenceId> evidenceRefs;
};
template<class T> using Result = std::variant<T, Error>;
using Status = Result<std::monostate>;

// NEW PROPOSED TYPE. Owner: Ameen Abd Allader.
// Typed evidence of a failed observation; never itself proves the target healthy or a guard effective.
struct FailureObservation {
ErrorCode code;
    std::string message;
};

// NEW PROPOSED TYPE. Owner: Samy mansour.
// Half-open [start,end), start < end; UTC millisecond precision.
struct TimeRange {
UtcTime start;
    UtcTime end;
};

// NEW PROPOSED TYPE. Owner: Samy mansour.
// Created timestamp is assigned by service, not caller; never delete a conflicting booking.
struct Booking {
BookingId id;
    RoomId roomId;
    TimeRange interval;
    std::string syntheticLabel;
    UtcTime createdAt;
    BookingStatus status;
    Version version;
    std::optional<std::string> cancellationReason;
};

// NEW PROPOSED TYPE. Owner: Samy mansour.
// Client-generated unique id is the idempotency key; identical retry returns same booking; different payload with same id is Conflict.
struct BookingRequest {
BookingId id;
    RoomId roomId;
    TimeRange interval;
    std::string syntheticLabel;
};

// NEW PROPOSED TYPE. Owner: Samy mansour.
// Consistent committed snapshot. Revision increases on every booking/guard mutation; IDs sorted bytewise.
struct BookingSnapshot {
Version revision;
    bool guardEnabled;
    std::vector<Booking> bookings;
};

// NEW PROPOSED TYPE. Owner: Ameen Abd Allader.
// Order by (createdAt, bytewise booking id). Only active same-room intersecting rows qualify.
struct OverlapPair {
BookingId earlierId;
    BookingId laterId;
};

// NEW PROPOSED TYPE. Owner: Ameen Abd Allader.
// Computed independently by Monitor, not read from a guard-success flag; sorted unique complete conflict set.
struct InvariantReport {
Version bookingRevision;
    bool holds;
    std::vector<OverlapPair> conflicts;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Fresh controller session invalidates old process plans; generation advances on each managed start/stop. Never a caller-supplied PID.
struct ProcessGeneration {
SessionId controllerSessionId;
    Version value;
};

// NEW PROPOSED TYPE. Owner: Ameen Abd Allader.
// Stopped requires positive fixed-process observation; a request timeout alone is Unknown or Unhealthy.
struct HealthObservation {
HealthStatus status;
    ProcessGeneration generation;
    UtcTime observedAt;
    std::string summary;
};
using Observation = std::variant<HealthObservation, BookingSnapshot, InvariantReport, FailureObservation>;
using ExpectedStateVersion = std::variant<ProcessGeneration, Version>;

// NEW PROPOSED TYPE. Owner: Samy mansour.
// One exact conflict pair in whole snapshot, preserve earlier, cancel later, enable guard in one SQLite transaction.
struct RestoreInvariantArgs {
BookingId earlierBookingId;
    BookingId laterBookingId;
    Version expectedEarlierVersion;
    Version expectedLaterVersion;
    Version expectedBookingRevision;
};

// NEW PROPOSED TYPE. Owner: Yazan Ganem.
// Only the configured stopped owned demo child may be started; no executable/argument/target input.
struct RestartArgs {
ProcessGeneration expectedGeneration;
};
using ActionArguments = std::variant<RestartArgs, RestoreInvariantArgs>;

// NEW PROPOSED TYPE. Owner: Samy mansour.
// Saved atomically with logical repair in booking store; query by attemptId during reconciliation.
struct BookingRepairReceipt {
AttemptId attemptId;
    RestoreInvariantArgs arguments;
    Version beforeRevision;
    Version afterRevision;
    Booking preservedBooking;
    Booking cancelledBooking;
    bool guardEnabled;
    UtcTime committedAt;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Ameen Abd Allader.
// Existing names id/timestamp/observation/reference retained; typed signature refined. Immutable once accepted.
struct Evidence {
EvidenceId id;
    UtcTime timestamp;
    Observation observation;
    EvidenceSource source;
    TargetId targetId;
    std::string summary;
    EvidenceId reference() const;
};

// NEW PROPOSED TYPE. Owner: Yazan Ganem.
// Displayed as part of immutable plan before approval. Logical repair requires both conflict probes; original retries later original interval, variant uses different overlapping bounds.
struct VerificationRecipe {
BookingRequest normalProbe;
    std::optional<BookingRequest> originalConflictProbe;
    std::optional<BookingRequest> variantConflictProbe;
    ProbeCleanupPolicy cleanupPolicy;
    DataPreservationPolicy preservationPolicy;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Shehab Shehab.
// Plan id/revision identifies immutable full content; changed content requires new revision and approval.
struct RepairPlan {
PlanId id;
    IncidentId incidentId;
    ActionId actionId;
    TargetId targetId;
    ExpectedStateVersion expectedStateVersion;
    Version revision;
    ActionArguments arguments;
    std::vector<EvidenceId> evidenceIds;
    EvidenceId baselineSnapshotRef;
    VerificationRecipe verification;
    std::string explanation;
    UtcTime createdAt;
    UtcTime expiresAt;
    std::vector<EvidenceId> evidenceReferences() const;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Usama Shmasneh.
// Only the operator boundary supplies decisions; matchesPlan checks id/incident/revision, not current state or expiry.
struct Approval {
PlanId planId;
    ApprovalDecision decision;
    UtcTime decisionTime;
    ApprovalId id;
    IncidentId incidentId;
    Version planRevision;
    Version expectedIncidentRevision;
    std::string operatorLabel;
    bool matchesPlan(const RepairPlan& plan) const;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Usama Shmasneh.
// attachEvidence changes only an in-memory draft, not lifecycle or durable revision. Duplicate identical id is a no-op; different content is Conflict.
struct Incident {
IncidentId id;
    IncidentStatus status;
    CauseState causeState;
    Version revision;
    Version currentPlanRevision;
    UtcTime detectedAt;
    std::optional<UtcTime> finishedAt;
    std::string summary;
    std::vector<Evidence> evidence;
    std::vector<RepairPlan> plans;
    std::vector<Approval> approvals;
    Status attachEvidence(const Evidence& value);
};

// NEW PROPOSED TYPE. Owner: Shehab Shehab.
// Positive bounded values must be selected and reviewed before implementation. No numeric defaults approved.
struct DiagnosticLimits {
std::size_t maxObservationReads;
    std::size_t maxEvidenceItems;
    std::size_t maxTextBytes;
    std::chrono::milliseconds overallTimeout;
};


// NEW PROPOSED TYPE. Owner: Shehab Shehab.
// Unresolved has no repair. Unsupported evidence/action/arguments never become executable.
struct DiagnosisDraft {
CauseState causeState;
    std::string explanation;
    std::vector<EvidenceId> evidenceRefs;
    std::optional<ActionId> proposedAction;
    std::optional<ActionArguments> proposedArguments;
};



// NEW PROPOSED TYPE. Owner: Shehab Shehab.
// Assigned by core; diagnosis cannot assign lifecycle revisions or identifiers.
struct DiagnosisContext {
PlanId proposedPlanId;
    Version proposedPlanRevision;
    UtcTime now;
    UtcTime planExpiresAt;
    Deadline deadline;
};

// NEW PROPOSED TYPE. Owner: Shehab Shehab.
// No plan on unresolved; a supported plan must pass deterministic PlanPolicy before presentation.
// P2 proposed audit of compiled-in rules; not an AI invocation or runtime rule loader.
struct RuleAudit {
    std::string rulesetVersion;
    std::optional<std::string> matchedRuleId;
    std::vector<EvidenceId> evidenceRefs;
    UtcTime evaluatedAt;
    std::string outcomeSummary;
};

struct DiagnosisResult {
CauseState causeState;
    std::string explanation;
    std::optional<RepairPlan> plan;
    std::vector<Evidence> collectedEvidence;
    RuleAudit ruleAudit;
    std::optional<Error> failure;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Prepared and incident Repairing are saved atomically before dispatch. Pending after restart is never automatically replayed.
struct RepairAttempt {
AttemptId id;
    IncidentId incidentId;
    PlanId planId;
    Version planRevision;
    ApprovalId approvalId;
    AttemptStatus status;
    UtcTime preparedAt;
    UtcTime expiresAt;
    std::optional<UtcTime> dispatchClaimedAt;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Returned only after committed Prepared -> Dispatching compare-and-swap; not a security token for untrusted code.
struct DispatchReceipt {
AttemptId attemptId;
    IncidentId incidentId;
    Version recordVersion;
    UtcTime claimedAt;
};

// NEW PROPOSED TYPE. Owner: Yazan Ganem.
// Once invocation may have begun, uncertainty is a returned outcome, never an ordinary retryable error.
struct ExecutionResult {
AttemptId attemptId;
    ExecutionOutcome outcome;
    std::optional<Error> failure;
    std::vector<Evidence> evidence;
    std::optional<BookingRepairReceipt> bookingReceipt;
    UtcTime finishedAt;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Yazan Ganem.
// passed iff outcome==Passed; false for Failed, Inconclusive, or NotRun. evidenceRef resolves to independent evidence.
struct VerificationResult {
CheckId checkId;
    bool passed;
    EvidenceId evidenceRef;
    CheckOutcome outcome;
    std::string detail;
    UtcTime checkedAt;
};

// NEW PROPOSED TYPE. Owner: Yazan Ganem.
// Probe ids/room/time selected before approval from isolated synthetic fixture. Original and variant probes are required for logical repair.
struct VerificationContext {
IncidentId incidentId;
    AttemptId attemptId;
    RepairPlan approvedPlan;
    BookingSnapshot baseline;
    Deadline deadline;
};

// NEW PROPOSED TYPE. Owner: Yazan Ganem.
// Results are observations, not a lifecycle transition. Probe cleanup failure fails verification.
struct VerificationBatch {
IncidentId incidentId;
    AttemptId attemptId;
    std::vector<VerificationResult> results;
    std::vector<Evidence> evidence;
    std::vector<BookingId> createdProbeIds;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// One consistent aggregate. Separate recordVersion CAS and incident revision; no separate successful UI state.
struct IncidentRecord {
Version recordVersion;
    Incident incident;
    std::vector<RepairAttempt> attempts;
    std::vector<ExecutionResult> executions;
    std::vector<VerificationResult> verificationResults;
    std::vector<EventId> acceptedEventIds;
    std::vector<RuleAudit> ruleEvaluations;
    std::vector<Error> failures;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Returned only after SQLite commit succeeds. Uncertain commit is an error requiring reload, never success.
struct SaveReceipt {
IncidentId incidentId;
    Version recordVersion;
    UtcTime committedAt;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Late timeout is rejected by event revisions; a possibly invoked action becomes NeedsReview.
struct TimeoutNotice {
TimeoutStage stage;
    std::optional<AttemptId> attemptId;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Internal core event from Detected only; not an implicit retry or operator authorization.
struct BeginDiagnosisNotice {
UtcTime requestedAt;
};

// NEW PROPOSED TYPE. Owner: Ameen Abd Allader.
// Derived only from independent observations, never a challenge-selector claim.
struct DetectionNotice {
std::vector<Evidence> evidence;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Read-only reconciliation supplies evidence; does not grant a dispatch or mark uncertain work Recovered.
struct ReconciliationNotice {
std::vector<Evidence> evidence;
    std::optional<BookingRepairReceipt> bookingReceipt;
    std::string reason;
};
using EventPayload = std::variant<DetectionNotice, BeginDiagnosisNotice, DiagnosisResult, Approval, ExecutionResult, VerificationBatch, TimeoutNotice, ReconciliationNotice>;

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// 0 plan revision only before any plan. Callbacks retain exact dispatched revisions and attempt identity.
struct IncidentEvent {
EventId id;
    IncidentId incidentId;
    Version expectedIncidentRevision;
    Version expectedPlanRevision;
    UtcTime observedAt;
    EventPayload payload;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Pure transition decision. Applied to authoritative in-memory view only after required persistence.
struct Transition {
IncidentStatus from;
    IncidentStatus to;
    Version fromRevision;
    Version toRevision;
    std::string reason;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Duplicate/Stale/Rejected cannot mutate target. Failed persistence reports NeedsReview in volatile view with durable=false.
struct EventOutcome {
EventDisposition disposition;
    IncidentStatus status;
    Version incidentRevision;
    Version recordVersion;
    std::optional<Error> failure;
};

// NEW PROPOSED TYPE. Owner: Shehab Shehab.
// Only view of core state; durable=false must be prominent, never show a saved recovery.
struct OperatorView {
IncidentRecord record;
    bool durable;
    bool repairsBlocked;
    std::optional<Error> visibleFailure;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Separate booking and incident files, owned local paths, foreign keys on, explicit transaction/durability settings reviewed before implementation.
struct SqliteConfiguration {
std::string databasePath;
    std::chrono::milliseconds busyTimeout;
};


// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// No shell interpolation. Configuration is operator-reviewed and immutable; only one isolated owned child; startup creates a fresh session.
struct DemoConfiguration {
std::string executablePath;
    std::vector<std::string> fixedArguments;
    std::string loopbackBaseUrl;
    SessionId controllerSessionId;
    std::chrono::milliseconds operationTimeout;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// All operations are typed, no arbitrary SQL. create/cancel/restore are atomic; restore validates the exact entire conflict set under the write transaction.
class BookingRepository {
public:
    virtual ~BookingRepository();
    virtual Result<BookingSnapshot> snapshot() const = 0;
    virtual Result<Booking> createBooking(const BookingRequest& request, UtcTime now) = 0;
    virtual Result<Booking> cancelBooking(const BookingId& id, Version expectedVersion, BookingStatus status, const std::string& reason, UtcTime now) = 0;
    virtual Result<BookingRepairReceipt> restoreInvariant(const RestoreInvariantArgs& arguments, const AttemptId& attemptId, UtcTime now) = 0;
    virtual Result<std::optional<BookingRepairReceipt>> loadRepairReceipt(const AttemptId& attemptId) const = 0;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Owns sqlite3 connection; destructor closes it. Construction can throw SetupError; no network storage.
class SqliteBookingRepository : public BookingRepository {
public:
    explicit SqliteBookingRepository(const SqliteConfiguration& configuration);
    ~SqliteBookingRepository() override;
    SqliteBookingRepository(const SqliteBookingRepository&) = delete;
    SqliteBookingRepository& operator=(const SqliteBookingRepository&) = delete;
    Result<BookingSnapshot> snapshot() const override;
    Result<Booking> createBooking(const BookingRequest& request, UtcTime now) override;
    Result<Booking> cancelBooking(const BookingId& id, Version expectedVersion, BookingStatus status, const std::string& reason, UtcTime now) override;
    Result<BookingRepairReceipt> restoreInvariant(const RestoreInvariantArgs& arguments, const AttemptId& attemptId, UtcTime now) override;
    Result<std::optional<BookingRepairReceipt>> loadRepairReceipt(const AttemptId& attemptId) const override;
private:
    SqliteConfiguration configuration_;
    sqlite3* database_;
    mutable std::mutex mutex_;
};

// NEW PROPOSED TYPE. Owner: Samy mansour.
// Shared domain semantics. Normal create is vulnerable only in the reviewed initial guard-disabled fixture; no public disable-guard API.
class BookingService {
public:
    BookingService(BookingRepository& repository, Now now);
    Result<Booking> createBooking(const BookingRequest& request);
    Result<Booking> cancelBooking(const BookingId& id, Version expectedVersion);
    Result<BookingSnapshot> listBookings() const;
    static Status validate(const BookingRequest& request);
    static bool overlaps(const TimeRange& first, const TimeRange& second);
private:
    BookingRepository& repository_;
    Now now_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Usama Shmasneh.
// Existing saveRecord/loadRecord refined to typed CAS operations. claimDispatch durably consumes Prepared once; stale or duplicate never dispatch.
class IncidentRepository {
public:
    virtual ~IncidentRepository();
    virtual Result<SaveReceipt> saveRecord(const IncidentRecord& record, Version expectedRecordVersion) = 0;
    virtual Result<IncidentRecord> loadRecord(const IncidentId& id) const = 0;
    virtual Result<std::vector<IncidentRecord>> loadPendingRecords() const = 0;
    virtual Result<DispatchReceipt> claimDispatch(const IncidentId& incidentId, const AttemptId& attemptId, Version expectedRecordVersion, UtcTime now) = 0;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Owns SQLite connection. One active nonterminal demo incident enforced transactionally, including after restart.
class SqliteIncidentRepository : public IncidentRepository {
public:
    explicit SqliteIncidentRepository(const SqliteConfiguration& configuration);
    ~SqliteIncidentRepository() override;
    SqliteIncidentRepository(const SqliteIncidentRepository&) = delete;
    SqliteIncidentRepository& operator=(const SqliteIncidentRepository&) = delete;
    Result<SaveReceipt> saveRecord(const IncidentRecord& record, Version expectedRecordVersion) override;
    Result<IncidentRecord> loadRecord(const IncidentId& id) const override;
    Result<std::vector<IncidentRecord>> loadPendingRecords() const override;
    Result<DispatchReceipt> claimDispatch(const IncidentId& incidentId, const AttemptId& attemptId, Version expectedRecordVersion, UtcTime now) override;
private:
    SqliteConfiguration configuration_;
    sqlite3* database_;
    mutable std::mutex mutex_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Usama Shmasneh.
// readHealthAndEvidence is read-only even while service stopped. Diagnosis receives const adapter only. Verification uses real service path, not direct repository insertion.
class DemoTargetAdapter {
public:
    virtual ~DemoTargetAdapter();
    virtual Result<std::vector<Evidence>> readHealthAndEvidence(Deadline deadline) const = 0;
    virtual ExecutionResult applyRegisteredAction(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) = 0;
    virtual Result<Booking> submitVerificationBooking(const BookingRequest& request, Deadline deadline) = 0;
    virtual Result<Booking> cancelVerificationBooking(const BookingId& id, Version expectedVersion, Deadline deadline) = 0;
    virtual Result<ReconciliationNotice> readExecutionEvidence(const AttemptId& attemptId, Deadline deadline) const = 0;
    virtual Status stopForControlledChallenge(Deadline deadline) = 0;
};

// P2 proposed OS boundary. Fixed configured executable only; no arbitrary PID or shell.
// All operations serialized by adapter; implementations must not mutate after losing ownership.
class OwnedDemoProcess {
public:
    virtual ~OwnedDemoProcess();
    virtual const DemoConfiguration& configuration() const = 0;
    virtual Status start(Deadline deadline) = 0;
    virtual Result<ProcessStatus> observe(Deadline deadline) const = 0;
    virtual Status stop(Deadline deadline) = 0;
};

// Windows-only implementation proposal. Native HANDLE stored as void* solely for ABI isolation;
// this is an OS handle, not opaque project state. No PID-based reopen or lookup is allowed.
class WindowsOwnedDemoProcess : public OwnedDemoProcess {
public:
    explicit WindowsOwnedDemoProcess(const DemoConfiguration& configuration);
    ~WindowsOwnedDemoProcess() override;
    WindowsOwnedDemoProcess(const WindowsOwnedDemoProcess&) = delete;
    WindowsOwnedDemoProcess& operator=(const WindowsOwnedDemoProcess&) = delete;
    const DemoConfiguration& configuration() const override;
    Status start(Deadline deadline) override;
    Result<ProcessStatus> observe(Deadline deadline) const override;
    Status stop(Deadline deadline) override;
private:
    const DemoConfiguration configuration_;
    void* processHandle_;
    bool ownershipUncertain_;
};

// macOS-only implementation proposal. Exclusive parent/reaper ownership is mandatory.
// Never signal after wait/reap; no SIGCHLD auto-reaping and no external wait on this child.
class MacOwnedDemoProcess : public OwnedDemoProcess {
public:
    explicit MacOwnedDemoProcess(const DemoConfiguration& configuration);
    ~MacOwnedDemoProcess() override;
    MacOwnedDemoProcess(const MacOwnedDemoProcess&) = delete;
    MacOwnedDemoProcess& operator=(const MacOwnedDemoProcess&) = delete;
    const DemoConfiguration& configuration() const override;
    Status start(Deadline deadline) override;
    Result<ProcessStatus> observe(Deadline deadline) const override;
    Status stop(Deadline deadline) override;
private:
    const DemoConfiguration configuration_;
    std::int64_t ownedChildPid_;
    bool childReaped_;
    bool ownershipUncertain_;
};

// Native local adapter proposal. startForSetup requires empty active-incident store; never a repair bypass.
class FixedDemoTargetAdapter : public DemoTargetAdapter {
public:
    FixedDemoTargetAdapter(const DemoConfiguration& configuration, OwnedDemoProcess& process, BookingRepository& bookingRepository, IncidentRepository& incidentRepository, Now now);
    ~FixedDemoTargetAdapter() override;
    FixedDemoTargetAdapter(const FixedDemoTargetAdapter&) = delete;
    FixedDemoTargetAdapter& operator=(const FixedDemoTargetAdapter&) = delete;
    Status startForSetup(Deadline deadline);
    Result<std::vector<Evidence>> readHealthAndEvidence(Deadline deadline) const override;
    ExecutionResult applyRegisteredAction(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) override;
    Result<Booking> submitVerificationBooking(const BookingRequest& request, Deadline deadline) override;
    Result<Booking> cancelVerificationBooking(const BookingId& id, Version expectedVersion, Deadline deadline) override;
    Result<ReconciliationNotice> readExecutionEvidence(const AttemptId& attemptId, Deadline deadline) const override;
    Status stopForControlledChallenge(Deadline deadline) override;
private:
    const DemoConfiguration configuration_;
    BookingRepository& bookingRepository_;
    IncidentRepository& incidentRepository_;
    Now now_;
    OwnedDemoProcess& process_;
    ProcessGeneration generation_;
    std::vector<AttemptId> consumedDispatches_;
    mutable std::mutex mutex_;
};

// NEW PROPOSED TYPE. Owner: Ameen Abd Allader.
// Independent overlap scan does not call BookingService::overlaps, storage repair logic, or diagnostic rules. No repair authority.
class Monitor {
public:
    Monitor(const DemoTargetAdapter& target, Now now);
    Result<DetectionNotice> poll(Deadline deadline) const;
    InvariantReport checkInvariant(const BookingSnapshot& snapshot) const;
    ChallengeOutcome classifyChallenge(const std::vector<Evidence>& before, const std::vector<Evidence>& after, const Result<Booking>& actualAttemptResult) const;
private:
    const DemoTargetAdapter& target_;
    Now now_;
};



// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Shehab Shehab.
// Deterministic allowlist plus action/argument/version-tag match, evidence support, expiry, approval binding, and current-plan checks. No diagnostic decision overrides rejection.
class PlanPolicy {
public:
    explicit PlanPolicy(TargetId target);
    Status validate(const RepairPlan& plan, const Incident& incident, UtcTime now) const;
    Status revalidate(const RepairPlan& plan, const Approval& approval, const Incident& incident, UtcTime now) const;
private:
    const TargetId target_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Shehab Shehab.
// Compiled-in evidence-based rules; no external provider, rule file or challenge selector. Unknown/conflicting evidence yields Unresolved and no plan.
class DiagnosticAgent {
public:
    DiagnosticAgent(const DemoTargetAdapter& target, const Monitor& monitor, const PlanPolicy& policy, DiagnosticLimits limits);
    Result<DiagnosisResult> propose(const Incident& incident, const DiagnosisContext& context);
private:
    const DemoTargetAdapter& target_;
    const Monitor& monitor_;
    const PlanPolicy& policy_;
    const DiagnosticLimits limits_;
    DiagnosisDraft evaluateRules(const std::vector<Evidence>& evidence) const;
    std::string rulesetVersion() const;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Yazan Ganem.
// Only executor invokes execute in reviewed composition; adapter rechecks committed dispatch and versions. Precheck success alone does not authorize mutation.
class RepairCommand {
public:
    virtual ~RepairCommand();
    virtual Status checkPreconditions(const RepairPlan& plan, Deadline deadline) const = 0;
    virtual ExecutionResult execute(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) = 0;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Yazan Ganem.
// Fixed-target stop observation and generation must still match immediately under control lock.
class RestartDemoServiceCommand : public RepairCommand {
public:
    explicit RestartDemoServiceCommand(DemoTargetAdapter& target);
    Status checkPreconditions(const RepairPlan& plan, Deadline deadline) const override;
    ExecutionResult execute(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) override;
private:
    DemoTargetAdapter& target_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Yazan Ganem.
// Early read-only check plus authoritative recheck inside atomic booking transaction; stale state never partly repairs.
class RestoreBookingInvariantCommand : public RepairCommand {
public:
    explicit RestoreBookingInvariantCommand(DemoTargetAdapter& target);
    Status checkPreconditions(const RepairPlan& plan, Deadline deadline) const override;
    ExecutionResult execute(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) override;
private:
    DemoTargetAdapter& target_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Yazan Ganem.
// Loads authoritative record; revalidates; checks preconditions; atomically claims durable prepared attempt; invokes exactly one registered command. No retry loop.
class RepairExecutor {
public:
    RepairExecutor(const PlanPolicy& policy, IncidentRepository& repository, RestartDemoServiceCommand& restart, RestoreBookingInvariantCommand& restore, Now now);
    Result<ExecutionResult> executeOnce(const RepairPlan& plan, const Approval& approval, const AttemptId& attemptId, Deadline deadline);
private:
    const PlanPolicy& policy_;
    IncidentRepository& repository_;
    RestartDemoServiceCommand& restart_;
    RestoreBookingInvariantCommand& restore_;
    Now now_;
    std::mutex dispatchMutex_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Yazan Ganem.
// Independent observations, real service booking, complete baseline comparison; logical action additionally runs original and variant guard probes and cleanup.
class RecoveryVerifier {
public:
    RecoveryVerifier(DemoTargetAdapter& target, const Monitor& monitor, Now now);
    Result<VerificationBatch> verifyHealth(const VerificationContext& context) const;
    Result<VerificationBatch> verifyNormalBooking(const VerificationContext& context);
    Result<VerificationBatch> verifyDataAndInvariant(const VerificationContext& context);
private:
    DemoTargetAdapter& target_;
    const Monitor& monitor_;
    Now now_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Usama Shmasneh.
// Pure decision against durable baseline; applyEvent does not mutate or dispatch. Core reconstructs this value after a committed transition. state/revision preserve conceptual fields.
class IncidentStateMachine {
public:
    explicit IncidentStateMachine(const Incident& durableIncident);
    bool canTransition(IncidentStatus destination) const;
    Result<Transition> applyEvent(const Incident& incident, const IncidentEvent& event) const;
private:
    IncidentStatus state_;
    Version revision_;
};

// RETAINED NAME; SIGNATURE/FIELD REFINEMENT PROPOSED. Owner: Usama Shmasneh.
// One owner thread performs state transitions and module calls; external callbacks only enqueue. Queue lock is never held across I/O. Explicit deadlines bound blocking calls; no extra lifecycle controller.
class IncidentOrchestrator {
public:
    IncidentOrchestrator(IncidentRepository& repository, DiagnosticAgent& diagnosticAgent, const PlanPolicy& policy, RepairExecutor& executor, RecoveryVerifier& verifier, const DemoTargetAdapter& target, Now now, std::size_t queueCapacity);
    Status enqueueEvent(IncidentEvent event);
    Result<EventOutcome> handleEvent(const IncidentEvent& event);
    Result<EventOutcome> submitApproval(const IncidentId& incidentId, const EventId& eventId, const Approval& approval);
    Result<std::vector<OperatorView>> reconcileAfterRestart(Deadline deadline);
    Result<OperatorView> currentView() const;
    Result<std::size_t> processPendingEvents(std::size_t limit);
private:
    IncidentRepository& repository_;
    DiagnosticAgent& diagnosticAgent_;
    const PlanPolicy& policy_;
    RepairExecutor& executor_;
    RecoveryVerifier& verifier_;
    const DemoTargetAdapter& target_;
    Now now_;
    std::optional<IncidentRecord> currentRecord_;
    std::optional<IncidentStateMachine> stateMachine_;
    std::deque<IncidentEvent> eventQueue_;
    const std::size_t queueCapacity_;
    std::optional<AttemptId> attemptId_;
    bool repairsBlocked_;
    std::optional<Error> volatileFailure_;
    mutable std::mutex queueMutex_;
    std::thread::id ownerThread_;
};

// NEW PROPOSED TYPE. Owner: Shehab Shehab.
// Presentation and input only. Exact plan and verification side effects displayed; core remains authoritative.
class OperatorCli {
public:
    OperatorCli(IncidentOrchestrator& core, std::istream& input, std::ostream& output);
    Status showCurrent() const;
    Result<EventOutcome> submitDecision(const IncidentId& incidentId, const EventId& eventId, const Approval& approval);
    Status runOneCommand();
private:
    IncidentOrchestrator& core_;
    std::istream& input_;
    std::ostream& output_;
};

// NEW PROPOSED TYPE. Owner: Usama Shmasneh.
// Only construction failures use this exception; ordinary operational failures return typed Result.
class SetupError : public std::runtime_error {
public:
    explicit SetupError(Error failure);
    const Error& failure() const noexcept;
private:
    Error failure_;
};

} // namespace rivive
