# Rivive Shield

**Course:** Software Development Methods  
**Team leader:** Usama Shmasneh  308127125
**Team members:** Samy mansour   208547190, Ameen Abd Allader   212809230, Shehab Shehab   208263376, Yazan Ganem   214518342
**Submission:** Proposal, requirements and system design, 4 November 2026  
**Lecturer decision:** Project approved, as reported by Usama Shmasneh; approval date and any conditions not provided.

## Project overview

Rivive Shield is a proposed learning and resilience-testing system for one isolated study-room booking application. It connects fault detection, evidence-based AI diagnosis, an explicitly approved repair and independent verification. Its intended users are the demonstration operator and course participants working with fictitious data.

The MVP contains three user stories: participate in a controlled challenge; investigate and recover a supported incident, the central story; and verify protection and inspect the saved incident record. The two proposed scenarios are a stopped booking service and acceptance of overlapping active bookings for the same room. The report specifies the planned detection, approval, repair and verification behavior.

## Project links

| Item | Location |
| --- | --- |
| Shared GitHub repository | https://github.com/1UsamaShmasneh1/Rivive_Shield |
| Trello backlog board | https://trello.com/b/GeLevegk/riviveshield |

The [project backlog](docs/backlog-seed.md) supplies task titles, priorities, planned owners, dependencies and acceptance evidence for the live board. It does not replace the required board.

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

## Team responsibilities

| Member | Planned responsibility | Reviewer |
| --- | --- | --- |
| Usama Shmasneh | C++ incident orchestrator and state machine; leadership; SQLite/SQL; external AI connection; Oracle; GitHub Actions; integration adapters and minimal web/API interface | Samy mansour |
| Samy mansour | C++ booking domain, controlled overlap scenario and booking-rule guard | Ameen Abd Allader |
| Ameen Abd Allader | C++ monitoring, evidence collection and independent invariant detection | Shehab Shehab |
| Shehab Shehab | C++ diagnostic module, bounded tool contracts, plan validation and local operator CLI | Yazan Ganem |
| Yazan Ganem | C++ approved repair commands, independent recovery verification and test runner | Usama Shmasneh |

Usama's incident core coordinates state transitions, asynchronous events, plan/approval binding, bounded dispatch, verification and interrupted execution. This responsibility is additional to his leadership and specialist work. Other members retain their own module implementation, tests, documentation and defect fixes. Yazan also checks the core-to-repair handoff.

The team shares a C++ background. Database implementation, SQL, cloud administration, deployment workflows, external AI-service authentication and browser/API integration remain with Usama. Other members use typed C++ interfaces and clearly labeled test doubles during development. The finished demonstration will require real adapters and actual AI access.

## Implementation status and planned stack

This submission specifies a proposed system. A verified working build, deployment and test results are not included. Build prerequisites, configuration and launch instructions will be documented with the implementation.

The intended stack is C++, SQLite and a minimal browser interface. Oracle Cloud hosting and GitHub Actions are planned, subject to feasibility and access constraints. Development begins with a local end-to-end workflow before cloud deployment or interface polish.

## Demonstration arrangements

Usama Shmasneh is the planned presenter. Testers will be selected randomly at the demonstration; their names are not yet known. Tester selection and presentation roles are separate from the actual contributors credited for the submitted files. For the graded final presentation, the assignment requires every team member to present (page 5, Stage 8; repeated on page 7). The presenter arrangement for that assessment therefore needs to be confirmed with the lecturer. This does not establish the participation rules for an earlier progress check.

## AI assistance and review

Usama Shmasneh plans to use ChatGPT. Other members plan to use a mix of ChatGPT, Gemini and Claude. Specific model versions and actual uses are not yet documented; these planned tools do not select the application's runtime AI provider. The actual-use log and completed human-review record have not been provided; these must be documented in report section 3.6.

## Safety boundaries

Only the designated isolated demonstration application and fictitious data are in scope. Production systems, real personal data, device tracking and attacker identification are excluded. An observed fault is not automatically classified as an attack. Unsupported causes remain unresolved. Repairs require human approval and independent verification before success is reported.

## Submission procedure

The first report is submitted as PDF with the Markdown sources, required images, README and linked backlog available in the shared repository. Each member submits the same specific commit link individually in Moodle and lists the actual contributors. Check the current Moodle deadline. Individual research and reflection remain private Moodle submissions, outside the team repository.

## Sources

Course requirements: `1-rolling_assignment_course_project_en_v19.docx`, pages 2–6 and 8; `SDM_Initiation_proposal_v21.pdf`, pages 14–17 and 27–28; and `SDM_Teamwork_part2_en_v10.pdf`, pages 5–9. The earlier Rivive Shield proposal dated 28 October 2026 supplies the project concept. Official alternative-tool sources are linked in report section 1.2.
