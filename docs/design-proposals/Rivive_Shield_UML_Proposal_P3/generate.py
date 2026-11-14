from pathlib import Path
import json, html, subprocess, hashlib
root=Path(__file__).parent
items=[]
def add(name,kind,group,fields=(),methods=(),private=(),base='',note=''):
    items.append(dict(name=name,kind=kind,group=group,fields=list(fields),methods=list(methods),private=list(private),base=base,note=note))
# All declarations below are NEW PROPOSAL P3, NOT APPROVED.
add('Error','struct','values',['ErrorCode code;','std::string sanitizedMessage;'])
add('TimeRange','struct','booking',['UtcTime start;','UtcTime end;'],note='APPROVED: store UTC; client displays in its local timezone; start < end. Half-open interval permits adjacent bookings. Exact wire format remains proposed.')
add('Room','struct','booking',['RoomId id;','std::string name;'])
add('BookingRequest','struct','booking',['BookingId id;','RoomId roomId;','TimeRange interval;','std::string syntheticLabel;'],note='APPROVED scope: ID is an idempotency key; identical retry returns the prior result without duplicate booking. APPROVED scope: different payload with same ID is rejected; exact wire status remains open.')
add('Booking','struct','booking',['BookingId id;','RoomId roomId;','TimeRange interval;','std::string syntheticLabel;','UtcTime createdAt;','Version version;'])
add('BookingCreateResult','struct','booking',['Booking booking;','bool created;'],note='Technical result: created=true only for newly committed booking; false for identical prior request. Enables proposed 201/200 distinction without re-query/race. Wire status mapping remains proposed.')
add('BookingSnapshot','struct','booking',['Version revision;','std::vector<Booking> bookings;'])
add('SqliteConfiguration','struct','values',['std::string databasePath;','std::chrono::milliseconds busyTimeout;'],note='Booking and incident paths must be distinct. No credentials. Journal/durability settings are fixed by reviewed schema policy, not an arbitrary SQL string.')
add('HttpServerConfiguration','struct','booking',['std::string bindAddress;','std::uint16_t port;','std::string staticAssetRoot;','std::size_t maxRequestBytes;','std::chrono::milliseconds requestTimeout;'],note='Local prototype binds 127.0.0.1; no security claim for later Oracle deployment.')
add('BookingRepository','class','booking',methods=['virtual ~BookingRepository() = default;','virtual Result<std::vector<Room>> listRooms() const = 0;','virtual Result<BookingSnapshot> snapshot() const = 0;','virtual Result<BookingCreateResult> createIfAvailable(const BookingRequest& request, UtcTime now) = 0;'],note='APPROVED: one write transaction checks overlaps, deduplicates and persists request ID/result with unique ID; explicit rollback on failure. Same ID with different data is rejected. Concurrent/crash tests required. Journal/synchronous policy remains open. No disable-guard/cancel/repair API in first milestone.')
add('SqliteBookingRepository','class','booking',base='BookingRepository',methods=['explicit SqliteBookingRepository(SqliteConfiguration configuration);','~SqliteBookingRepository() override;','Result<std::vector<Room>> listRooms() const override;','Result<BookingSnapshot> snapshot() const override;','Result<BookingCreateResult> createIfAvailable(const BookingRequest& request, UtcTime now) override;'],private=['SqliteConfiguration configuration_;','sqlite3* database_;','mutable std::mutex mutex_;'],note='Open existing/initialize new only in explicit setup mode documented by composition; schema migration failure aborts. Repository reports errors; no exception hidden as success.')
add('BookingService','class','booking',methods=['BookingService(BookingRepository& repository, Now now);','Result<std::vector<Room>> listRooms() const;','Result<BookingSnapshot> listBookings() const;','Result<BookingCreateResult> createBooking(const BookingRequest& request);','static Status validate(const BookingRequest& request);','static bool overlaps(const TimeRange& first, const TimeRange& second);'],private=['BookingRepository& repository_;','Now now_;'])
add('BookingHttpServer','class','booking',methods=['BookingHttpServer(BookingService& service, HttpServerConfiguration configuration);','~BookingHttpServer();','Status run();','void requestStop();'],private=['BookingService& service_;','HttpServerConfiguration configuration_;','httplib::Server* server_;','Status registerRoutes();'],note='Owns external httplib server; run blocks only backend main thread. APPROVED booking scope: list rooms, list bookings and create booking. APPROVED routes: GET /api/rooms, GET /api/bookings, POST /api/bookings. JSON schema, UTC wire representation and HTTP status mapping remain proposed. No dependency on Rivive Shield.')
# Process + packet boundary
add('TargetConfiguration','struct','values',['std::string targetLabel;','std::string executablePath;','std::vector<std::string> fixedArguments;','std::string workingDirectory;','std::string bookingDatabasePath;','std::uint16_t loopbackPort;'],note='One operator-reviewed local target, fixed executable/arguments; no shell, no caller PID, no remote execution. Paths validated at setup.')
add('ProcessIdentity','struct','values',['SessionId controllerSessionId;','Version generation;','std::uint64_t processId;','std::string creationToken;'],note='PID is descriptive, not a mutation authority. Windows authority is retained HANDLE; Mac authority is exclusive unreaped child relationship.')
add('ProcessObservation','struct','values',['ProcessIdentity identity;','ProcessState state;','UtcTime observedAt;','std::optional<std::uint32_t> exitCode;','bool controllerRequestedStop;'],note='Exit code alone never classifies user intent or crash cause.')
add('CaptureConfiguration','struct','values',['std::string interfaceName;','std::uint16_t loopbackPort;','std::size_t maxPacketBytes;','std::size_t maxBufferedBytes;','std::size_t maxFlows;','std::size_t maxPairsPerFlow;','std::chrono::milliseconds flowIdleTimeout;','std::chrono::milliseconds operationObservationLimit;'],note='All values positive and reviewed; no invented numeric production defaults. Only loopback IPv4 and port; failure/limit marks uncertainty.')
add('FlowKey','struct','values',['std::string clientAddress;','std::uint16_t clientPort;','std::string serverAddress;','std::uint16_t serverPort;'],methods=['bool operator==(const FlowKey& other) const;'])
add('ConnectionSnapshot','struct','values',['FlowKey flow;','UtcTime queryStartedAt;','UtcTime queryFinishedAt;','std::optional<ProcessIdentity> expectedTarget;','Attribution attribution;'],note='SnapshotCandidate means match NOW, not packet-time ownership. None/Unknown never authorize repair.')
add('HttpObservation','struct','values',['FlowKey flow;','std::uint64_t requestOrdinal;','HttpMethod method;','HttpOutcome outcome;','std::optional<unsigned> statusCode;','std::optional<std::chrono::microseconds> observedInterval;','UtcTime observedAt;','bool captureUncertain;','std::optional<ConnectionSnapshot> ownerSnapshot;'],note='No URI/body/header values. Incomplete/unsupported is not success; HTTP status and complete transport are independent.')
add('DatabaseCheck','struct','values',['CheckOutcome integrity;','CheckOutcome foreignKeys;','bool writersConfirmedStopped;','UtcTime checkedAt;','std::string sanitizedDetail;'],note='Read-only open without CREATE; preserve sidecars. Busy/missing/invalid/unknown never PASS. Not business invariants or forensic immutability.')
add('Evidence','struct','values',['EvidenceId id;','UtcTime observedAt;','EvidenceKind kind;','std::variant<ProcessObservation, HttpObservation, DatabaseCheck, Error> observation;'],note='Immutable accepted metadata; no raw packet or request payload in stored evidence.')
add('CaptureStatistics','struct','values',['std::uint64_t packetsRead;','std::uint64_t packetsDropped;','bool statisticsAvailable;','bool uncertain;'])
add('PacketView','struct','values',['const std::uint8_t* bytes;','std::size_t size;','int linkType;','UtcTime captureTimestamp;'],note='Borrowed memory valid only during callback; decoder must not retain pointer.')
add('StreamChunk','struct','values',['FlowKey flow;','Direction direction;','const std::uint8_t* bytes;','std::size_t size;','bool bytesMissing;','bool beganAtSyn;','bool connectionClosed;'],note='Temporary bytes owned by reassembler; missing marker is never fed to HTTP parser. connectionClosed does not infer application cause.')
add('OwnedDemoProcess','class','process',methods=['virtual ~OwnedDemoProcess() = default;','virtual Result<ProcessIdentity> start(Deadline deadline) = 0;','virtual Result<ProcessObservation> observe() = 0;','virtual Status requestGracefulStop(const ProcessIdentity& expected, Deadline deadline) = 0;','virtual Status forceStop(const ProcessIdentity& expected, Deadline deadline) = 0;'],note='Only controller-created fixed target; graceful stop is platform capability, Unsupported is valid. Do not terminate/reopen arbitrary PID.')
for name,fields in [('WindowsOwnedDemoProcess',['void* processHandle_;','void* threadHandle_;']),('MacOwnedDemoProcess',['std::int64_t ownedChildPid_;','bool childReaped_;'])]:
 add(name,'class','process',base='OwnedDemoProcess',methods=[f'{name}(TargetConfiguration configuration, SessionId session);',f'~{name}() override;','Result<ProcessIdentity> start(Deadline deadline) override;','Result<ProcessObservation> observe() override;','Status requestGracefulStop(const ProcessIdentity& expected, Deadline deadline) override;','Status forceStop(const ProcessIdentity& expected, Deadline deadline) override;'],private=['TargetConfiguration configuration_;','SessionId session_;','Version generation_;','std::optional<ProcessIdentity> identity_;','bool ownershipUncertain_;','bool stopRequested_;']+fields,note='Serialized owner thread. Destructor releases handles/resources but never silently forces termination. Mac implementation/permissions pending actual test.')
add('ConnectionObserver','class','process',methods=['virtual ~ConnectionObserver() = default;','virtual Result<ConnectionSnapshot> snapshot(const FlowKey& flow, const ProcessIdentity& expected) const = 0;'])
for name in ['WindowsConnectionObserver','MacConnectionObserver']:
 add(name,'class','process',base='ConnectionObserver',methods=['Result<ConnectionSnapshot> snapshot(const FlowKey& flow, const ProcessIdentity& expected) const override;'],note='Mac may return Unsupported/Unknown until implementation verified. Windows snapshot cannot prove packet-time attribution.')
add('PacketCapture','class','monitor',methods=['virtual ~PacketCapture() = default;','virtual Status open(const CaptureConfiguration& configuration) = 0;','virtual Result<std::size_t> readBatch(std::size_t maxPackets, const PacketSink& sink, Deadline deadline) = 0;','virtual Result<CaptureStatistics> statistics() const = 0;','virtual void close() = 0;'])
add('PcapLoopbackCapture','class','monitor',base='PacketCapture',methods=['PcapLoopbackCapture();','~PcapLoopbackCapture() override;','Status open(const CaptureConfiguration& configuration) override;','Result<std::size_t> readBatch(std::size_t maxPackets, const PacketSink& sink, Deadline deadline) override;','Result<CaptureStatistics> statistics() const override;','void close() override;'],private=['pcap* handle_;','std::optional<CaptureConfiguration> configuration_;','CaptureStatistics statistics_;'],note='External pcap state; Npcap Windows / libpcap Mac. No silent permissions elevation. Filter and link type verified before read.')
add('TcpStreamDecoder','class','monitor',methods=['TcpStreamDecoder(CaptureConfiguration configuration, StreamSink sink);','~TcpStreamDecoder();','Status accept(const PacketView& packet);','void expire(SteadyTime now);','void reset();'],private=['CaptureConfiguration configuration_;','StreamSink sink_;','pcpp::TcpReassembly* reassembly_;','std::vector<FlowKey> activeFlows_;','std::size_t bufferedBytes_;','void deliver(const StreamChunk& chunk);'],note='Wrap PCPP callback; bytesMissing/overflow/midstream forces uncertain flow. close/reset release buffers; never network sockets.')
add('HttpFlowState','struct','monitor',['FlowKey flow;','llhttp__internal_s* requestParser;','llhttp__internal_s* responseParser;','std::optional<SteadyTime> requestObservedStart;','std::uint64_t completedRequests;','std::uint64_t completedResponses;','HttpMethod pendingMethod;','bool requestComplete;','bool synchronized;','bool unsupported;','bool gap;','bool parseError;','SteadyTime lastActivity;'],note='Parser pointers are external llhttp state. No application body accumulation. One pending request per flow; later keep-alive is serial.')
add('HttpSessionTracker','class','monitor',methods=['HttpSessionTracker(CaptureConfiguration configuration, HttpSink sink, Now now);','~HttpSessionTracker();','Status accept(const StreamChunk& chunk);','void expire(SteadyTime now);','void invalidateAll(const Error& reason);','void reset();'],private=['CaptureConfiguration configuration_;','HttpSink sink_;','Now now_;','std::list<HttpFlowState> flows_;','Status beginRequest(HttpFlowState& flow);','Status completeRequest(HttpFlowState& flow);','Status completeResponse(HttpFlowState& flow);','void markUncertain(HttpFlowState& flow, HttpOutcome outcome);'],note='GET/POST HTTP1.1 serial only; unsupported 1xx/upgrade/pipeline marked unknown, not silently skipped. Timing is observation duration, not internal execution. Parser callbacks must map to listed helpers.')
add('Monitor','class','monitor',methods=['Monitor(PacketCapture& capture, TcpStreamDecoder& tcp, HttpSessionTracker& http, OwnedDemoProcess& process, const ConnectionObserver& connections, EvidenceSink sink, Now now);','Status start(const CaptureConfiguration& configuration);','Status tick(Deadline deadline);','void stop();'],private=['PacketCapture& capture_;','TcpStreamDecoder& tcp_;','HttpSessionTracker& http_;','OwnedDemoProcess& process_;','const ConnectionObserver& connections_;','EvidenceSink sink_;','Now now_;','std::optional<ProcessObservation> lastProcess_;','void receiveHttp(const HttpObservation& observation);'],note='Passive observation; no automatic HTTP health/booking requests. Absence of traffic is not health. OS process checks are separate from network polling.')
# Incident domain/recovery
add('RuleAudit','struct','incident',['std::string rulesetVersion;','std::optional<std::string> matchedRuleId;','std::vector<EvidenceId> evidenceRefs;','UtcTime evaluatedAt;'])
add('RepairPlan','struct','incident',['PlanId id;','IncidentId incidentId;','Version revision;','ActionId action;','ProcessIdentity expectedProcess;','std::vector<EvidenceId> evidenceRefs;','UtcTime createdAt;','UtcTime expiresAt;','std::string explanation;'],note='Immutable exact action/target revision. Proposed actions only StartStoppedOwnedTarget or ForceStopOwnedTarget. Force stop is a separate displayed/approved plan; never inferred approval.')
add('Approval','struct','incident',['ApprovalId id;','PlanId planId;','Version planRevision;','IncidentId incidentId;','Version incidentRevision;','ApprovalDecision decision;','UtcTime decidedAt;','std::string operatorLabel;'],methods=['bool matches(const RepairPlan& plan) const;'])
add('Incident','struct','incident',['IncidentId id;','Version revision;','IncidentStatus status;','IncidentKind kind;','UtcTime detectedAt;','std::vector<Evidence> evidence;','std::vector<RepairPlan> plans;','std::vector<Approval> approvals;','std::vector<RuleAudit> ruleAudits;'])
add('DiagnosisResult','struct','incident',['IncidentKind kind;','std::optional<RepairPlan> proposedPlan;','RuleAudit audit;','std::string explanation;'],note='HTTP-only symptoms/unknown ownership may produce no plan. No inference of cause from status alone.')
add('RepairAttempt','struct','incident',['AttemptId id;','IncidentId incidentId;','PlanId planId;','Version planRevision;','ApprovalId approvalId;','AttemptStatus status;','UtcTime preparedAt;','std::optional<UtcTime> dispatchClaimedAt;'])
add('DispatchReceipt','struct','incident',['AttemptId attemptId;','IncidentId incidentId;','Version recordVersion;','UtcTime claimedAt;'])
add('ExecutionResult','struct','incident',['AttemptId attemptId;','ExecutionOutcome outcome;','std::optional<ProcessObservation> resultingProcess;','std::optional<Error> failure;','UtcTime finishedAt;'],note='Maybe-invoked failure is Uncertain; no automatic retry.')
add('VerificationBatch','struct','incident',['AttemptId attemptId;','CheckOutcome processCheck;','CheckOutcome databaseCheck;','CheckOutcome functionalCheck;','std::vector<Evidence> evidence;'],note='Functional confirmation comes from a separately labeled operator-driven synthetic test; no automatic modifying booking probe. NotRun blocks Recovered.')
add('IncidentRecord','struct','incident',['Version recordVersion;','Incident incident;','std::vector<RepairAttempt> attempts;','std::vector<ExecutionResult> executions;','std::vector<VerificationBatch> verifications;','std::vector<EventId> acceptedEventIds;'])
add('SaveReceipt','struct','incident',['IncidentId incidentId;','Version recordVersion;','UtcTime committedAt;'])
add('ControlEvent','struct','incident',['ControlEventKind kind;','std::optional<RepairPlan> proposedPlan;','std::string reasonCode;'],note='Technical typed control event, never a magic Error. BeginDiagnosis/PlanInvalidated carry no plan; ManualStopProposal carries a proposed ForceStop plan bound to selected owned target. It is not approval or dispatch authority.')
add('IncidentEvent','struct','incident',['EventId id;','IncidentId incidentId;','Version expectedRevision;','UtcTime observedAt;','std::variant<Evidence, DiagnosisResult, Approval, ExecutionResult, VerificationBatch, ControlEvent, Error> payload;'])
add('Transition','struct','incident',['IncidentStatus from;','IncidentStatus to;','Version nextRevision;','std::string reason;'])
add('OperatorView','struct','ui',['std::optional<IncidentRecord> current;','std::vector<HttpObservation> recentActivity;','std::optional<ProcessObservation> process;','bool durable;','bool repairsBlocked;','std::optional<Error> visibleError;'],note='Bounded metadata only; durable=false conspicuous; never show unsaved recovery.')
add('IncidentRepository','class','incident',methods=['virtual ~IncidentRepository() = default;','virtual Result<SaveReceipt> save(const IncidentRecord& record, Version expectedRecordVersion) = 0;','virtual Result<IncidentRecord> load(const IncidentId& id) const = 0;','virtual Result<std::vector<IncidentRecord>> loadPending() const = 0;','virtual Result<DispatchReceipt> claimDispatch(const IncidentId& id, const AttemptId& attempt, Version expectedRecordVersion, UtcTime now) = 0;','virtual Result<std::vector<IncidentRecord>> history(std::size_t offset, std::size_t limit) const = 0;'])
add('SqliteIncidentRepository','class','incident',base='IncidentRepository',methods=['explicit SqliteIncidentRepository(SqliteConfiguration configuration);','~SqliteIncidentRepository() override;','Result<SaveReceipt> save(const IncidentRecord& record, Version expectedRecordVersion) override;','Result<IncidentRecord> load(const IncidentId& id) const override;','Result<std::vector<IncidentRecord>> loadPending() const override;','Result<DispatchReceipt> claimDispatch(const IncidentId& id, const AttemptId& attempt, Version expectedRecordVersion, UtcTime now) override;','Result<std::vector<IncidentRecord>> history(std::size_t offset, std::size_t limit) const override;'],private=['SqliteConfiguration configuration_;','sqlite3* database_;','mutable std::mutex mutex_;'])
add('DatabaseVerifier','class','recovery',methods=['Result<DatabaseCheck> check(const std::string& databasePath, const ProcessObservation& stoppedProcess, Deadline deadline) const;'],note='Verify expected local configured DB and writers stopped; no CREATE; no repair SQL; no sidecar deletion. Other possible writers => Inconclusive.')
add('PlanPolicy','class','recovery',methods=['explicit PlanPolicy(TargetConfiguration configuration);','Status validate(const RepairPlan& plan, const Incident& incident, UtcTime now) const;','Status revalidate(const RepairPlan& plan, const Approval& approval, const Incident& incident, const ProcessObservation& current, UtcTime now) const;'],private=['TargetConfiguration configuration_;'])
add('DiagnosticAgent','class','recovery',methods=['DiagnosticAgent(const PlanPolicy& policy, Now now);','Result<DiagnosisResult> propose(const Incident& incident, const PlanId& newPlanId, Version newPlanRevision, UtcTime expiresAt) const;'],private=['const PlanPolicy& policy_;','Now now_;','DiagnosisResult evaluateCompiledRules(const Incident& incident, const PlanId& planId, Version revision, UtcTime expiresAt) const;'],note='Compiled-in versioned rules only. Unknown cause/ownership cannot become destructive authorization.')
add('RepairExecutor','class','recovery',methods=['RepairExecutor(OwnedDemoProcess& target, IncidentRepository& repository, const PlanPolicy& policy, Now now);','Result<ExecutionResult> executeOnce(const PlanId& planId, const ApprovalId& approvalId, const AttemptId& attemptId, const IncidentId& incidentId, Deadline deadline);'],private=['OwnedDemoProcess& target_;','IncidentRepository& repository_;','const PlanPolicy& policy_;','Now now_;','std::mutex executionMutex_;'],note='Reload authoritative exact record; validate; durable CAS claim; invoke allowlisted action once. No automatic replay after uncertain result or restart.')
add('RecoveryVerifier','class','recovery',methods=['RecoveryVerifier(OwnedDemoProcess& target, const DatabaseVerifier& database, TargetConfiguration configuration, Now now);','Result<VerificationBatch> verify(const RepairPlan& approvedPlan, const RepairAttempt& attempt, const ExecutionResult& execution, const DatabaseCheck& priorStoppedCheck, const std::vector<Evidence>& laterEvidence, Deadline deadline) const;'],private=['OwnedDemoProcess& target_;','const DatabaseVerifier& database_;','TargetConfiguration configuration_;','Now now_;'],note='Stop is not recovery. For start, valid prior stopped DB check + new process + independent later functional evidence required; no requests replayed.')
add('IncidentStateMachine','class','incident',methods=['Result<Transition> apply(const IncidentRecord& current, const IncidentEvent& event) const;'],note='APPROVED state names: Detected, Diagnosing, AwaitingApproval, Executing, Verifying, Recovered, NeedsReview, Declined, Unresolved. No closing an incident with unresolved side effects. Exact transition rules remain under review. Pure function; no persistence, process or network side effects.')
add('IncidentOrchestrator','class','incident',methods=['IncidentOrchestrator(IncidentRepository& repository, DiagnosticAgent& diagnosis, RepairExecutor& executor, RecoveryVerifier& verifier, const IncidentStateMachine& stateMachine, Now now, std::size_t queueCapacity);','Status enqueue(IncidentEvent event);','Result<std::size_t> processPending(std::size_t maximum, Deadline deadline);','Result<OperatorView> view() const;','Status reconcileAfterRestart();'],private=['IncidentRepository& repository_;','DiagnosticAgent& diagnosis_;','RepairExecutor& executor_;','RecoveryVerifier& verifier_;','const IncidentStateMachine& stateMachine_;','Now now_;','std::size_t queueCapacity_;','std::deque<IncidentEvent> events_;','std::optional<IncidentRecord> current_;','bool repairsBlocked_;','std::optional<Error> volatileFailure_;','mutable std::mutex queueMutex_;','std::thread::id ownerThread_;'],note='APPROVED scope: one active incident per configured target, aggregate observations, no concurrent actions, and no old approval reused for a new incident. Single background owner; callbacks enqueue only. No lock across I/O. Store failure blocks actions. Old dispatch remains NeedsReview after controller restart.')
# UI adapters, composition
add('SettingsRepository','class','ui',methods=['virtual ~SettingsRepository() = default;','virtual Result<TargetConfiguration> load() const = 0;','virtual Status save(const TargetConfiguration& configuration) = 0;'])
add('LocalSettingsRepository','class','ui',base='SettingsRepository',methods=['explicit LocalSettingsRepository(std::string path);','Result<TargetConfiguration> load() const override;','Status save(const TargetConfiguration& configuration) override;'],private=['std::string path_;'],note='Local bounded config, atomic replacement; no credentials; no arbitrary commands.')
add('DesktopView','class','ui',methods=['virtual ~DesktopView() = default;','virtual void render(const OperatorView& view) = 0;','virtual void showError(const Error& error) = 0;','virtual void postToUi(std::function<void()> work) = 0;'],note='UI thread boundary, wxWidgets external scheduler; no state transitions in view.')
add('WxDesktopView','class','ui',base='DesktopView',methods=['WxDesktopView();','~WxDesktopView() override;','Status create();','void show();','void render(const OperatorView& view) override;','void showError(const Error& error) override;','void postToUi(std::function<void()> work) override;'],private=['wxFrame* frame_;','wxPanel* applicationPanel_;','wxPanel* activityPanel_;','wxPanel* incidentPanel_;','wxPanel* historyPanel_;'],note='External wx object tree owns widgets. Constructor/destructor and event bindings UI-thread only; widget IDs/layout are not business contracts. Callback binding requires exact reviewed composition.')
add('DesktopPresenter','class','ui',methods=['DesktopPresenter(DesktopView& view, IncidentOrchestrator& core, ControllerRuntime& runtime);','Status saveSettings(const TargetConfiguration& configuration);','Status submitDecision(const IncidentEvent& approvalEvent);','void refresh();'],private=['DesktopView& view_;','IncidentOrchestrator& core_;','ControllerRuntime& runtime_;'],note='No waiting on I/O in GUI event handlers; settings change forbidden while target/incident active. Settings apply next explicit setup session.')
add('ControllerRuntime','class','ui',methods=['ControllerRuntime(Monitor& monitor, IncidentOrchestrator& core, OwnedDemoProcess& process, DesktopView& view, SettingsRepository& settings);','~ControllerRuntime();','Status startMonitoring(const CaptureConfiguration& configuration);','Status stopMonitoring(Deadline deadline);'],private=['Monitor& monitor_;','IncidentOrchestrator& core_;','OwnedDemoProcess& process_;','DesktopView& view_;','std::thread worker_;','std::atomic<bool> stopRequested_;'],note='One background control thread; bounded tick; joins before releasing dependencies. Window close does not silently kill booking. Runtime setup/start target needs explicit UI gate still to finalize.')
# Technical wiring completion proposal; not additional user decisions.
for i in items:
    if i['name']=='OperatorView':
        i['fields'] += ['std::vector<IncidentRecord> historyPage;', 'std::size_t historyOffset;', 'bool historyPending;', 'std::optional<Error> historyError;', 'bool settingsPending;', 'bool settingsSaved;', 'std::optional<Error> settingsError;']
    if i['name']=='IncidentOrchestrator':
        i['methods'] += ['Status requestHistory(std::size_t offset, std::size_t limit);', 'void publishSettingsResult(bool pending, bool saved, std::optional<Error> error);']
        i['private'] += ['std::deque<std::pair<std::size_t, std::size_t>> historyRequests_;', 'Status processHistoryRequest();']
        i['note'] += ' requestHistory validates bounded page size and enqueues under queueMutex_. processPending loads history on owner thread and publishes immutable page under viewMutex_; GUI never calls repository. publishSettingsResult is owner-thread only.'
    if i['name']=='DesktopPresenter':
        i['methods'] += ['Status requestHistory(std::size_t offset, std::size_t limit);']
        i['note'] += ' saveSettings delegates only to runtime.requestSettingsSave; returned Status means queued, not persisted. requestHistory delegates to core queue.'
    if i['name']=='ControllerRuntime':
        i['methods'] += ['Status requestSettingsSave(const TargetConfiguration& configuration);']
        i['private'] += ['SettingsRepository& settings_;', 'std::deque<TargetConfiguration> settingsRequests_;', 'std::mutex settingsMutex_;', 'Status processSettingsRequest();']
    if i['name']=='HttpSessionTracker':
        i['note'] += ' Stable list node addresses back llhttp.data; never move nodes. Free both parsers before erasing a node; callbacks are synchronous on owner thread and cannot survive erase/reset.'

# Pre-generation review corrections: explicit ownership and callback/lifecycle gates.
for i in items:
    if i['name'] in ['DatabaseVerifier','IncidentStateMachine','WindowsConnectionObserver','MacConnectionObserver']:
        i['methods'].insert(0, i['name']+'() = default;')
    if i['name']=='IncidentOrchestrator':
        i['private'] += ['OperatorView publishedView_;', 'mutable std::mutex viewMutex_;']
        i['note'] += ' view() reads only publishedView_ under viewMutex_, never storage/current_ from UI. Publish a value-copy after commit or visible failure.'
    if i['name']=='SqliteConfiguration':
        i['fields'] += ['SqliteOpenMode openMode;']
    if i['name']=='DatabaseCheck':
        i['fields'] += ['ProcessIdentity checkedGeneration;', 'std::string configuredDatabaseKey;']
    if i['name']=='HttpFlowState':
        i['fields'] += ['HttpSessionTracker* owner;']
    if i['name']=='HttpSessionTracker':
        i['private'] += ['llhttp_settings_s* parserSettings_;']
    if i['name']=='Monitor':
        i['private'].remove('void receiveHttp(const HttpObservation& observation);')
        i['methods'] += ['void receiveHttp(const HttpObservation& observation);']
    if i['name']=='IncidentOrchestrator':
        i['methods'] += ['Result<bool> mayStartForSetup() const;', 'Status proposeManualStop(const IncidentId& incidentId, const ProcessIdentity& expected, const PlanId& planId, UtcTime expiresAt);']
        i['note'] += ' Manual stop proposal is not approval. It requires a subsequent exact plan decision; diagnostics cannot create force-stop authority from HTTP-only evidence.'
    if i['name']=='ControllerRuntime':
        i['methods'] += ['Status requestInitialStart();']
        i['private'] += ['std::atomic<bool> initialStartRequested_;']
        i['note'] = 'Settings saves are queued with bounded capacity, processed only by owner thread after stopped/no-active-incident gate, then published through core.publishSettingsResult. No settings I/O in GUI. One background control thread; bounded tick; join before releasing dependencies. requestInitialStart queues only. Owner thread checks core.mayStartForSetup(), no pending durable incidents and current NotStarted/Exited before fixed process start. No start bypass during incident. Window close does not silently kill booking.'
    if i['name']=='TcpStreamDecoder':
        i['private'] = [x.replace('bufferedBytes_', 'acceptedBytes_') for x in i['private']]
        i['note'] += ' Conservative accepted-byte budget is enforced before reassembly; rotation/reset never turns incomplete data into complete evidence.'
    if i['kind']=='class' and i['name'] not in ['BookingRepository','IncidentRepository','OwnedDemoProcess','ConnectionObserver','PacketCapture','SettingsRepository','DesktopView']:
        i['methods'] += [i['name']+'(const '+i['name']+'&) = delete;',i['name']+'& operator=(const '+i['name']+'&) = delete;']
pre='''#pragma once
// P3 DRAFT FOR REVIEW ONLY. Not adopted; not an implementation. Do not replace P2 silently.
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <list>
#include <utility>
#include <optional>
#include <string>
#include <thread>
#include <variant>
#include <vector>
struct sqlite3; struct pcap; struct llhttp__internal_s; struct llhttp_settings_s;
class wxFrame; class wxPanel;
namespace httplib { class Server; }
namespace pcpp { class TcpReassembly; }
namespace rivive::p3_draft {
using Version=std::uint64_t;
using UtcTime=std::chrono::time_point<std::chrono::system_clock,std::chrono::milliseconds>;
using SteadyTime=std::chrono::steady_clock::time_point;
using Deadline=SteadyTime;
using Now=std::function<UtcTime()>;
using RoomId=std::string; using BookingId=std::string; using SessionId=std::string;
using EvidenceId=std::string; using IncidentId=std::string; using PlanId=std::string;
using ApprovalId=std::string; using AttemptId=std::string; using EventId=std::string;
enum class ErrorCode { InvalidArgument, NotFound, Conflict, OverlapRejected, Busy, Unsupported, NotOwner, Stale, Expired, ApprovalRequired, StorageUnavailable, OutcomeUncertain, LimitExceeded, InternalError };
enum class SqliteOpenMode { OpenExisting, InitializeNew };
enum class ProcessState { NotStarted, Running, Exited, Unknown };
enum class Attribution { Unknown, SnapshotCandidate };
enum class HttpMethod { Get, Post, Unsupported };
enum class HttpOutcome { Completed, Incomplete, ObservationLimitExceeded, Unsupported, CaptureUncertain };
enum class Direction { ClientToServer, ServerToClient };
enum class CheckOutcome { Passed, Failed, Inconclusive, NotRun };
enum class EvidenceKind { Process, Http, Database, Failure };
enum class IncidentKind { ProcessExited, HttpErrorResponse, ResponseIncomplete, ObservationLimitExceeded, DatabaseCheckFailed, Unknown };
enum class IncidentStatus { Detected, Diagnosing, AwaitingApproval, Executing, Verifying, Recovered, Declined, Unresolved, NeedsReview };
enum class ActionId { StartStoppedOwnedTarget, ForceStopOwnedTarget };
enum class ControlEventKind { BeginDiagnosis, PlanInvalidated, ManualStopProposal };
enum class ApprovalDecision { Approve, Reject };
enum class AttemptStatus { Prepared, Dispatching, Succeeded, Failed, Uncertain };
enum class ExecutionOutcome { Applied, KnownFailure, Uncertain };
struct Error;
template<class T> using Result=std::variant<T,Error>;
using Status=Result<std::monostate>;
struct PacketView; struct StreamChunk; struct HttpObservation; struct Evidence; class HttpSessionTracker; class ControllerRuntime;
using PacketSink=std::function<void(const PacketView&)>;
using StreamSink=std::function<void(const StreamChunk&)>;
using HttpSink=std::function<void(const HttpObservation&)>;
using EvidenceSink=std::function<void(const Evidence&)>;
'''
# Order values first but dependent classes use complete other structs already emitted in insertion order.
header=pre
for i in items:
 header+=f'\n// {i["note"]}\n{i["kind"]} {i["name"]}' + (f' : public {i["base"]}' if i['base'] else '')+' {\n'
 if i['kind']=='class':header+='public:\n'
 for x in i['fields']+i['methods']:header+='    '+x+'\n'
 if i['private']:
  header+='private:\n'
  for x in i['private']:header+='    '+x+'\n'
 header+='};\n'
header+='\n} // namespace rivive::p3_draft\n'
(root/'contracts/p3_draft.hpp').write_text(header)
(root/'contracts/include_smoke.cpp').write_text('#include "p3_draft.hpp"\nint main() { return 0; }\n')
(root/'docs/ENUMS_AND_ALIASES.md').write_text('# Exact P3 draft enums and aliases\n\nThese declarations are part of the review draft; no new values without review. IncidentStatus values/basic meanings are user-approved; transitions and other enums are not thereby approved. Never close an incident with unresolved side effects.\n\n```cpp\n'+pre.split('namespace rivive::p3_draft {')[1]+'\n```\n')
(root/'catalog.json').write_text(json.dumps(items,indent=2))
md='# P3 exact draft class catalog\n\n**Unapproved proposal. All signatures and fields are draft.**\n\n'
for i in items:
 md+='## '+i['name']+'\n\n'+i['note']+'\n\n```cpp\n'+i['kind']+' '+i['name']+(f' : public {i["base"]}' if i['base'] else '')+' {\n'
 if i['kind']=='class':md+='public:\n'
 md+=''.join('  '+x+'\n' for x in i['fields']+i['methods'])
 if i['private']:md+='private:\n'+''.join('  '+x+'\n' for x in i['private'])
 md+='};\n```\n\n'
(root/'docs/EXACT_CATALOG.md').write_text(md)
# Exact field/method UML, split by group; class overview SVG kept readable.
for group in sorted(set(i['group'] for i in items)):
 puml='@startuml\ntitle P3 DRAFT - '+group+' - NOT APPROVED\nhide empty members\nskinparam classAttributeIconSize 0\n'
 for i in items:
  if i['group']!=group:continue
  puml+=f'class {i["name"]} {{\n'
  for x in i['fields']+i['methods']:puml+='  + '+x.rstrip(';')+'\n'
  for x in i['private']:puml+='  - '+x.rstrip(';')+'\n'
  puml+='}\n'
  if i['base']:puml+=i['base']+' <|-- '+i['name']+'\n'
 puml+='@enduml\n'
 (root/f'diagrams/{group}_exact.puml').write_text(puml)
# Every class/struct gets own exact SVG from same catalog (no hand-transcribed drift).
for i in items:
 rows=[f'<TR><TD BGCOLOR="#dbeafe"><B>{html.escape(i["name"])}</B> (P3 DRAFT)</TD></TR>']
 for label,arr in [('+',i['fields']+i['methods']),('-',i['private'])]:
  for x in arr:rows.append('<TR><TD ALIGN="LEFT">'+html.escape(label+' '+x)+'</TD></TR>')
 dot='digraph G { graph [bgcolor="white"]; node [shape=plain fontname="Arial" fontsize=10]; c [label=<<TABLE BORDER="1" CELLBORDER="0" CELLSPACING="0" CELLPADDING="5">'+''.join(rows)+'</TABLE>>]; }'
 path=root/f'diagrams/{i["name"]}.dot';path.write_text(dot)
 subprocess.run(['dot','-Tsvg',str(path),'-o',str(path.with_suffix('.svg'))],check=True,stderr=subprocess.DEVNULL)
print('generated',len(items),'types')
