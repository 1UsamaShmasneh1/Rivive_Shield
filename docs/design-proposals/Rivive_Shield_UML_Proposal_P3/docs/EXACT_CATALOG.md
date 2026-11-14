# P3 exact draft class catalog

**Unapproved proposal. All signatures and fields are draft.**

## Error



```cpp
struct Error {
  ErrorCode code;
  std::string sanitizedMessage;
};
```

## TimeRange

APPROVED: store UTC; client displays in its local timezone; start < end. Half-open interval permits adjacent bookings. Exact wire format remains proposed.

```cpp
struct TimeRange {
  UtcTime start;
  UtcTime end;
};
```

## Room



```cpp
struct Room {
  RoomId id;
  std::string name;
};
```

## BookingRequest

APPROVED scope: ID is an idempotency key; identical retry returns the prior result without duplicate booking. APPROVED scope: different payload with same ID is rejected; exact wire status remains open.

```cpp
struct BookingRequest {
  BookingId id;
  RoomId roomId;
  TimeRange interval;
  std::string syntheticLabel;
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
  Version version;
};
```

## BookingCreateResult

Technical result: created=true only for newly committed booking; false for identical prior request. Enables proposed 201/200 distinction without re-query/race. Wire status mapping remains proposed.

```cpp
struct BookingCreateResult {
  Booking booking;
  bool created;
};
```

## BookingSnapshot



```cpp
struct BookingSnapshot {
  Version revision;
  std::vector<Booking> bookings;
};
```

## SqliteConfiguration

Booking and incident paths must be distinct. No credentials. Journal/durability settings are fixed by reviewed schema policy, not an arbitrary SQL string.

```cpp
struct SqliteConfiguration {
  std::string databasePath;
  std::chrono::milliseconds busyTimeout;
  SqliteOpenMode openMode;
};
```

## HttpServerConfiguration

Local prototype binds 127.0.0.1; no security claim for later Oracle deployment.

```cpp
struct HttpServerConfiguration {
  std::string bindAddress;
  std::uint16_t port;
  std::string staticAssetRoot;
  std::size_t maxRequestBytes;
  std::chrono::milliseconds requestTimeout;
};
```

## BookingRepository

APPROVED: one write transaction checks overlaps, deduplicates and persists request ID/result with unique ID; explicit rollback on failure. Same ID with different data is rejected. Concurrent/crash tests required. Journal/synchronous policy remains open. No disable-guard/cancel/repair API in first milestone.

```cpp
class BookingRepository {
public:
  virtual ~BookingRepository() = default;
  virtual Result<std::vector<Room>> listRooms() const = 0;
  virtual Result<BookingSnapshot> snapshot() const = 0;
  virtual Result<BookingCreateResult> createIfAvailable(const BookingRequest& request, UtcTime now) = 0;
};
```

## SqliteBookingRepository

Open existing/initialize new only in explicit setup mode documented by composition; schema migration failure aborts. Repository reports errors; no exception hidden as success.

```cpp
class SqliteBookingRepository : public BookingRepository {
public:
  explicit SqliteBookingRepository(SqliteConfiguration configuration);
  ~SqliteBookingRepository() override;
  Result<std::vector<Room>> listRooms() const override;
  Result<BookingSnapshot> snapshot() const override;
  Result<BookingCreateResult> createIfAvailable(const BookingRequest& request, UtcTime now) override;
  SqliteBookingRepository(const SqliteBookingRepository&) = delete;
  SqliteBookingRepository& operator=(const SqliteBookingRepository&) = delete;
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
  Result<std::vector<Room>> listRooms() const;
  Result<BookingSnapshot> listBookings() const;
  Result<BookingCreateResult> createBooking(const BookingRequest& request);
  static Status validate(const BookingRequest& request);
  static bool overlaps(const TimeRange& first, const TimeRange& second);
  BookingService(const BookingService&) = delete;
  BookingService& operator=(const BookingService&) = delete;
private:
  BookingRepository& repository_;
  Now now_;
};
```

## BookingHttpServer

Owns external httplib server; run blocks only backend main thread. APPROVED booking scope: list rooms, list bookings and create booking. APPROVED routes: GET /api/rooms, GET /api/bookings, POST /api/bookings. JSON schema, UTC wire representation and HTTP status mapping remain proposed. No dependency on Rivive Shield.

```cpp
class BookingHttpServer {
public:
  BookingHttpServer(BookingService& service, HttpServerConfiguration configuration);
  ~BookingHttpServer();
  Status run();
  void requestStop();
  BookingHttpServer(const BookingHttpServer&) = delete;
  BookingHttpServer& operator=(const BookingHttpServer&) = delete;
private:
  BookingService& service_;
  HttpServerConfiguration configuration_;
  httplib::Server* server_;
  Status registerRoutes();
};
```

## TargetConfiguration

One operator-reviewed local target, fixed executable/arguments; no shell, no caller PID, no remote execution. Paths validated at setup.

```cpp
struct TargetConfiguration {
  std::string targetLabel;
  std::string executablePath;
  std::vector<std::string> fixedArguments;
  std::string workingDirectory;
  std::string bookingDatabasePath;
  std::uint16_t loopbackPort;
};
```

## ProcessIdentity

PID is descriptive, not a mutation authority. Windows authority is retained HANDLE; Mac authority is exclusive unreaped child relationship.

```cpp
struct ProcessIdentity {
  SessionId controllerSessionId;
  Version generation;
  std::uint64_t processId;
  std::string creationToken;
};
```

## ProcessObservation

Exit code alone never classifies user intent or crash cause.

```cpp
struct ProcessObservation {
  ProcessIdentity identity;
  ProcessState state;
  UtcTime observedAt;
  std::optional<std::uint32_t> exitCode;
  bool controllerRequestedStop;
};
```

## CaptureConfiguration

All values positive and reviewed; no invented numeric production defaults. Only loopback IPv4 and port; failure/limit marks uncertainty.

```cpp
struct CaptureConfiguration {
  std::string interfaceName;
  std::uint16_t loopbackPort;
  std::size_t maxPacketBytes;
  std::size_t maxBufferedBytes;
  std::size_t maxFlows;
  std::size_t maxPairsPerFlow;
  std::chrono::milliseconds flowIdleTimeout;
  std::chrono::milliseconds operationObservationLimit;
};
```

## FlowKey



```cpp
struct FlowKey {
  std::string clientAddress;
  std::uint16_t clientPort;
  std::string serverAddress;
  std::uint16_t serverPort;
  bool operator==(const FlowKey& other) const;
};
```

## ConnectionSnapshot

SnapshotCandidate means match NOW, not packet-time ownership. None/Unknown never authorize repair.

```cpp
struct ConnectionSnapshot {
  FlowKey flow;
  UtcTime queryStartedAt;
  UtcTime queryFinishedAt;
  std::optional<ProcessIdentity> expectedTarget;
  Attribution attribution;
};
```

## HttpObservation

No URI/body/header values. Incomplete/unsupported is not success; HTTP status and complete transport are independent.

```cpp
struct HttpObservation {
  FlowKey flow;
  std::uint64_t requestOrdinal;
  HttpMethod method;
  HttpOutcome outcome;
  std::optional<unsigned> statusCode;
  std::optional<std::chrono::microseconds> observedInterval;
  UtcTime observedAt;
  bool captureUncertain;
  std::optional<ConnectionSnapshot> ownerSnapshot;
};
```

## DatabaseCheck

Read-only open without CREATE; preserve sidecars. Busy/missing/invalid/unknown never PASS. Not business invariants or forensic immutability.

```cpp
struct DatabaseCheck {
  CheckOutcome integrity;
  CheckOutcome foreignKeys;
  bool writersConfirmedStopped;
  UtcTime checkedAt;
  std::string sanitizedDetail;
  ProcessIdentity checkedGeneration;
  std::string configuredDatabaseKey;
};
```

## Evidence

Immutable accepted metadata; no raw packet or request payload in stored evidence.

```cpp
struct Evidence {
  EvidenceId id;
  UtcTime observedAt;
  EvidenceKind kind;
  std::variant<ProcessObservation, HttpObservation, DatabaseCheck, Error> observation;
};
```

## CaptureStatistics



```cpp
struct CaptureStatistics {
  std::uint64_t packetsRead;
  std::uint64_t packetsDropped;
  bool statisticsAvailable;
  bool uncertain;
};
```

## PacketView

Borrowed memory valid only during callback; decoder must not retain pointer.

```cpp
struct PacketView {
  const std::uint8_t* bytes;
  std::size_t size;
  int linkType;
  UtcTime captureTimestamp;
};
```

## StreamChunk

Temporary bytes owned by reassembler; missing marker is never fed to HTTP parser. connectionClosed does not infer application cause.

```cpp
struct StreamChunk {
  FlowKey flow;
  Direction direction;
  const std::uint8_t* bytes;
  std::size_t size;
  bool bytesMissing;
  bool beganAtSyn;
  bool connectionClosed;
};
```

## OwnedDemoProcess

Only controller-created fixed target; graceful stop is platform capability, Unsupported is valid. Do not terminate/reopen arbitrary PID.

```cpp
class OwnedDemoProcess {
public:
  virtual ~OwnedDemoProcess() = default;
  virtual Result<ProcessIdentity> start(Deadline deadline) = 0;
  virtual Result<ProcessObservation> observe() = 0;
  virtual Status requestGracefulStop(const ProcessIdentity& expected, Deadline deadline) = 0;
  virtual Status forceStop(const ProcessIdentity& expected, Deadline deadline) = 0;
};
```

## WindowsOwnedDemoProcess

Serialized owner thread. Destructor releases handles/resources but never silently forces termination. Mac implementation/permissions pending actual test.

```cpp
class WindowsOwnedDemoProcess : public OwnedDemoProcess {
public:
  WindowsOwnedDemoProcess(TargetConfiguration configuration, SessionId session);
  ~WindowsOwnedDemoProcess() override;
  Result<ProcessIdentity> start(Deadline deadline) override;
  Result<ProcessObservation> observe() override;
  Status requestGracefulStop(const ProcessIdentity& expected, Deadline deadline) override;
  Status forceStop(const ProcessIdentity& expected, Deadline deadline) override;
  WindowsOwnedDemoProcess(const WindowsOwnedDemoProcess&) = delete;
  WindowsOwnedDemoProcess& operator=(const WindowsOwnedDemoProcess&) = delete;
private:
  TargetConfiguration configuration_;
  SessionId session_;
  Version generation_;
  std::optional<ProcessIdentity> identity_;
  bool ownershipUncertain_;
  bool stopRequested_;
  void* processHandle_;
  void* threadHandle_;
};
```

## MacOwnedDemoProcess

Serialized owner thread. Destructor releases handles/resources but never silently forces termination. Mac implementation/permissions pending actual test.

```cpp
class MacOwnedDemoProcess : public OwnedDemoProcess {
public:
  MacOwnedDemoProcess(TargetConfiguration configuration, SessionId session);
  ~MacOwnedDemoProcess() override;
  Result<ProcessIdentity> start(Deadline deadline) override;
  Result<ProcessObservation> observe() override;
  Status requestGracefulStop(const ProcessIdentity& expected, Deadline deadline) override;
  Status forceStop(const ProcessIdentity& expected, Deadline deadline) override;
  MacOwnedDemoProcess(const MacOwnedDemoProcess&) = delete;
  MacOwnedDemoProcess& operator=(const MacOwnedDemoProcess&) = delete;
private:
  TargetConfiguration configuration_;
  SessionId session_;
  Version generation_;
  std::optional<ProcessIdentity> identity_;
  bool ownershipUncertain_;
  bool stopRequested_;
  std::int64_t ownedChildPid_;
  bool childReaped_;
};
```

## ConnectionObserver



```cpp
class ConnectionObserver {
public:
  virtual ~ConnectionObserver() = default;
  virtual Result<ConnectionSnapshot> snapshot(const FlowKey& flow, const ProcessIdentity& expected) const = 0;
};
```

## WindowsConnectionObserver

Mac may return Unsupported/Unknown until implementation verified. Windows snapshot cannot prove packet-time attribution.

```cpp
class WindowsConnectionObserver : public ConnectionObserver {
public:
  WindowsConnectionObserver() = default;
  Result<ConnectionSnapshot> snapshot(const FlowKey& flow, const ProcessIdentity& expected) const override;
  WindowsConnectionObserver(const WindowsConnectionObserver&) = delete;
  WindowsConnectionObserver& operator=(const WindowsConnectionObserver&) = delete;
};
```

## MacConnectionObserver

Mac may return Unsupported/Unknown until implementation verified. Windows snapshot cannot prove packet-time attribution.

```cpp
class MacConnectionObserver : public ConnectionObserver {
public:
  MacConnectionObserver() = default;
  Result<ConnectionSnapshot> snapshot(const FlowKey& flow, const ProcessIdentity& expected) const override;
  MacConnectionObserver(const MacConnectionObserver&) = delete;
  MacConnectionObserver& operator=(const MacConnectionObserver&) = delete;
};
```

## PacketCapture



```cpp
class PacketCapture {
public:
  virtual ~PacketCapture() = default;
  virtual Status open(const CaptureConfiguration& configuration) = 0;
  virtual Result<std::size_t> readBatch(std::size_t maxPackets, const PacketSink& sink, Deadline deadline) = 0;
  virtual Result<CaptureStatistics> statistics() const = 0;
  virtual void close() = 0;
};
```

## PcapLoopbackCapture

External pcap state; Npcap Windows / libpcap Mac. No silent permissions elevation. Filter and link type verified before read.

```cpp
class PcapLoopbackCapture : public PacketCapture {
public:
  PcapLoopbackCapture();
  ~PcapLoopbackCapture() override;
  Status open(const CaptureConfiguration& configuration) override;
  Result<std::size_t> readBatch(std::size_t maxPackets, const PacketSink& sink, Deadline deadline) override;
  Result<CaptureStatistics> statistics() const override;
  void close() override;
  PcapLoopbackCapture(const PcapLoopbackCapture&) = delete;
  PcapLoopbackCapture& operator=(const PcapLoopbackCapture&) = delete;
private:
  pcap* handle_;
  std::optional<CaptureConfiguration> configuration_;
  CaptureStatistics statistics_;
};
```

## TcpStreamDecoder

Wrap PCPP callback; bytesMissing/overflow/midstream forces uncertain flow. close/reset release buffers; never network sockets. Conservative accepted-byte budget is enforced before reassembly; rotation/reset never turns incomplete data into complete evidence.

```cpp
class TcpStreamDecoder {
public:
  TcpStreamDecoder(CaptureConfiguration configuration, StreamSink sink);
  ~TcpStreamDecoder();
  Status accept(const PacketView& packet);
  void expire(SteadyTime now);
  void reset();
  TcpStreamDecoder(const TcpStreamDecoder&) = delete;
  TcpStreamDecoder& operator=(const TcpStreamDecoder&) = delete;
private:
  CaptureConfiguration configuration_;
  StreamSink sink_;
  pcpp::TcpReassembly* reassembly_;
  std::vector<FlowKey> activeFlows_;
  std::size_t acceptedBytes_;
  void deliver(const StreamChunk& chunk);
};
```

## HttpFlowState

Parser pointers are external llhttp state. No application body accumulation. One pending request per flow; later keep-alive is serial.

```cpp
struct HttpFlowState {
  FlowKey flow;
  llhttp__internal_s* requestParser;
  llhttp__internal_s* responseParser;
  std::optional<SteadyTime> requestObservedStart;
  std::uint64_t completedRequests;
  std::uint64_t completedResponses;
  HttpMethod pendingMethod;
  bool requestComplete;
  bool synchronized;
  bool unsupported;
  bool gap;
  bool parseError;
  SteadyTime lastActivity;
  HttpSessionTracker* owner;
};
```

## HttpSessionTracker

GET/POST HTTP1.1 serial only; unsupported 1xx/upgrade/pipeline marked unknown, not silently skipped. Timing is observation duration, not internal execution. Parser callbacks must map to listed helpers. Stable list node addresses back llhttp.data; never move nodes. Free both parsers before erasing a node; callbacks are synchronous on owner thread and cannot survive erase/reset.

```cpp
class HttpSessionTracker {
public:
  HttpSessionTracker(CaptureConfiguration configuration, HttpSink sink, Now now);
  ~HttpSessionTracker();
  Status accept(const StreamChunk& chunk);
  void expire(SteadyTime now);
  void invalidateAll(const Error& reason);
  void reset();
  HttpSessionTracker(const HttpSessionTracker&) = delete;
  HttpSessionTracker& operator=(const HttpSessionTracker&) = delete;
private:
  CaptureConfiguration configuration_;
  HttpSink sink_;
  Now now_;
  std::list<HttpFlowState> flows_;
  Status beginRequest(HttpFlowState& flow);
  Status completeRequest(HttpFlowState& flow);
  Status completeResponse(HttpFlowState& flow);
  void markUncertain(HttpFlowState& flow, HttpOutcome outcome);
  llhttp_settings_s* parserSettings_;
};
```

## Monitor

Passive observation; no automatic HTTP health/booking requests. Absence of traffic is not health. OS process checks are separate from network polling.

```cpp
class Monitor {
public:
  Monitor(PacketCapture& capture, TcpStreamDecoder& tcp, HttpSessionTracker& http, OwnedDemoProcess& process, const ConnectionObserver& connections, EvidenceSink sink, Now now);
  Status start(const CaptureConfiguration& configuration);
  Status tick(Deadline deadline);
  void stop();
  void receiveHttp(const HttpObservation& observation);
  Monitor(const Monitor&) = delete;
  Monitor& operator=(const Monitor&) = delete;
private:
  PacketCapture& capture_;
  TcpStreamDecoder& tcp_;
  HttpSessionTracker& http_;
  OwnedDemoProcess& process_;
  const ConnectionObserver& connections_;
  EvidenceSink sink_;
  Now now_;
  std::optional<ProcessObservation> lastProcess_;
};
```

## RuleAudit



```cpp
struct RuleAudit {
  std::string rulesetVersion;
  std::optional<std::string> matchedRuleId;
  std::vector<EvidenceId> evidenceRefs;
  UtcTime evaluatedAt;
};
```

## RepairPlan

Immutable exact action/target revision. Proposed actions only StartStoppedOwnedTarget or ForceStopOwnedTarget. Force stop is a separate displayed/approved plan; never inferred approval.

```cpp
struct RepairPlan {
  PlanId id;
  IncidentId incidentId;
  Version revision;
  ActionId action;
  ProcessIdentity expectedProcess;
  std::vector<EvidenceId> evidenceRefs;
  UtcTime createdAt;
  UtcTime expiresAt;
  std::string explanation;
};
```

## Approval



```cpp
struct Approval {
  ApprovalId id;
  PlanId planId;
  Version planRevision;
  IncidentId incidentId;
  Version incidentRevision;
  ApprovalDecision decision;
  UtcTime decidedAt;
  std::string operatorLabel;
  bool matches(const RepairPlan& plan) const;
};
```

## Incident



```cpp
struct Incident {
  IncidentId id;
  Version revision;
  IncidentStatus status;
  IncidentKind kind;
  UtcTime detectedAt;
  std::vector<Evidence> evidence;
  std::vector<RepairPlan> plans;
  std::vector<Approval> approvals;
  std::vector<RuleAudit> ruleAudits;
};
```

## DiagnosisResult

HTTP-only symptoms/unknown ownership may produce no plan. No inference of cause from status alone.

```cpp
struct DiagnosisResult {
  IncidentKind kind;
  std::optional<RepairPlan> proposedPlan;
  RuleAudit audit;
  std::string explanation;
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

Maybe-invoked failure is Uncertain; no automatic retry.

```cpp
struct ExecutionResult {
  AttemptId attemptId;
  ExecutionOutcome outcome;
  std::optional<ProcessObservation> resultingProcess;
  std::optional<Error> failure;
  UtcTime finishedAt;
};
```

## VerificationBatch

Functional confirmation comes from a separately labeled operator-driven synthetic test; no automatic modifying booking probe. NotRun blocks Recovered.

```cpp
struct VerificationBatch {
  AttemptId attemptId;
  CheckOutcome processCheck;
  CheckOutcome databaseCheck;
  CheckOutcome functionalCheck;
  std::vector<Evidence> evidence;
};
```

## IncidentRecord



```cpp
struct IncidentRecord {
  Version recordVersion;
  Incident incident;
  std::vector<RepairAttempt> attempts;
  std::vector<ExecutionResult> executions;
  std::vector<VerificationBatch> verifications;
  std::vector<EventId> acceptedEventIds;
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

## ControlEvent

Technical typed control event, never a magic Error. BeginDiagnosis/PlanInvalidated carry no plan; ManualStopProposal carries a proposed ForceStop plan bound to selected owned target. It is not approval or dispatch authority.

```cpp
struct ControlEvent {
  ControlEventKind kind;
  std::optional<RepairPlan> proposedPlan;
  std::string reasonCode;
};
```

## IncidentEvent



```cpp
struct IncidentEvent {
  EventId id;
  IncidentId incidentId;
  Version expectedRevision;
  UtcTime observedAt;
  std::variant<Evidence, DiagnosisResult, Approval, ExecutionResult, VerificationBatch, ControlEvent, Error> payload;
};
```

## Transition



```cpp
struct Transition {
  IncidentStatus from;
  IncidentStatus to;
  Version nextRevision;
  std::string reason;
};
```

## OperatorView

Bounded metadata only; durable=false conspicuous; never show unsaved recovery.

```cpp
struct OperatorView {
  std::optional<IncidentRecord> current;
  std::vector<HttpObservation> recentActivity;
  std::optional<ProcessObservation> process;
  bool durable;
  bool repairsBlocked;
  std::optional<Error> visibleError;
  std::vector<IncidentRecord> historyPage;
  std::size_t historyOffset;
  bool historyPending;
  std::optional<Error> historyError;
  bool settingsPending;
  bool settingsSaved;
  std::optional<Error> settingsError;
};
```

## IncidentRepository



```cpp
class IncidentRepository {
public:
  virtual ~IncidentRepository() = default;
  virtual Result<SaveReceipt> save(const IncidentRecord& record, Version expectedRecordVersion) = 0;
  virtual Result<IncidentRecord> load(const IncidentId& id) const = 0;
  virtual Result<std::vector<IncidentRecord>> loadPending() const = 0;
  virtual Result<DispatchReceipt> claimDispatch(const IncidentId& id, const AttemptId& attempt, Version expectedRecordVersion, UtcTime now) = 0;
  virtual Result<std::vector<IncidentRecord>> history(std::size_t offset, std::size_t limit) const = 0;
};
```

## SqliteIncidentRepository



```cpp
class SqliteIncidentRepository : public IncidentRepository {
public:
  explicit SqliteIncidentRepository(SqliteConfiguration configuration);
  ~SqliteIncidentRepository() override;
  Result<SaveReceipt> save(const IncidentRecord& record, Version expectedRecordVersion) override;
  Result<IncidentRecord> load(const IncidentId& id) const override;
  Result<std::vector<IncidentRecord>> loadPending() const override;
  Result<DispatchReceipt> claimDispatch(const IncidentId& id, const AttemptId& attempt, Version expectedRecordVersion, UtcTime now) override;
  Result<std::vector<IncidentRecord>> history(std::size_t offset, std::size_t limit) const override;
  SqliteIncidentRepository(const SqliteIncidentRepository&) = delete;
  SqliteIncidentRepository& operator=(const SqliteIncidentRepository&) = delete;
private:
  SqliteConfiguration configuration_;
  sqlite3* database_;
  mutable std::mutex mutex_;
};
```

## DatabaseVerifier

Verify expected local configured DB and writers stopped; no CREATE; no repair SQL; no sidecar deletion. Other possible writers => Inconclusive.

```cpp
class DatabaseVerifier {
public:
  DatabaseVerifier() = default;
  Result<DatabaseCheck> check(const std::string& databasePath, const ProcessObservation& stoppedProcess, Deadline deadline) const;
  DatabaseVerifier(const DatabaseVerifier&) = delete;
  DatabaseVerifier& operator=(const DatabaseVerifier&) = delete;
};
```

## PlanPolicy



```cpp
class PlanPolicy {
public:
  explicit PlanPolicy(TargetConfiguration configuration);
  Status validate(const RepairPlan& plan, const Incident& incident, UtcTime now) const;
  Status revalidate(const RepairPlan& plan, const Approval& approval, const Incident& incident, const ProcessObservation& current, UtcTime now) const;
  PlanPolicy(const PlanPolicy&) = delete;
  PlanPolicy& operator=(const PlanPolicy&) = delete;
private:
  TargetConfiguration configuration_;
};
```

## DiagnosticAgent

Compiled-in versioned rules only. Unknown cause/ownership cannot become destructive authorization.

```cpp
class DiagnosticAgent {
public:
  DiagnosticAgent(const PlanPolicy& policy, Now now);
  Result<DiagnosisResult> propose(const Incident& incident, const PlanId& newPlanId, Version newPlanRevision, UtcTime expiresAt) const;
  DiagnosticAgent(const DiagnosticAgent&) = delete;
  DiagnosticAgent& operator=(const DiagnosticAgent&) = delete;
private:
  const PlanPolicy& policy_;
  Now now_;
  DiagnosisResult evaluateCompiledRules(const Incident& incident, const PlanId& planId, Version revision, UtcTime expiresAt) const;
};
```

## RepairExecutor

Reload authoritative exact record; validate; durable CAS claim; invoke allowlisted action once. No automatic replay after uncertain result or restart.

```cpp
class RepairExecutor {
public:
  RepairExecutor(OwnedDemoProcess& target, IncidentRepository& repository, const PlanPolicy& policy, Now now);
  Result<ExecutionResult> executeOnce(const PlanId& planId, const ApprovalId& approvalId, const AttemptId& attemptId, const IncidentId& incidentId, Deadline deadline);
  RepairExecutor(const RepairExecutor&) = delete;
  RepairExecutor& operator=(const RepairExecutor&) = delete;
private:
  OwnedDemoProcess& target_;
  IncidentRepository& repository_;
  const PlanPolicy& policy_;
  Now now_;
  std::mutex executionMutex_;
};
```

## RecoveryVerifier

Stop is not recovery. For start, valid prior stopped DB check + new process + independent later functional evidence required; no requests replayed.

```cpp
class RecoveryVerifier {
public:
  RecoveryVerifier(OwnedDemoProcess& target, const DatabaseVerifier& database, TargetConfiguration configuration, Now now);
  Result<VerificationBatch> verify(const RepairPlan& approvedPlan, const RepairAttempt& attempt, const ExecutionResult& execution, const DatabaseCheck& priorStoppedCheck, const std::vector<Evidence>& laterEvidence, Deadline deadline) const;
  RecoveryVerifier(const RecoveryVerifier&) = delete;
  RecoveryVerifier& operator=(const RecoveryVerifier&) = delete;
private:
  OwnedDemoProcess& target_;
  const DatabaseVerifier& database_;
  TargetConfiguration configuration_;
  Now now_;
};
```

## IncidentStateMachine

APPROVED state names: Detected, Diagnosing, AwaitingApproval, Executing, Verifying, Recovered, NeedsReview, Declined, Unresolved. No closing an incident with unresolved side effects. Exact transition rules remain under review. Pure function; no persistence, process or network side effects.

```cpp
class IncidentStateMachine {
public:
  IncidentStateMachine() = default;
  Result<Transition> apply(const IncidentRecord& current, const IncidentEvent& event) const;
  IncidentStateMachine(const IncidentStateMachine&) = delete;
  IncidentStateMachine& operator=(const IncidentStateMachine&) = delete;
};
```

## IncidentOrchestrator

APPROVED scope: one active incident per configured target, aggregate observations, no concurrent actions, and no old approval reused for a new incident. Single background owner; callbacks enqueue only. No lock across I/O. Store failure blocks actions. Old dispatch remains NeedsReview after controller restart. requestHistory validates bounded page size and enqueues under queueMutex_. processPending loads history on owner thread and publishes immutable page under viewMutex_; GUI never calls repository. publishSettingsResult is owner-thread only. view() reads only publishedView_ under viewMutex_, never storage/current_ from UI. Publish a value-copy after commit or visible failure. Manual stop proposal is not approval. It requires a subsequent exact plan decision; diagnostics cannot create force-stop authority from HTTP-only evidence.

```cpp
class IncidentOrchestrator {
public:
  IncidentOrchestrator(IncidentRepository& repository, DiagnosticAgent& diagnosis, RepairExecutor& executor, RecoveryVerifier& verifier, const IncidentStateMachine& stateMachine, Now now, std::size_t queueCapacity);
  Status enqueue(IncidentEvent event);
  Result<std::size_t> processPending(std::size_t maximum, Deadline deadline);
  Result<OperatorView> view() const;
  Status reconcileAfterRestart();
  Status requestHistory(std::size_t offset, std::size_t limit);
  void publishSettingsResult(bool pending, bool saved, std::optional<Error> error);
  Result<bool> mayStartForSetup() const;
  Status proposeManualStop(const IncidentId& incidentId, const ProcessIdentity& expected, const PlanId& planId, UtcTime expiresAt);
  IncidentOrchestrator(const IncidentOrchestrator&) = delete;
  IncidentOrchestrator& operator=(const IncidentOrchestrator&) = delete;
private:
  IncidentRepository& repository_;
  DiagnosticAgent& diagnosis_;
  RepairExecutor& executor_;
  RecoveryVerifier& verifier_;
  const IncidentStateMachine& stateMachine_;
  Now now_;
  std::size_t queueCapacity_;
  std::deque<IncidentEvent> events_;
  std::optional<IncidentRecord> current_;
  bool repairsBlocked_;
  std::optional<Error> volatileFailure_;
  mutable std::mutex queueMutex_;
  std::thread::id ownerThread_;
  std::deque<std::pair<std::size_t, std::size_t>> historyRequests_;
  Status processHistoryRequest();
  OperatorView publishedView_;
  mutable std::mutex viewMutex_;
};
```

## SettingsRepository



```cpp
class SettingsRepository {
public:
  virtual ~SettingsRepository() = default;
  virtual Result<TargetConfiguration> load() const = 0;
  virtual Status save(const TargetConfiguration& configuration) = 0;
};
```

## LocalSettingsRepository

Local bounded config, atomic replacement; no credentials; no arbitrary commands.

```cpp
class LocalSettingsRepository : public SettingsRepository {
public:
  explicit LocalSettingsRepository(std::string path);
  Result<TargetConfiguration> load() const override;
  Status save(const TargetConfiguration& configuration) override;
  LocalSettingsRepository(const LocalSettingsRepository&) = delete;
  LocalSettingsRepository& operator=(const LocalSettingsRepository&) = delete;
private:
  std::string path_;
};
```

## DesktopView

UI thread boundary, wxWidgets external scheduler; no state transitions in view.

```cpp
class DesktopView {
public:
  virtual ~DesktopView() = default;
  virtual void render(const OperatorView& view) = 0;
  virtual void showError(const Error& error) = 0;
  virtual void postToUi(std::function<void()> work) = 0;
};
```

## WxDesktopView

External wx object tree owns widgets. Constructor/destructor and event bindings UI-thread only; widget IDs/layout are not business contracts. Callback binding requires exact reviewed composition.

```cpp
class WxDesktopView : public DesktopView {
public:
  WxDesktopView();
  ~WxDesktopView() override;
  Status create();
  void show();
  void render(const OperatorView& view) override;
  void showError(const Error& error) override;
  void postToUi(std::function<void()> work) override;
  WxDesktopView(const WxDesktopView&) = delete;
  WxDesktopView& operator=(const WxDesktopView&) = delete;
private:
  wxFrame* frame_;
  wxPanel* applicationPanel_;
  wxPanel* activityPanel_;
  wxPanel* incidentPanel_;
  wxPanel* historyPanel_;
};
```

## DesktopPresenter

No waiting on I/O in GUI event handlers; settings change forbidden while target/incident active. Settings apply next explicit setup session. saveSettings delegates only to runtime.requestSettingsSave; returned Status means queued, not persisted. requestHistory delegates to core queue.

```cpp
class DesktopPresenter {
public:
  DesktopPresenter(DesktopView& view, IncidentOrchestrator& core, ControllerRuntime& runtime);
  Status saveSettings(const TargetConfiguration& configuration);
  Status submitDecision(const IncidentEvent& approvalEvent);
  void refresh();
  Status requestHistory(std::size_t offset, std::size_t limit);
  DesktopPresenter(const DesktopPresenter&) = delete;
  DesktopPresenter& operator=(const DesktopPresenter&) = delete;
private:
  DesktopView& view_;
  IncidentOrchestrator& core_;
  ControllerRuntime& runtime_;
};
```

## ControllerRuntime

Settings saves are queued with bounded capacity, processed only by owner thread after stopped/no-active-incident gate, then published through core.publishSettingsResult. No settings I/O in GUI. One background control thread; bounded tick; join before releasing dependencies. requestInitialStart queues only. Owner thread checks core.mayStartForSetup(), no pending durable incidents and current NotStarted/Exited before fixed process start. No start bypass during incident. Window close does not silently kill booking.

```cpp
class ControllerRuntime {
public:
  ControllerRuntime(Monitor& monitor, IncidentOrchestrator& core, OwnedDemoProcess& process, DesktopView& view, SettingsRepository& settings);
  ~ControllerRuntime();
  Status startMonitoring(const CaptureConfiguration& configuration);
  Status stopMonitoring(Deadline deadline);
  Status requestSettingsSave(const TargetConfiguration& configuration);
  Status requestInitialStart();
  ControllerRuntime(const ControllerRuntime&) = delete;
  ControllerRuntime& operator=(const ControllerRuntime&) = delete;
private:
  Monitor& monitor_;
  IncidentOrchestrator& core_;
  OwnedDemoProcess& process_;
  DesktopView& view_;
  std::thread worker_;
  std::atomic<bool> stopRequested_;
  SettingsRepository& settings_;
  std::deque<TargetConfiguration> settingsRequests_;
  std::mutex settingsMutex_;
  Status processSettingsRequest();
  std::atomic<bool> initialStartRequested_;
};
```

