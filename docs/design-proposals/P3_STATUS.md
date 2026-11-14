# P3-R2 working baseline adoption

**Status: APPROVED WORKING DESIGN BASELINE. Owner adoption: 2026-11-13.**

The owner adopted the complete P3-R2 snapshot as the basis for work. This companion is authoritative for adoption. The [snapshot](Rivive_Shield_UML_Proposal_P3/README_AR.md) contains 67 struct/class declarations, their matching catalog and diagrams; original proposed/review wording and filenames are preserved as preparation history. No source bytes or checksums were rewritten merely to relabel adoption.

Adopted archive SHA256: `1d82db428ace0e72a697c24cca22f32147ccfffd320789fde41eeb0870f7faea`.
Package: `Rivive_Shield_UML_Proposal_P3/`; see [revision](Rivive_Shield_UML_Proposal_P3/REVISION.md) and [checksums](Rivive_Shield_UML_Proposal_P3/SHA256SUMS).

Owner adoption, independent peer review, implementation and tested acceptance are separate. macOS results remain pending. Open settings and technical assumptions recorded in the adopted snapshot are not silently resolved by this status.

## Adopted direction

- C++ desktop controller with wxWidgets, targeting Windows and macOS.
- Independent C++ booking backend and SQLite storage, with a React browser frontend.
- Browser access and HTTP on localhost from the first local prototype.
- Predefined diagnostic and repair rules, without a runtime AI model.
- Authorized, bounded passive observation; ambiguous ownership or incomplete evidence stays inconclusive.
- Explicit approval for bounded recovery, with independent verification afterward.
- Separate local booking/incident stores; no saved request-body replay.
- Oracle deployment follows a working local prototype and a reviewed remote-operation design. Local process and database access must not be presented as remote access.

## Working contracts and open implementation choices

The desktop interface has four areas: status, activity, the active incident and approvals, and history. Monitoring runs in the background; closing the controller must not close the independent booking app.

Approved API routes: `GET /api/rooms`, `GET /api/bookings`, `POST /api/bookings`. Store times in UTC, display client-local time, require start before end, and allow adjacent bookings. Creation must be atomic and idempotent; an identical request identity/content cannot duplicate a booking, and identity reuse with different content conflicts. JSON fields, normalized comparison, wire-time format, response codes and size limits are not yet frozen. Cancellation is deferred.

Approved states: `Detected`, `Diagnosing`, `AwaitingApproval`, `Executing`, `Verifying`, `Recovered`, `NeedsReview`, `Declined`, `Unresolved`. The P3-R2 transition documents are the adopted working reference; explicit open details require review before implementation. Do not close an incident with an in-flight action or uncertain effect. Keep one active incident per target; bind approval to incident, exact plan/version and target. Reject conflicting event identities and prevent duplicate execution.

Forced termination needs explicit approval. After confirmed termination, check SQLite with writers stopped, then obtain new approval for the restart plan. Measurement-based limits or incomplete evidence lead to unknown/inconclusive outcomes and alerts, never automatic termination.

## Approved assignment status: applied to Trello

The owner has approved the complete [assignment table](../backlog-seed.md), including RS-11 observation for Usama, RS-14 desktop UI and RS-24 SQLite for Ameen, RS-25 rules for Shehab, and new RS-31 React for Usama. Other roles follow that table. P3-R2 is adopted; the card contents, assignments and attachments have been applied to Trello and independently verified. RS-01 and RS-02 remain completed historical records, unchanged.

Review rotation: Samy reviews Usama; Ameen reviews Samy; Shehab reviews Ameen; Yazan reviews Shehab; Usama reviews Yazan. Multi-author work requires non-author review for each contribution. [RS-31 React](https://trello.com/c/fk0HHQkw) has been created and is separate from the phases tracker.

## Evidence status

Windows experiments provide bounded feasibility evidence, not an implemented end-to-end system. Native macOS testing on Ameen's device is pending. Do not assume PASS, mark its acceptance checks complete, or claim full cross-platform compatibility without actual results. Missing tools or permissions may yield SKIP rather than PASS.

## Remaining review and implementation gates

1. Complete and review scenario scope and acceptance criteria.
2. Review the prepared exact C++ contracts, matching UML and structural change explanation relative to P2; resolve identified issues before adoption.
3. Review architecture, process identity, observation uncertainty, approval/execution boundaries and independent verification.
4. Apply the approved owners/reviewers and review the revised dependencies.
5. Synchronize README, Trello cards, Arabic explanations and links without altering historical P2 artifacts.
6. Keep this adoption record separate from subsequent peer-review outcomes.
7. Implement and collect native-platform and integration evidence before claiming runtime acceptance.

The original P2 package and [P2 adoption record](P2_APPROVAL_STATUS.txt) remain historical, unchanged references. No original report, diagram, source contract or checksum is replaced by this status page.
