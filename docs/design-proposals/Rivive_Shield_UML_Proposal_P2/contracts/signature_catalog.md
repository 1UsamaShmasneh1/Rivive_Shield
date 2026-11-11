# P2 exact declaration catalogue
Review only. Source of truth: matching header. Every declared field and method, including private helpers and deleted special members, is included. Enums/aliases are included at end.

## Error

```cpp
struct Error {
ErrorCode code;
    std::string message;
    std::vector<EvidenceId> evidenceRefs;
};
```

## FailureObservation

```cpp
struct FailureObservation {
ErrorCode code;
    std::string message;
};
```

## TimeRange

```cpp
struct TimeRange {
UtcTime start;
    UtcTime end;
};
```

## Booking

```cpp
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
```

## BookingRequest

```cpp
struct BookingRequest {
BookingId id;
    RoomId roomId;
    TimeRange interval;
    std::string syntheticLabel;
};
```

## BookingSnapshot

```cpp
struct BookingSnapshot {
Version revision;
    bool guardEnabled;
    std::vector<Booking> bookings;
};
```

## OverlapPair

```cpp
struct OverlapPair {
BookingId earlierId;
    BookingId laterId;
};
```

## InvariantReport

```cpp
struct InvariantReport {
Version bookingRevision;
    bool holds;
    std::vector<OverlapPair> conflicts;
};
```

## ProcessGeneration

```cpp
struct ProcessGeneration {
SessionId controllerSessionId;
    Version value;
};
```

## HealthObservation

```cpp
struct HealthObservation {
HealthStatus status;
    ProcessGeneration generation;
    UtcTime observedAt;
    std::string summary;
};
```

## RestoreInvariantArgs

```cpp
struct RestoreInvariantArgs {
BookingId earlierBookingId;
    BookingId laterBookingId;
    Version expectedEarlierVersion;
    Version expectedLaterVersion;
    Version expectedBookingRevision;
};
```

## RestartArgs

```cpp
struct RestartArgs {
ProcessGeneration expectedGeneration;
};
```

## BookingRepairReceipt

```cpp
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
```

## Evidence

```cpp
struct Evidence {
EvidenceId id;
    UtcTime timestamp;
    Observation observation;
    EvidenceSource source;
    TargetId targetId;
    std::string summary;
    EvidenceId reference() const;
};
```

## VerificationRecipe

```cpp
struct VerificationRecipe {
BookingRequest normalProbe;
    std::optional<BookingRequest> originalConflictProbe;
    std::optional<BookingRequest> variantConflictProbe;
    ProbeCleanupPolicy cleanupPolicy;
    DataPreservationPolicy preservationPolicy;
};
```

## RepairPlan

```cpp
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
```

## Approval

```cpp
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
```

## Incident

```cpp
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
```

## DiagnosticLimits

```cpp
struct DiagnosticLimits {
std::size_t maxObservationReads;
    std::size_t maxEvidenceItems;
    std::size_t maxTextBytes;
    std::chrono::milliseconds overallTimeout;
};
```

## DiagnosisDraft

```cpp
struct DiagnosisDraft {
CauseState causeState;
    std::string explanation;
    std::vector<EvidenceId> evidenceRefs;
    std::optional<ActionId> proposedAction;
    std::optional<ActionArguments> proposedArguments;
};
```

## DiagnosisContext

```cpp
struct DiagnosisContext {
PlanId proposedPlanId;
    Version proposedPlanRevision;
    UtcTime now;
    UtcTime planExpiresAt;
    Deadline deadline;
};
```

## RuleAudit

```cpp
struct RuleAudit {
    std::string rulesetVersion;
    std::optional<std::string> matchedRuleId;
    std::vector<EvidenceId> evidenceRefs;
    UtcTime evaluatedAt;
    std::string outcomeSummary;
};
```

## DiagnosisResult

```cpp
struct DiagnosisResult {
CauseState causeState;
    std::string explanation;
    std::optional<RepairPlan> plan;
    std::vector<Evidence> collectedEvidence;
    RuleAudit ruleAudit;
    std::optional<Error> failure;
};
```

## RepairAttempt

```cpp
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
```

## DispatchReceipt

```cpp
struct DispatchReceipt {
AttemptId attemptId;
    IncidentId incidentId;
    Version recordVersion;
    UtcTime claimedAt;
};
```

## ExecutionResult

```cpp
struct ExecutionResult {
AttemptId attemptId;
    ExecutionOutcome outcome;
    std::optional<Error> failure;
    std::vector<Evidence> evidence;
    std::optional<BookingRepairReceipt> bookingReceipt;
    UtcTime finishedAt;
};
```

## VerificationResult

```cpp
struct VerificationResult {
CheckId checkId;
    bool passed;
    EvidenceId evidenceRef;
    CheckOutcome outcome;
    std::string detail;
    UtcTime checkedAt;
};
```

## VerificationContext

```cpp
struct VerificationContext {
IncidentId incidentId;
    AttemptId attemptId;
    RepairPlan approvedPlan;
    BookingSnapshot baseline;
    Deadline deadline;
};
```

## VerificationBatch

```cpp
struct VerificationBatch {
IncidentId incidentId;
    AttemptId attemptId;
    std::vector<VerificationResult> results;
    std::vector<Evidence> evidence;
    std::vector<BookingId> createdProbeIds;
};
```

## IncidentRecord

```cpp
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
```

## SaveReceipt

```cpp
struct SaveReceipt {
IncidentId incidentId;
    Version recordVersion;
    UtcTime committedAt;
};
```

## TimeoutNotice

```cpp
struct TimeoutNotice {
TimeoutStage stage;
    std::optional<AttemptId> attemptId;
};
```

## BeginDiagnosisNotice

```cpp
struct BeginDiagnosisNotice {
UtcTime requestedAt;
};
```

## DetectionNotice

```cpp
struct DetectionNotice {
std::vector<Evidence> evidence;
};
```

## ReconciliationNotice

```cpp
struct ReconciliationNotice {
std::vector<Evidence> evidence;
    std::optional<BookingRepairReceipt> bookingReceipt;
    std::string reason;
};
```

## IncidentEvent

```cpp
struct IncidentEvent {
EventId id;
    IncidentId incidentId;
    Version expectedIncidentRevision;
    Version expectedPlanRevision;
    UtcTime observedAt;
    EventPayload payload;
};
```

## Transition

```cpp
struct Transition {
IncidentStatus from;
    IncidentStatus to;
    Version fromRevision;
    Version toRevision;
    std::string reason;
};
```

## EventOutcome

```cpp
struct EventOutcome {
EventDisposition disposition;
    IncidentStatus status;
    Version incidentRevision;
    Version recordVersion;
    std::optional<Error> failure;
};
```

## OperatorView

```cpp
struct OperatorView {
IncidentRecord record;
    bool durable;
    bool repairsBlocked;
    std::optional<Error> visibleFailure;
};
```

## SqliteConfiguration

```cpp
struct SqliteConfiguration {
std::string databasePath;
    std::chrono::milliseconds busyTimeout;
};
```

## DemoConfiguration

```cpp
struct DemoConfiguration {
std::string executablePath;
    std::vector<std::string> fixedArguments;
    std::string loopbackBaseUrl;
    SessionId controllerSessionId;
    std::chrono::milliseconds operationTimeout;
};
```

## BookingRepository

```cpp
class BookingRepository {
public:
    virtual ~BookingRepository();
    virtual Result<BookingSnapshot> snapshot() const = 0;
    virtual Result<Booking> createBooking(const BookingRequest& request, UtcTime now) = 0;
    virtual Result<Booking> cancelBooking(const BookingId& id, Version expectedVersion, BookingStatus status, const std::string& reason, UtcTime now) = 0;
    virtual Result<BookingRepairReceipt> restoreInvariant(const RestoreInvariantArgs& arguments, const AttemptId& attemptId, UtcTime now) = 0;
    virtual Result<std::optional<BookingRepairReceipt>> loadRepairReceipt(const AttemptId& attemptId) const = 0;
};
```

## SqliteBookingRepository

```cpp
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
```

## BookingService

```cpp
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
```

## IncidentRepository

```cpp
class IncidentRepository {
public:
    virtual ~IncidentRepository();
    virtual Result<SaveReceipt> saveRecord(const IncidentRecord& record, Version expectedRecordVersion) = 0;
    virtual Result<IncidentRecord> loadRecord(const IncidentId& id) const = 0;
    virtual Result<std::vector<IncidentRecord>> loadPendingRecords() const = 0;
    virtual Result<DispatchReceipt> claimDispatch(const IncidentId& incidentId, const AttemptId& attemptId, Version expectedRecordVersion, UtcTime now) = 0;
};
```

## SqliteIncidentRepository

```cpp
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
```

## DemoTargetAdapter

```cpp
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
```

## OwnedDemoProcess

```cpp
class OwnedDemoProcess {
public:
    virtual ~OwnedDemoProcess();
    virtual const DemoConfiguration& configuration() const = 0;
    virtual Status start(Deadline deadline) = 0;
    virtual Result<ProcessStatus> observe(Deadline deadline) const = 0;
    virtual Status stop(Deadline deadline) = 0;
};
```

## WindowsOwnedDemoProcess

```cpp
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
```

## MacOwnedDemoProcess

```cpp
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
```

## FixedDemoTargetAdapter

```cpp
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
```

## Monitor

```cpp
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
```

## PlanPolicy

```cpp
class PlanPolicy {
public:
    explicit PlanPolicy(TargetId target);
    Status validate(const RepairPlan& plan, const Incident& incident, UtcTime now) const;
    Status revalidate(const RepairPlan& plan, const Approval& approval, const Incident& incident, UtcTime now) const;
private:
    const TargetId target_;
};
```

## DiagnosticAgent

```cpp
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
```

## RepairCommand

```cpp
class RepairCommand {
public:
    virtual ~RepairCommand();
    virtual Status checkPreconditions(const RepairPlan& plan, Deadline deadline) const = 0;
    virtual ExecutionResult execute(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) = 0;
};
```

## RestartDemoServiceCommand

```cpp
class RestartDemoServiceCommand : public RepairCommand {
public:
    explicit RestartDemoServiceCommand(DemoTargetAdapter& target);
    Status checkPreconditions(const RepairPlan& plan, Deadline deadline) const override;
    ExecutionResult execute(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) override;
private:
    DemoTargetAdapter& target_;
};
```

## RestoreBookingInvariantCommand

```cpp
class RestoreBookingInvariantCommand : public RepairCommand {
public:
    explicit RestoreBookingInvariantCommand(DemoTargetAdapter& target);
    Status checkPreconditions(const RepairPlan& plan, Deadline deadline) const override;
    ExecutionResult execute(const RepairPlan& plan, const DispatchReceipt& dispatch, Deadline deadline) override;
private:
    DemoTargetAdapter& target_;
};
```

## RepairExecutor

```cpp
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
```

## RecoveryVerifier

```cpp
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
```

## IncidentStateMachine

```cpp
class IncidentStateMachine {
public:
    explicit IncidentStateMachine(const Incident& durableIncident);
    bool canTransition(IncidentStatus destination) const;
    Result<Transition> applyEvent(const Incident& incident, const IncidentEvent& event) const;
private:
    IncidentStatus state_;
    Version revision_;
};
```

## IncidentOrchestrator

```cpp
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
```

## OperatorCli

```cpp
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
```

## SetupError

```cpp
class SetupError : public std::runtime_error {
public:
    explicit SetupError(Error failure);
    const Error& failure() const noexcept;
private:
    Error failure_;
};
```

## Enums and aliases
```cpp
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
template<class T> using Result = std::variant<T, Error>;
using Status = Result<std::monostate>;
using Observation = std::variant<HealthObservation, BookingSnapshot, InvariantReport, FailureObservation>;
using ExpectedStateVersion = std::variant<ProcessGeneration, Version>;
using ActionArguments = std::variant<RestartArgs, RestoreInvariantArgs>;
using EventPayload = std::variant<DetectionNotice, BeginDiagnosisNotice, DiagnosisResult, Approval, ExecutionResult, VerificationBatch, TimeoutNotice, ReconciliationNotice>;
```