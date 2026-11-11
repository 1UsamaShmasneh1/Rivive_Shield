# Exact P1-to-P2 declaration change index
Review-only proposal; no Git/Trello mutation.
## Removed types
- AiAudit
- AiConfiguration
- AiProvider
- AiRequest
- RuntimeAiProvider
- ToolRequest
## Added types
- MacOwnedDemoProcess
- OwnedDemoProcess
- RuleAudit
- WindowsOwnedDemoProcess
## Modified types
### DiagnosisResult
```diff
--- 
+++ 
@@ -3,6 +3,6 @@
     std::string explanation;
     std::optional<RepairPlan> plan;
     std::vector<Evidence> collectedEvidence;
-    AiAudit aiAudit;
+    RuleAudit ruleAudit;
     std::optional<Error> failure;
 };
```
### DiagnosticAgent
```diff
--- 
+++ 
@@ -1,11 +1,12 @@
 class DiagnosticAgent {
 public:
-    DiagnosticAgent(AiProvider& provider, const DemoTargetAdapter& target, const Monitor& monitor, const PlanPolicy& policy, DiagnosticLimits limits);
+    DiagnosticAgent(const DemoTargetAdapter& target, const Monitor& monitor, const PlanPolicy& policy, DiagnosticLimits limits);
     Result<DiagnosisResult> propose(const Incident& incident, const DiagnosisContext& context);
 private:
-    AiProvider& provider_;
     const DemoTargetAdapter& target_;
     const Monitor& monitor_;
     const PlanPolicy& policy_;
     const DiagnosticLimits limits_;
+    DiagnosisDraft evaluateRules(const std::vector<Evidence>& evidence) const;
+    std::string rulesetVersion() const;
 };
```
### DiagnosticLimits
```diff
--- 
+++ 
@@ -1,5 +1,5 @@
 struct DiagnosticLimits {
-std::size_t maxProviderTurns;
+std::size_t maxObservationReads;
     std::size_t maxEvidenceItems;
     std::size_t maxTextBytes;
     std::chrono::milliseconds overallTimeout;
```
### FixedDemoTargetAdapter
```diff
--- 
+++ 
@@ -1,6 +1,6 @@
 class FixedDemoTargetAdapter : public DemoTargetAdapter {
 public:
-    FixedDemoTargetAdapter(const DemoConfiguration& configuration, BookingRepository& bookingRepository, IncidentRepository& incidentRepository, Now now);
+    FixedDemoTargetAdapter(const DemoConfiguration& configuration, OwnedDemoProcess& process, BookingRepository& bookingRepository, IncidentRepository& incidentRepository, Now now);
     ~FixedDemoTargetAdapter() override;
     FixedDemoTargetAdapter(const FixedDemoTargetAdapter&) = delete;
     FixedDemoTargetAdapter& operator=(const FixedDemoTargetAdapter&) = delete;
@@ -16,8 +16,7 @@
     BookingRepository& bookingRepository_;
     IncidentRepository& incidentRepository_;
     Now now_;
-    std::int64_t ownedProcessId_;
-    std::optional<std::uint64_t> ownedProcessStartTicks_;
+    OwnedDemoProcess& process_;
     ProcessGeneration generation_;
     std::vector<AttemptId> consumedDispatches_;
     mutable std::mutex mutex_;
```
### IncidentRecord
```diff
--- 
+++ 
@@ -5,6 +5,6 @@
     std::vector<ExecutionResult> executions;
     std::vector<VerificationResult> verificationResults;
     std::vector<EventId> acceptedEventIds;
-    std::vector<AiAudit> aiCalls;
+    std::vector<RuleAudit> ruleEvaluations;
     std::vector<Error> failures;
 };
```
## Enum / alias changes
- Remove DiagnosticToolId and AiResponse (unused provider tool protocol).
- Remove ErrorCode::ProviderUnavailable and ErrorCode::ProviderProtocolError.
- Add ProcessStatus: Running, Stopped, Unknown. Running is OS liveness only.
- No external RuleCatalog or rule-file parser is introduced.
## Behavior / documentation changes
- Replace actual AI requirement with compiled-in evidence rules, rule audit and explicit no-match behavior.
- Replace Linux start-ticks assumption with reviewed native Windows handle/macOS exclusive child ownership contracts.
- Remove unapproved illustrative numeric defaults for diagnostic turns/data/deadline, event-queue capacity and synthetic-label size. Their concrete values remain open; no new limits were invented.
- Preserve original domain/data/lifecycle rules. Values such as one isolated target, one active incident and one exact conflict pair are inherited P1 scope restrictions, not newly invented workload limits.
- Remove stale P1 PDFs/scripts and evidence from current deliverable; regenerate current diagrams/catalogue/verification. Historical changes remain available as a text diff, not an old approved design.