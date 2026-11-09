# Rivive Shield

**Course:** Software Development Methods  
**Team leader:** Usama Shmasneh  
**Team members:** Samy mansour, Ameen Abd Allader, Shehab Shehab, Yazan Ganem  
**Submission:** Proposal, requirements and system design, 4 November 2026  
**Lecturer decision:** Project approved, as reported by Usama Shmasneh. Usama also reports lecturer approval of the initial prototype using predefined diagnostic rules and repair solutions without an external or locally running AI model. The original approval date and any additional conditions have not been provided.

## Project overview

Rivive Shield is a proposed learning and resilience-testing system for one isolated study-room booking application. It connects fault detection, evidence-based diagnosis using predefined rules, an explicitly approved repair and independent verification. Its intended users are the demonstration operator and course participants working with fictitious data.

The MVP contains three user stories: participate in a controlled challenge; investigate and recover a supported incident, the central story; and verify protection and inspect the saved incident record. The two proposed scenarios are a stopped booking service and acceptance of overlapping active bookings for the same room. The first-submission report describes the original design; its runtime-AI requirements require reconciliation with the revised prototype scope below.

The initial prototype uses predefined diagnostic rules and repair solutions. It does not use an external AI provider or a locally running AI model. Rules match collected evidence against supported conditions and propose an allowlisted repair with its evidence references and preconditions. The hidden challenge selector must not be used as the diagnosis. Unknown or insufficient evidence leaves the incident unresolved.

Predetermined repair rules do not predetermine the outcome. The application must exhibit the actual fault, an operator must explicitly approve the proposed repair, and independent checks must establish recovery, preservation of unaffected data and successful persistence before the system reports success. Further development will be considered after the prototype has been tested and shown to work; no specific later AI integration, provider or schedule has been selected.

## Project links

| Item | Location |
| --- | --- |
| Shared GitHub repository | https://github.com/1UsamaShmasneh1/Rivive_Shield |
| Trello backlog board | https://trello.com/b/GeLevegk/riviveshield |

The [project backlog](docs/backlog-seed.md) supplies the initial task titles, priorities, planned owners, dependencies and acceptance evidence for the live board. It does not replace the required board. The backlog file and live Trello cards must be reconciled with the revised prototype scope; older references to mandatory runtime AI must not be treated as current prototype requirements.

## Submission documents

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

### Design and contract reconciliation required

The linked first-submission documents record the earlier design. The report, backlog, UML diagrams and detailed C++ contract baseline are not yet established as fully consistent with the revised rule-based prototype.

In particular, the current detailed contracts still include a `DiagnosticAgent` dependency on `AiProvider&`, an `AiAudit` member in `DiagnosisResult`, and `maxProviderTurns` in `DiagnosticLimits`. These are unresolved design dependencies. This README update does not remove, rename or approve replacement of any class, field, method, constructor parameter or interface.

Before implementing the affected diagnostic components, review the necessary contract changes through RS-06 and RS-07, obtain approval, and update the corresponding UML, contract baseline and card attachments together. Team members must follow the approved baseline and must not add or alter principal or helper fields and methods outside that review process. A rule-based implementation must not be presented as actual runtime AI merely because an older interface retains an AI-related name.

Preserve the incident core, separate booking and control processes, evidence collection, plan validation, human approval, restricted repair execution, independent verification and durable incident recording. Operating-platform and process-management choices, HTTP/browser contracts and concrete execution limits remain separate decisions requiring review.

## Team responsibilities

| Member | Planned responsibility | Reviewer |
| --- | --- | --- |
| Usama Shmasneh | C++ incident orchestrator and state machine; leadership; SQLite/SQL; Oracle; GitHub Actions; integration adapters and minimal web/API interface | Samy mansour |
| Samy mansour | C++ booking domain, controlled overlap scenario and booking-rule guard | Ameen Abd Allader |
| Ameen Abd Allader | C++ monitoring, evidence collection and independent invariant detection | Shehab Shehab |
| Shehab Shehab | C++ rule-based diagnostic module, bounded tool contracts subject to the required contract review, plan validation and local operator CLI | Yazan Ganem |
| Yazan Ganem | C++ approved repair commands, independent recovery verification and test runner | Usama Shmasneh |

Usama's incident core coordinates state transitions, asynchronous events, plan/approval binding, bounded dispatch, verification and interrupted execution. This responsibility is additional to his leadership and specialist work. Other members retain their own module implementation, tests, documentation and defect fixes. Yazan also checks the core-to-repair handoff.

The team shares a C++ background. Database implementation, SQL, cloud administration, deployment workflows and browser/API integration remain with Usama. Other members use reviewed typed C++ interfaces and clearly labeled test doubles during development. The prototype demonstration requires the real booking application, the required adapters and predefined diagnostic and repair rules. Test doubles and prerecorded success results do not establish successful end-to-end recovery. External AI-service setup is not a prerequisite for this prototype.

## Implementation status and planned stack

This submission specifies a proposed system. A verified working build, deployment and test results are not included. Build prerequisites, configuration and launch instructions will be documented with the implementation.

The intended stack is C++, SQLite and a minimal browser interface. Durable SQLite storage is required. Oracle Cloud hosting and a GitHub Actions build, test and deployment workflow are required before the final presentation. Their detailed infrastructure, access, capacity, cost constraints and deployment configuration still require review and verification; this requirement is not evidence that deployment has been implemented.

Development begins with a local end-to-end workflow before cloud deployment or interface polish. SQLite schema, migrations and durability settings; HTTP/browser contracts; the process platform and process-management approach; and concrete timeouts and other limits remain open until reviewed and recorded in their respective Trello cards. No operating system, paid service or numerical limit is selected by this README update.

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
