# Rivive Shield

**Course:** Software Development Methods  
**Team leader:** Usama Shmasneh  
**Team members:** Samy mansour, Ameen Abd Allader, Shehab Shehab, Yazan Ganem  
**Submission:** Proposal, requirements and system design, 4 November 2026  
**Lecturer decision:** Project approved, as reported by Usama Shmasneh. Usama also reports lecturer approval of the initial prototype using predefined diagnostic rules and repair solutions without an external or locally running AI model. The original approval date and any additional conditions have not been provided.

## Project overview

Rivive Shield is a proposed learning and resilience-testing system for one isolated study-room booking application. It connects fault detection, evidence-based diagnosis using predefined rules, an explicitly approved repair and independent verification. Its intended users are the demonstration operator and course participants working with fictitious data.

The current planning direction is a desktop controller that observes an independently functioning study-room booking application, investigates supported failures using predefined rules, requests operator approval for a bounded recovery action, and verifies the result. The booking application must work normally without special diagnostic callbacks or dependencies on Rivive Shield. Start with local development on the team's Windows and macOS computers; macOS verification remains pending.

The first-submission report and P2 package record earlier designs, including the controlled overlapping-booking repair scenario. They must not be read as exact contracts for the new passive-monitoring/desktop direction. Ordinary booking validation must still prevent overlapping active reservations, but permission to rewrite booking records is not implied by the new recovery scope. The detailed scenario set and acceptance criteria must be synchronized in RS-05, RS-06, RS-07 and their dependent cards before implementation.

The initial prototype uses predefined diagnostic rules and repair solutions. It does not use an external AI provider or a locally running AI model. Rules match collected evidence against supported conditions and propose an allowlisted repair with its evidence references and preconditions. The hidden challenge selector must not be used as the diagnosis. Unknown or insufficient evidence leaves the incident unresolved.

Predetermined repair rules do not predetermine the outcome. The application must exhibit the actual fault, an operator must explicitly approve the proposed repair, and independent checks must establish recovery, preservation of unaffected data and successful persistence before the system reports success. Further development will be considered after the prototype has been tested and shown to work; no specific later AI integration, provider or schedule has been selected.

## Project links

| Item | Location |
| --- | --- |
| Shared GitHub repository | https://github.com/1UsamaShmasneh1/Rivive_Shield |
| Trello backlog board | https://trello.com/b/GeLevegk/riviveshield |

The [backlog transition index](docs/backlog-seed.md) links the existing RS cards and records the approved redistribution, including [RS-31 React](https://trello.com/c/fk0HHQkw). Trello card content, assignments and attachments have been applied and independently verified; that does not establish implementation completion. Use the live board for current progress and acceptance evidence. The original B01–B26 planning backlog is preserved as a [historical snapshot](docs/history/backlog-seed-2026-11-04.md), not a second active work plan.

## Design status: P3-R2 adopted working baseline

**Owner adoption: 2026-11-13. P3-R2 is the approved working design baseline.** See the authoritative [P3 adoption status](docs/design-proposals/P3_STATUS.md). The frozen source package retains its original review labels and checksums; those labels describe its preparation, not the current adoption status.

P2 remains historical. P3-R2 introduces the desktop interface, independent booking web application, passive monitoring and bounded recovery. Do not extend contracts silently: any structural change must update the exact source, UML and affected card attachments together.

Owner adoption is not completed peer review, implementation, native-platform acceptance or end-to-end recovery verification. RS-06 and RS-07 still require their appropriate independent review and evidence. Open implementation settings and explicit technical assumptions in the adopted package must be resolved through change control; do not turn proposed numerical limits or unverified platform capabilities into tested facts.

For the retained earlier design, see [P2 status and design navigation](docs/design-proposals/README.md). The complete P2 source package is preserved byte-for-byte in its own directory, including its original preparation labels and checksums. Read the accompanying adoption status before interpreting the package's earlier “PROPOSED” wording.

- [Exact C++ declarations](docs/design-proposals/Rivive_Shield_UML_Proposal_P2/contracts/rivive_shield_proposed.hpp)
- [Full member/signature catalog](docs/design-proposals/Rivive_Shield_UML_Proposal_P2/contracts/signature_catalog.md)
- [Arabic design review memorandum](docs/design-proposals/Rivive_Shield_UML_Proposal_P2/docs/review_ar.md)
- [Design change log](docs/design-proposals/Rivive_Shield_UML_Proposal_P2/docs/change_log.md)
- [Actual verification scope](docs/design-proposals/Rivive_Shield_UML_Proposal_P2/checks/verification.txt)

## Adopted P3-R2 source package

Read the [P3 entry guide](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/README_AR.md) and [readiness limits](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/IMPLEMENTATION_READINESS_AR.md) first.

- [Frozen revision](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/REVISION.md)
- [Approved assignments](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/APPROVED_ASSIGNMENTS_AR.md)
- [Proposed C++17 declarations](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp)
- [Exact catalog of 67 struct/class declarations](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/docs/EXACT_CATALOG.md)
- [Enums and aliases](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/docs/ENUMS_AND_ALIASES.md)
- [Behavior and execution gates](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/docs/BEHAVIOR_AND_GATES.md)
- [P2 changes](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/docs/P2_DELTA.md)
- [Artifact validation scope](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/checks/VALIDATION.md)
- [Snapshot checksums](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/SHA256SUMS)

The 67 types are shared data and service declarations, not 67 separate tasks or implemented modules. Declaration/catalog/diagram consistency and syntax checks do not establish runtime correctness. State transitions, schemas and technical assumptions inside the snapshot are governed by the adopted snapshot and its explicitly open assumptions; adoption does not prove implementation correctness. Keep the adopted snapshot bytes intact; subsequent structural revisions require a reviewed change and updated checksums.

## Historical first-submission documents

| Document | File |
| --- | --- |
| One-page proposal, PDF | [Proposal PDF](docs/01-proposal.pdf) |
| Editable proposal | [Proposal Markdown](docs/01-proposal.md) |
| Three-section first-submission report, PDF | [Report PDF](docs/first-submission-report.pdf) |
| Editable first-submission report | [Report Markdown](docs/first-submission-report.md) |
| Planned work | [Project backlog](docs/backlog-seed.md) |
| Remaining submission details | [Submission checklist](docs/submission-checklist.md) |
| Main-screen sketch | [Operator dashboard](docs/ui-sketch.png) |
| UML use case diagram | [Use case diagram](docs/diagrams/use-case.png) |
| UML class diagram | [Class diagram](docs/diagrams/class.png) |
| UML sequence diagram | [Sequence diagram](docs/diagrams/sequence.png) |
| Supplementary incident-state diagram | [State diagram](docs/diagrams/incident-state.png) |

Report section 1 defines the proposal and MVP; section 2 contains the stories, acceptance criteria, use case and screen sketch; section 3 contains architecture, the Command pattern, UML and team responsibilities. Editable diagram sources are in `docs/diagrams/source/`. Keep the folder structure intact so relative links continue to work.

### Historical documents and change control

The proposal, first-submission report, screen sketch and original diagrams above are retained together as the submitted historical design. Their AI-related statements and older platform assumptions do not describe the current prototype. Their relative links and original bytes are preserved. Do not overwrite those diagrams with P2 images while leaving the old report unchanged.

P2 replaced the earlier AI contract path with predefined rules in `DiagnosticAgent` and a `RuleAudit` record. This explains the historical package; it does not freeze those exact class names for the new architecture. Predefined rules remain the current prototype direction. The replacement exact contract and UML must be reviewed together before implementation.

Team members must not add or change classes, fields, methods, helpers, overloads, visibility or relationships, including private members, outside a reviewed UML/source change. Update the contract, affected diagrams and Trello attachments together before implementing an approved structural change. User adoption is separate from peer review and tested behavior.

Preserve the incident core, separate booking and control processes, immutable evidence, bounded plan validation, explicit approval, allowlisted repair execution, independent verification and durable incident recording. Unknown evidence and uncertain execution must not be reported as success or blindly retried.

## Planning direction and system boundaries

- **Rivive Shield:** C++ desktop application, targeting Windows and macOS, with wxWidgets as the current GUI choice after the Windows feasibility trial. Its approved four UI areas cover target application status, observed activity, the active incident with approvals, and history. Monitoring runs in the background without blocking UI interaction. Closing Rivive Shield must not close the independent booking application.
- **Booking application:** C++ backend with cpp-httplib and SQLite, and a React browser frontend owned by Usama. No Java application is required. The booking backend and controller are separate executables; the React frontend is a separate web deliverable, not another C++ executable.
- **Local first:** browser access and HTTP are required locally from the start using localhost. They are not postponed until Oracle deployment. First establish a functioning local prototype; then move the booking backend and web frontend to Oracle under a separately verified remote-operation design.
- **Independent target:** the booking app must function without Rivive Shield running. Passive network observation and process-lifecycle evidence must not rely on the target reporting specially constructed diagnosis events to the controller.
- **Process ownership:** initially manage one designated isolated application. Process identity and ownership must be established using the reviewed native platform mechanisms before an operation can target it. A port number, process name, PID alone or packet snapshot is not sufficient authorization to terminate a process.
- **Recovery:** predefined evidence-based rules may propose bounded, allowlisted actions. Forced termination requires explicit, appropriately scoped approval. After confirmed termination, stop writers and perform the applicable SQLite checks; a new restart plan requires new approval. Passing a database check does not itself authorize restart. Permission to monitor does not authorize termination. Never replay saved requests, blindly retry uncertain writes, or claim recovery before independent checks pass.
- **Storage:** booking data and incident records use separate SQLite stores. Local settings and incident history are retained locally for the initial prototype. No SQLite database file is shared directly over a network.
- **Build organization:** the current Windows plan is one solution containing independently launchable C++ projects. A Visual Studio solution alone does not make a native macOS build available. Project build instructions still need verification on both platforms; CMake used to build an isolated experiment library is not adoption of CMake as the project build system.

### Observation limits

The first bounded network scope is authorized local IPv4 HTTP/1.1 traffic for the designated test application. The monitored stream must be reconstructed before interpreting HTTP messages. Missing data, unsupported protocols or requests, midstream capture and ambiguous ownership remain unknown or inconclusive. No observed traffic is not proof of application health or failure.

An HTTP response can be complete and still contain an error status such as 500. A 200 header can arrive with an incomplete response body. Connection existence does not prove that a request finished. Elapsed observation time is not the backend's internal execution time. A current operating-system connection-table snapshot does not retrospectively prove ownership of an earlier captured packet.

Request content may be parsed temporarily in memory within the approved test scope, but request bodies, credentials and raw packet captures are not to be stored in the incident record. Keep sanitized metadata and necessary evidence. Automatic request replay is deferred. HTTPS decryption/interception, HTTP/2, unrestricted network monitoring and device tracking are outside the current tested scope.

## Booking API and incident-state decisions

The approved booking routes are `GET /api/rooms`, `GET /api/bookings` and `POST /api/bookings`. Booking creation is atomic and idempotent: repeating the same request identity with the same normalized content must not create a second booking; reusing that identity with different content is a conflict. Use the adopted P3-R2 schema and technical-assumption documents as the working reference. Explicitly unresolved normalization, format, status-mapping and limit choices require recorded review before implementation; do not infer that adoption tested their behavior.

Store booking times in UTC and display them in the client device's timezone. Start must precede end. Adjacent bookings are valid: one booking may end exactly when the next begins. Enforce overlap prevention and idempotency atomically, not with a separate unprotected check then insert. Cancellation and any cancellation endpoint are deferred; no special diagnostic endpoint is required from the independent booking app.

The approved incident states are `Detected`, `Diagnosing`, `AwaitingApproval`, `Executing`, `Verifying`, `Recovered`, `NeedsReview`, `Declined` and `Unresolved`. The P3-R2 transition and C++ contract documents are now the adopted working reference, subject to their stated open assumptions and independent review. There must be no closure of an incident while an operation is in flight or its effects remain uncertain.

Keep one active incident per target and bind each approval to its incident, exact plan/version and target. Duplicate events must not execute an action twice; reuse of an event identity with different content is a conflict. Limits must be based on measurements and reviewed settings. Missing evidence or a limit exceedance produces an unknown/inconclusive result and an alert, not automatic termination. A new observation is not approval.

## Approved team responsibilities; applied to Trello

The owner has approved the revised distribution. The distribution and P3-R2 working baseline are approved. Trello synchronization has been applied and independently verified; no card is complete merely because its documentation was updated.

- **Usama:** React ([RS-31](https://trello.com/c/fk0HHQkw)), complex C++ observation (RS-11), incident states/core (RS-17/18), architecture, coordination, integration and Oracle/CI. The most complex C++ work remains his responsibility.
- **Samy:** reproducible scenarios, C++ booking backend, controlled tests and booking-rule acceptance (RS-05/08/09/21).
- **Ameen:** SQLite (RS-24), evidence persistence (RS-10), wxWidgets desktop UI (RS-14), and documentation coordination (RS-27).
- **Shehab:** predefined rules (RS-25), plan validation (RS-13) and diagnosis (RS-12).
- **Yazan:** lifecycle/recovery (RS-15), independent verification (RS-16), safe uncertain outcomes (RS-19), and coordinating, not solely writing, team tests (RS-22).

See the [full approved assignment table](docs/backlog-seed.md). The index records the applied dependencies. [RS-31](https://trello.com/c/fk0HHQkw) is the separate React development card, not the phases tracker.

Review rotation: Samy reviews Usama; Ameen reviews Samy; Shehab reviews Ameen; Yazan reviews Shehab; Usama reviews Yazan. Multi-author cards require an independent reviewer for each contribution. For RS-19, Usama reviews Yazan, and Samy reviews Usama's core changes. RS-23 fixes also require a non-author reviewer. Avatar order is not an ownership rule.

Each owner implements, tests, documents and fixes integration defects for their module. Keep one principal task in progress per person where feasible; sequence Ameen's storage, evidence and UI work, and schedule Samy's review load alongside Usama's development work. macOS acceptance is not implied by assigning work to Ameen.

## Implementation status and verification

At the reviewed repository commit `f93302721c4b5700920418184aed1dbacbba6f62`, the repository contains design documents and the declaration-only P2 package. The existing solution is not evidence of a runnable end-to-end application. The later experiments were performed separately from the project; their outputs do not establish that the application, deployment or CI is implemented.

### Windows feasibility evidence

The following outcomes were reported through the operator's command output and manual observations, not by a new automated run of the repository:

| Area | Observed scope | Important limit |
| --- | --- | --- |
| GUI | wxWidgets example built and its window was reported working | Not the Rivive Shield interface; macOS GUI pending |
| SQLite | Synthetic normal/WAL process-termination tests and missing/invalid/valid database checks passed | Not a power-loss test or proof of booking business rules |
| HTTP parser and TCP reassembly | Offline fragmented, reordered/retransmitted, missing-data and serial-session cases passed; integrated session suite 9/9 | Synthetic offline results are not live loss/reordering tests |
| Local passive HTTP | Complete response, two-part response, incomplete response, HTTP 500, no response and POST followed by GET on one connection behaved as expected | Restricted localhost HTTP/1.1; no packet-to-process attribution proof |
| Connection ownership snapshots | Three exact endpoint snapshots matched the expected live server/client processes | Observation at query time only, not historical packet attribution |
| Owned-process lifecycle | Normal exit and intentional forced termination of the test's own child were observed through the retained process handle | Exit code alone cannot prove user intent or crash cause |

For the final local session experiment, the client and server completed POST then GET on one connection; the passive monitor reported `pairs=2`, `capture_uncertain=0`, and exit code 0. In negative incomplete/no-response tests, `INCONCLUSIVE` and exit code 3 were expected, not claims of recovery. These are bounded feasibility results, not a security or full-compatibility certification.

### macOS status: pending, not assumed passed

A new automated experiment bundle was prepared for Ameen. It distinguishes PASS, FAIL, SKIP and INCONCLUSIVE, and does not install software, use sudo or change security settings. Missing tools or capture permissions result in SKIP. Its Linux preparation checks do not establish native macOS results. No `RESULTS.tsv` from Ameen has yet established his device's outcome; GUI confirmation and packet-to-process attribution remain separate gaps.

Planning and independent implementation work may continue while macOS testing is pending. Do not mark a macOS acceptance item complete or claim cross-platform compatibility until the actual results have been reviewed. Any incompatibility requires a tracked design or tooling adjustment, not an invented passing result.

### Remaining acceptance gates

- Independently review the adopted P3-R2 exact UML/contracts and synchronize affected card descriptions, dependencies and attachments.
- Apply the approved responsibilities consistently across the reviewed design, cards and explanations; preserve non-author review.
- Implement and integrate the actual controller, desktop UI, booking backend, React frontend and SQLite stores.
- Design and test per-request timeout classification; the curl timeouts in feasibility tests are not a controller feature.
- Resolve process-attribution uncertainty and validate platform-specific ownership safeguards before approval/execution can target an application.
- Verify approved recovery, SQLite checks after forced termination, preservation of valid bookings and durable incident recording end to end.
- Review native macOS results and document supported versions, tools and limitations.
- Verify Oracle hosting and GitHub Actions build/test/deployment before the final presentation. These remain required, not implemented or waived.

Oracle's operating system, remote management/repair channel, deployment configuration, capacity and cost boundaries remain explicit review items. The local controller cannot inspect a remote machine's process table or SQLite files using the local-only mechanisms demonstrated here. No remote agent, transport or remote access permission is implied by this README.

## Demonstration arrangements

Usama Shmasneh is the planned presenter. Testers will be selected randomly at the demonstration; their names are not yet known. Tester selection and presentation roles are separate from the actual contributors credited for the submitted files. For the graded final presentation, the assignment requires every team member to present (page 5, Stage 8; repeated on page 7). The presenter arrangement for that assessment therefore needs to be confirmed with the lecturer. This does not establish the participation rules for an earlier progress check.

## AI assistance and review

Usama Shmasneh plans to use ChatGPT. Other members plan to use a mix of ChatGPT, Gemini and Claude. These are development-assistance tools and are separate from the prototype's predefined diagnostic rules; the prototype itself does not run an AI model. Specific model versions and actual uses are not yet documented. The actual-use log and completed human-review record have not been provided; these must be documented in report section 3.6.

## Safety boundaries

Only the designated isolated demonstration application and fictitious data are in scope. Production systems, real personal data, device tracking and attacker identification are excluded. An observed fault is not automatically classified as an attack. Unsupported causes remain unresolved. Repairs require human approval and independent verification before success is reported.

## Submission procedure

The first report is submitted as PDF with the Markdown sources, required images, README and linked backlog available in the shared repository. Each member submits the same specific commit link individually in Moodle and lists the actual contributors. Check the current Moodle deadline. Individual research and reflection remain private Moodle submissions, outside the team repository.

## Sources

Course requirements: `1-rolling_assignment_course_project_en_v19.docx`, pages 2–6 and 8; `SDM_Initiation_proposal_v21.pdf`, pages 14–17 and 27–28; and `SDM_Teamwork_part2_en_v10.pdf`, pages 5–9. The earlier Rivive Shield proposal dated 28 October 2026 supplies the project concept. Official alternative-tool sources are linked in report section 1.2.

