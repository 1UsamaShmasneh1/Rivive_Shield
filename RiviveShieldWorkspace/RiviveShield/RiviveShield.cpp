#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "RiviveShield.h"
#include <windows.h>
#include <wx/wx.h>
#include <llhttp.h>
#include <cstring>
#include <IpAddress.h>
#include <chrono>
#include <mutex>
#include <string>
#include <utility>
#include <variant>

class TestApp : public wxApp
{
public:
    bool OnInit() override
    {
        llhttp_settings_t settings;
        llhttp_settings_init(&settings);

        llhttp_t parser;
        llhttp_init(&parser, HTTP_RESPONSE, &settings);

        const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        const llhttp_errno_t result = llhttp_execute(&parser, response, std::strlen(response));

        const bool ok = result == HPE_OK && llhttp_get_status_code(&parser) == 200;
        wxMessageBox(ok ? "llhttp test passed: HTTP 200" : "llhttp test failed", "Library test");

        pcpp::IPv4Address address("127.0.0.1");
        wxMessageBox(wxString::FromUTF8(address.toString().c_str()), "PcapPlusPlus test");

        auto* window = new wxFrame(nullptr, wxID_ANY, "Rivive Shield - wxWidgets Test");
        window->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(TestApp);

namespace rivive::p3_draft {

    WindowsOwnedDemoProcess::WindowsOwnedDemoProcess(TargetConfiguration configuration, SessionId session)
        : configuration_(std::move(configuration)),
        session_(std::move(session)),
        generation_(0),
        identity_(std::nullopt),
        ownershipUncertain_(false),
        stopRequested_(false),
        processHandle_(nullptr),
        threadHandle_(nullptr) {
    }

    WindowsOwnedDemoProcess::~WindowsOwnedDemoProcess() {
        if (threadHandle_ != nullptr) {
            ::CloseHandle(static_cast<HANDLE>(threadHandle_));
            threadHandle_ = nullptr;
        }
        if (processHandle_ != nullptr) {
            ::CloseHandle(static_cast<HANDLE>(processHandle_));
            processHandle_ = nullptr;
        }
    }

    Result<ProcessIdentity> WindowsOwnedDemoProcess::start(Deadline deadline) {
        if (std::chrono::steady_clock::now() >= deadline) {
            return Error{ ErrorCode::Busy, "Start deadline already expired" };
        }
        if (ownershipUncertain_) {
            return Error{ ErrorCode::OutcomeUncertain, "Process ownership is uncertain" };
        }
        if (configuration_.executablePath.empty() || session_.empty()) {
            return Error{ ErrorCode::InvalidArgument, "Target executable path or session is empty" };
        }

        if (processHandle_ != nullptr) {
            DWORD exitCode = 0;
            if (::GetExitCodeProcess(static_cast<HANDLE>(processHandle_), &exitCode) != 0 && exitCode == STILL_ACTIVE) {
                return Error{ ErrorCode::Conflict, "Owned target process is already running" };
            }
            if (threadHandle_ != nullptr) {
                ::CloseHandle(static_cast<HANDLE>(threadHandle_));
                threadHandle_ = nullptr;
            }
            ::CloseHandle(static_cast<HANDLE>(processHandle_));
            processHandle_ = nullptr;
        }

        std::string commandLine = "\"" + configuration_.executablePath + "\"";
        for (const auto& arg : configuration_.fixedArguments) {
            commandLine += " \"" + arg + "\"";
        }
        std::vector<char> mutableCmd(commandLine.begin(), commandLine.end());
        mutableCmd.push_back('\0');

        STARTUPINFOA startupInfo{};
        startupInfo.cb = sizeof(STARTUPINFOA);
        PROCESS_INFORMATION procInfo{};

        const char* workDir = configuration_.workingDirectory.empty()
            ? nullptr
            : configuration_.workingDirectory.c_str();

        const BOOL created = ::CreateProcessA(
            configuration_.executablePath.c_str(),
            mutableCmd.data(),
            nullptr,
            nullptr,
            FALSE,
            CREATE_NO_WINDOW,
            nullptr,
            workDir,
            &startupInfo,
            &procInfo);

        if (created == 0) {
            return Error{ ErrorCode::InternalError, "Failed to start owned target process" };
        }

        if (std::chrono::steady_clock::now() > deadline) {
            ::CloseHandle(procInfo.hThread);
            ::CloseHandle(procInfo.hProcess);
            ownershipUncertain_ = true;
            return Error{ ErrorCode::OutcomeUncertain, "Start exceeded deadline after process creation" };
        }

        processHandle_ = procInfo.hProcess;
        threadHandle_ = procInfo.hThread;
        stopRequested_ = false;
        ++generation_;

        const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();

        ProcessIdentity createdIdentity{
            session_,
            generation_,
            static_cast<std::uint64_t>(procInfo.dwProcessId),
            session_ + ":" + std::to_string(generation_) + ":" + std::to_string(nowMs)
        };
        identity_ = createdIdentity;
        return createdIdentity;
    }

    Result<ProcessObservation> WindowsOwnedDemoProcess::observe() {
        const UtcTime observedAt = std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now());

        if (!identity_.has_value() || processHandle_ == nullptr) {
            ProcessIdentity emptyId{ session_, generation_, 0, "" };
            return ProcessObservation{
                emptyId,
                ownershipUncertain_ ? ProcessState::Unknown : ProcessState::NotStarted,
                observedAt,
                std::nullopt,
                stopRequested_
            };
        }

        if (ownershipUncertain_) {
            return ProcessObservation{
                *identity_,
                ProcessState::Unknown,
                observedAt,
                std::nullopt,
                stopRequested_
            };
        }

        DWORD exitCode = 0;
        if (::GetExitCodeProcess(static_cast<HANDLE>(processHandle_), &exitCode) == 0) {
            return Error{ ErrorCode::InternalError, "Failed to query owned process state" };
        }

        if (exitCode == STILL_ACTIVE) {
            return ProcessObservation{
                *identity_,
                ProcessState::Running,
                observedAt,
                std::nullopt,
                stopRequested_
            };
        }

        return ProcessObservation{
            *identity_,
            ProcessState::Exited,
            observedAt,
            static_cast<std::uint32_t>(exitCode),
            stopRequested_
        };
    }

    Status WindowsOwnedDemoProcess::requestGracefulStop(const ProcessIdentity& expected, Deadline deadline) {
        (void)deadline;
        if (ownershipUncertain_) {
            return Error{ ErrorCode::OutcomeUncertain, "Process ownership is uncertain" };
        }
        if (!identity_.has_value() || processHandle_ == nullptr ||
            identity_->controllerSessionId != expected.controllerSessionId ||
            identity_->generation != expected.generation ||
            identity_->processId != expected.processId ||
            identity_->creationToken != expected.creationToken) {
            return Error{ ErrorCode::NotOwner, "Expected process identity does not match owned target" };
        }
        return Error{ ErrorCode::Unsupported, "Graceful stop is unsupported for Windows owned target" };
    }

    Status WindowsOwnedDemoProcess::forceStop(const ProcessIdentity& expected, Deadline deadline) {
        if (ownershipUncertain_) {
            return Error{ ErrorCode::OutcomeUncertain, "Process ownership is uncertain" };
        }
        if (!identity_.has_value() || processHandle_ == nullptr ||
            identity_->controllerSessionId != expected.controllerSessionId ||
            identity_->generation != expected.generation ||
            identity_->processId != expected.processId ||
            identity_->creationToken != expected.creationToken) {
            return Error{ ErrorCode::NotOwner, "Expected process identity does not match owned target" };
        }

        DWORD exitCode = 0;
        if (::GetExitCodeProcess(static_cast<HANDLE>(processHandle_), &exitCode) != 0 && exitCode != STILL_ACTIVE) {
            return std::monostate{};
        }

        const auto nowSteady = std::chrono::steady_clock::now();
        if (nowSteady >= deadline) {
            return Error{ ErrorCode::Busy, "Force stop deadline already expired" };
        }

        stopRequested_ = true;
        if (::TerminateProcess(static_cast<HANDLE>(processHandle_), 1) == 0) {
            DWORD postExit = 0;
            if (::GetExitCodeProcess(static_cast<HANDLE>(processHandle_), &postExit) != 0 && postExit != STILL_ACTIVE) {
                return std::monostate{};
            }
            ownershipUncertain_ = true;
            return Error{ ErrorCode::OutcomeUncertain, "Failed to terminate owned target process" };
        }

        const auto remainingMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now()).count();
        const DWORD waitMs = remainingMs > 0 ? static_cast<DWORD>(remainingMs) : 0;

        const DWORD waitRes = ::WaitForSingleObject(static_cast<HANDLE>(processHandle_), waitMs);
        if (waitRes == WAIT_OBJECT_0) {
            return std::monostate{};
        }

        ownershipUncertain_ = true;
        return Error{ ErrorCode::OutcomeUncertain, "Force stop wait timed out before process exit confirmation" };
    }

    bool Approval::matches(const RepairPlan& plan) const {
        return planId == plan.id &&
            planRevision == plan.revision &&
            incidentId == plan.incidentId;
    }

    Status PlanPolicy::validate(const RepairPlan& plan, const Incident& incident, UtcTime now) const {
        if (plan.id.empty() || plan.incidentId.empty()) {
            return Error{ ErrorCode::InvalidArgument, "Plan or incident identifier is empty" };
        }
        if (plan.incidentId != incident.id) {
            return Error{ ErrorCode::Conflict, "Plan incident does not match target incident" };
        }
        if (plan.revision == 0) {
            return Error{ ErrorCode::InvalidArgument, "Invalid plan revision" };
        }
        if (plan.expiresAt <= now) {
            return Error{ ErrorCode::Expired, "Repair plan has expired" };
        }
        if (plan.expectedProcess.processId == 0 || plan.expectedProcess.controllerSessionId.empty()) {
            return Error{ ErrorCode::InvalidArgument, "Expected process identity is incomplete" };
        }
        if (configuration_.targetLabel.empty() || configuration_.executablePath.empty()) {
            return Error{ ErrorCode::InvalidArgument, "Target configuration is incomplete" };
        }
        if (plan.action != ActionId::StartStoppedOwnedTarget &&
            plan.action != ActionId::ForceStopOwnedTarget) {
            return Error{ ErrorCode::Unsupported, "Action is outside allowed recovery actions" };
        }
        return std::monostate{};
    }

    Status PlanPolicy::revalidate(const RepairPlan& plan,
        const Approval& approval,
        const Incident& incident,
        const ProcessObservation& obs,
        UtcTime now) const {
        Status baseStatus = validate(plan, incident, now);
        if (std::holds_alternative<Error>(baseStatus)) {
            return baseStatus;
        }
        if (!approval.matches(plan) ||
            approval.incidentId != incident.id ||
            approval.decision != ApprovalDecision::Approve) {
            return Error{ ErrorCode::ApprovalRequired, "Approval does not authorize this plan and incident" };
        }
        if (obs.identity.controllerSessionId != plan.expectedProcess.controllerSessionId) {
            return Error{ ErrorCode::NotOwner, "Observed process does not belong to controller session" };
        }
        if (plan.action == ActionId::StartStoppedOwnedTarget && obs.state == ProcessState::Running) {
            return Error{ ErrorCode::Conflict, "Target process is already running" };
        }
        if (plan.action == ActionId::ForceStopOwnedTarget && obs.state == ProcessState::NotStarted) {
            return Error{ ErrorCode::Conflict, "Target process is already not started" };
        }
        return std::monostate{};
    }

    RepairExecutor::RepairExecutor(OwnedDemoProcess& target,
        IncidentRepository& repository,
        const PlanPolicy& policy,
        Now now)
        : target_(target),
        repository_(repository),
        policy_(policy),
        now_(std::move(now)) {
    }

    Result<ExecutionResult> RepairExecutor::executeOnce(const PlanId& planId,
        const ApprovalId& approvalId,
        const AttemptId& attemptId,
        const IncidentId& incidentId,
        Deadline deadline) {
        std::lock_guard<std::mutex> lock(executionMutex_);

        const UtcTime currentTime = now_();

        Result<IncidentRecord> loaded = repository_.load(incidentId);
        if (std::holds_alternative<Error>(loaded)) {
            return std::get<Error>(loaded);
        }

        const IncidentRecord& record = std::get<IncidentRecord>(loaded);

        const RepairPlan* matchedPlan = nullptr;
        for (const auto& plan : record.incident.plans) {
            if (plan.id == planId && plan.incidentId == incidentId) {
                matchedPlan = &plan;
                break;
            }
        }
        if (!matchedPlan) {
            return Error{ ErrorCode::NotFound, "Matching repair plan not found" };
        }

        const Approval* matchedApproval = nullptr;
        for (const auto& approval : record.incident.approvals) {
            if (approval.id == approvalId &&
                approval.planId == matchedPlan->id &&
                approval.planRevision == matchedPlan->revision &&
                approval.incidentId == incidentId &&
                approval.decision == ApprovalDecision::Approve &&
                approval.matches(*matchedPlan)) {
                matchedApproval = &approval;
                break;
            }
        }
        if (!matchedApproval) {
            return Error{ ErrorCode::ApprovalRequired, "Valid approval not found for plan" };
        }

        const RepairAttempt* matchedAttempt = nullptr;
        for (const auto& attempt : record.attempts) {
            if (attempt.id == attemptId &&
                attempt.incidentId == incidentId &&
                attempt.planId == matchedPlan->id &&
                attempt.planRevision == matchedPlan->revision &&
                attempt.approvalId == matchedApproval->id) {
                matchedAttempt = &attempt;
                break;
            }
        }
        if (!matchedAttempt) {
            return Error{ ErrorCode::NotFound, "Matching repair attempt not found" };
        }
        if (matchedAttempt->status != AttemptStatus::Prepared || matchedAttempt->dispatchClaimedAt.has_value()) {
            return Error{ ErrorCode::Conflict, "Repair attempt is not eligible for initial dispatch" };
        }
        for (const auto& term : record.preparedTerminations) {
            if (term.attemptId == attemptId) {
                return Error{ ErrorCode::Conflict, "Prepared attempt was already terminated" };
            }
        }
        for (const auto& prevExec : record.executions) {
            if (prevExec.attemptId == attemptId) {
                return Error{ ErrorCode::Conflict, "Execution already recorded for this attempt" };
            }
        }

        Status planValid = policy_.validate(*matchedPlan, record.incident, currentTime);
        if (std::holds_alternative<Error>(planValid)) {
            return std::get<Error>(planValid);
        }

        Result<ProcessObservation> currentObsRes = target_.observe();
        if (std::holds_alternative<Error>(currentObsRes)) {
            return std::get<Error>(currentObsRes);
        }

        const ProcessObservation& currentObs = std::get<ProcessObservation>(currentObsRes);
        Status reval = policy_.revalidate(*matchedPlan, *matchedApproval, record.incident, currentObs, currentTime);
        if (std::holds_alternative<Error>(reval)) {
            return std::get<Error>(reval);
        }

        Result<DispatchReceipt> claimed = repository_.claimDispatch(incidentId, attemptId, record.recordVersion, currentTime);
        if (std::holds_alternative<Error>(claimed)) {
            return std::get<Error>(claimed);
        }

        const DispatchReceipt& receipt = std::get<DispatchReceipt>(claimed);

        ExecutionResult execResult{
            attemptId,
            ExecutionOutcome::Uncertain,
            std::nullopt,
            std::nullopt,
            currentTime
        };

        if (matchedPlan->action == ActionId::StartStoppedOwnedTarget) {
            Result<ProcessIdentity> startRes = target_.start(deadline);
            execResult.finishedAt = now_();
            if (std::holds_alternative<Error>(startRes)) {
                const Error& err = std::get<Error>(startRes);
                execResult.failure = err;
                execResult.outcome = (err.code == ErrorCode::OutcomeUncertain)
                    ? ExecutionOutcome::Uncertain
                    : ExecutionOutcome::KnownFailure;
            }
            else {
                execResult.outcome = ExecutionOutcome::Applied;
                Result<ProcessObservation> afterObs = target_.observe();
                if (!std::holds_alternative<Error>(afterObs)) {
                    execResult.resultingProcess = std::get<ProcessObservation>(afterObs);
                }
            }
        }
        else if (matchedPlan->action == ActionId::ForceStopOwnedTarget) {
            Status stopStatus = target_.forceStop(matchedPlan->expectedProcess, deadline);
            execResult.finishedAt = now_();
            if (std::holds_alternative<Error>(stopStatus)) {
                const Error& err = std::get<Error>(stopStatus);
                execResult.failure = err;
                execResult.outcome = (err.code == ErrorCode::OutcomeUncertain)
                    ? ExecutionOutcome::Uncertain
                    : ExecutionOutcome::KnownFailure;
            }
            else {
                execResult.outcome = ExecutionOutcome::Applied;
                Result<ProcessObservation> afterObs = target_.observe();
                if (!std::holds_alternative<Error>(afterObs)) {
                    execResult.resultingProcess = std::get<ProcessObservation>(afterObs);
                }
            }
        }
        else {
            execResult.finishedAt = now_();
            execResult.outcome = ExecutionOutcome::KnownFailure;
            execResult.failure = Error{ ErrorCode::Unsupported, "Unsupported repair action" };
        }

        IncidentRecord updated = record;
        updated.recordVersion = receipt.recordVersion;
        updated.executions.push_back(execResult);

        Result<SaveReceipt> saved = repository_.save(updated, receipt.recordVersion);
        if (std::holds_alternative<Error>(saved)) {
            return std::get<Error>(saved);
        }

        return execResult;
    }

} // namespace rivive::p3_draft