# Rivive Shield: Project Backlog

**4 November 2026. Team:** Usama Shmasneh (leader), Samy mansour, Ameen Abd Allader, Shehab Shehab and Yazan Ganem. This backlog defines planned work. Trello is the selected backlog platform; its actual board URL must be added to README.

**Priority:** P0 = needed to agree or prove the central MVP; P1 = required remainder of the agreed MVP or submission; P2 = later improvement. **All listed work items are To do.** Sprint estimates and dates will be assigned during planning.

| ID | Task title | Priority | Planned owner | Depends on | Acceptance / evidence |
| --- | --- | --- | --- | --- | --- |
| B01 | Confirm module boundaries and additional core workload | P0 | Usama Shmasneh + all members | None | Named handoffs and reviewers documented; core and specialist workload estimated separately |
| B02 | Validate problem and compare alternatives | P0 | Samy mansour + Shehab Shehab; all contribute | None | Operator walkthrough recorded and official-source alternatives compared |
| B03 | Agree exact MVP and logical invariant | P0 | Usama Shmasneh + all members | B02 | Three stories, two scenarios, scope boundaries and lecturer decision recorded |
| B04 | Define metric trials and target values | P0 | Ameen Abd Allader + Yazan Ganem; team agrees | B03 | Trial count, success/time targets, evaluation date and trial protocol fixed before evaluation |
| B05 | Establish repository and real backlog links | P0 | Usama Shmasneh | B01 | README points to real repository and board; tasks have title and priority |
| B06 | Agree C++ interfaces and review design | P0 | Usama Shmasneh coordinates; every owner supplies their contract | B03 | Booking, storage, evidence, provider, plan, approval and result interfaces reviewed |
| B07 | Implement booking domain and synthetic fixtures | P0 | Samy mansour | B06 | T-01 passes; in-memory adapter supports local unit tests; no SQL required |
| B08 | Implement fixed-target demo control adapter | P0 | Usama Shmasneh | B06, B07 | Process configuration is restricted to the isolated demo; Yazan consumes the C++ interface |
| B09 | Implement monitoring and incident evidence | P0 | Ameen Abd Allader | B06; real-target check after B08 | T-02 passes; independent observations and false-positive tests included |
| B10 | Implement SQLite repositories | P0 | Usama Shmasneh | B06 | Store/reload booking, evidence, decisions and results behind C++ interfaces |
| B11 | Implement approved restart and execute-once logic | P0 | Yazan Ganem | B06; real adapter after B08 | Approval/precondition/duplicate checks; T-04–T-06 pass for stop scenario |
| B12 | Integrate first real local recovery slice | P0 | Usama Shmasneh owns the core; each owner fixes their module | B07–B11, B25 | Stop, detect, approve, restore, verify and persist work together |
| B13 | Verify AI access and connect provider adapter | P0 | Usama Shmasneh | B06 | Free-access feasibility, actual model/version and limits recorded; secrets kept out of repo |
| B14 | Implement bounded C++ diagnostic module and plan policy | P0 | Shehab Shehab | B06, B09; real provider after B13 | Evidence-linked diagnostic result to the core; invalid responses tested; no second lifecycle controller |
| B15 | Implement controlled logical scenario and guard | P1 | Samy mansour | B03, B07 | T-03 proves agreed invariant violation; guard prevents the rule violation, not one input string |
| B16 | Implement logical repair and independent verification | P1 | Yazan Ganem; Ameen reviews invariant evidence | B15 | Original/variant/normal-use retests pass without losing unaffected data |
| B17 | Implement local operator CLI and view-model | P0 | Shehab Shehab | B06, B14, B25 | CLI calls the core for approval/state changes and displays its authoritative state; no duplicate workflow logic |
| B18 | Connect minimal browser/API interface | P1 | Usama Shmasneh; Shehab supplies C++ view-model | B12, B14, B17 | Thin browser view uses the same validated C++ flow; no duplicated domain logic |
| B19 | Run acceptance and metric trials | P1 | Yazan Ganem coordinates; each owner runs their cases | B04, B14–B18, B26 | T-01–T-12 actual outcomes/failures recorded; Usama runs core tests; Ameen supplies timing evidence |
| B20 | Configure GitHub Actions and validate Oracle hosting | P1 | Usama Shmasneh | B12; final demo after B19 | Reproducible build/tests; deployment uses reviewed branch; no false claim of successful hosting |
| B21 | Review first-submission files and UML | P0 | Each member reviews their module; Shehab assembles changes | B01–B06 | Three-section report, required images, names and links reviewed; submission TODOs resolved |
| B22 | Record sprint and pull-request evidence | P1 | Every member for their own changes | B05 onward | Factual goal, work, review and corrections; named review rotation followed |
| B23 | Prepare final report and team demonstration | P1 | All five members prepare; Usama is planned presenter | B19, B22 | Real central story; results and run guide; randomly selected testers; presenter arrangement checked against course requirements |
| B24 | Select distinct personal research topics | P1 | Each member | Course topic list | Five different choices; individual research/reflection stay private in Moodle |
| B25 | Implement core incident orchestrator and state machine | P0 | Usama Shmasneh | B06; module contracts first, real integration after B07–B14 | Serialized events; guarded transitions; approval/revision binding; bounded dispatch and independent verification |
| B26 | Test concurrency, interruption and state recovery | P0 | Usama Shmasneh; Samy reviews, Yazan checks repair handoff | B25; mocked failures first, real adapters before final test | T-10–T-12 cover racing events, timeouts, failed persistence and uncertain execution; no blind replay |

## Current project decisions

Project approval has been reported by Usama Shmasneh; the date and any conditions are not supplied. Planned assistance tools are ChatGPT for Usama and a mix of ChatGPT, Gemini and Claude for other members. Record actual tool/model use and human review as work occurs. This update does not mark implementation tasks complete.

## Additional core ownership and handoffs

Usama's C++ incident core is additional to leadership and specialist work. Core and specialist estimates will be tracked separately when planning sprint capacity.

- **Usama Shmasneh:** `IncidentOrchestrator` and `IncidentStateMachine`, serialized event handling, approval/plan revision binding, safe dispatch, timeouts and crash reconciliation, core tests T-10–T-12; plus leadership, SQLite/SQL, external AI wiring, Oracle, GitHub Actions and integration/web glue.
- **Samy mansour:** C++ booking domain, controlled logical scenario, guard, domain tests and booking documentation. No incident lifecycle controller.
- **Ameen Abd Allader:** C++ monitoring, evidence, independent invariant detection, timing/false-positive tests and monitoring documentation. Sends observations to the core; no repair authority.
- **Shehab Shehab:** C++ diagnostic tool contracts, evidence-to-plan reasoning support, plan-content validation, response-error tests and CLI/view-model. Returns diagnostic results and sends UI requests to the core; no lifecycle scheduler or global incident state machine.
- **Yazan Ganem:** C++ bounded repair commands, command preconditions, independent verification, failure tests and reusable test runner. Invoked by the core; no unapproved autonomous retries.

SQLite/SQL, external AI-service setup, web/API glue, Oracle and GitHub Actions remain with Usama. Other members use typed C++ interfaces and labeled fake adapters for unit tests. The final demonstration must use real behavior and the actual AI connection.

Review rotation remains Samy reviews Usama; Ameen reviews Samy; Shehab reviews Ameen; Yazan reviews Shehab; Usama reviews Yazan. Yazan additionally checks the core-to-repair contract. For specialist changes, peers verify documented outcomes and flag unfamiliar configuration; they do not claim expert cloud/security review.

Each owner is responsible for their module's tests, documentation and integration defects. Local core integration precedes hosting and interface polish; capacity issues and ownership changes will be addressed explicitly during sprint planning.

## Suggested first sprint

Agree B01–B06, then start the booking, monitoring, diagnosis-policy and repair modules in parallel using shared C++ contracts and test doubles. Usama starts B25 and B26 with the agreed contracts, then supplies storage/fixed-target adapters and integrates the smallest real local recovery slice. Core work will be scheduled alongside the specialist responsibilities. Sprint dates and capacity will be agreed during planning. AI-backed diagnosis and the logical scenario remain part of the proposed finished MVP even if the first integration slice is smaller.

## Definition of Done proposal

- The module's acceptance criteria have reproducible evidence.
- Normal and relevant failure cases are tested by its owner.
- A named teammate reviews the requirements, behavior and changes and records findings/corrections.
- The owner updates interface documentation and their report section and demonstration material.
- The module integrates without breaking existing checks; its owner resolves its defects.
- AI-generated output is checked by the responsible team member.

## Future items, not commitments

More supported faults, additional target applications and concurrent isolated sessions require a later scope decision. Device tracking is excluded and is not assigned.
