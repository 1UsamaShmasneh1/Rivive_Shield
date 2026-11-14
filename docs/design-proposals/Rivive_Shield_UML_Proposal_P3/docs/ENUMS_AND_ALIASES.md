# Exact P3 draft enums and aliases

These declarations are part of the review draft; no new values without review. IncidentStatus values/basic meanings are user-approved; transitions and other enums are not thereby approved. Never close an incident with unresolved side effects.

```cpp

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

```
