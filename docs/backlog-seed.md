# Rivive Shield: Current RS Backlog Index

Design baseline: user-adopted P2. This index reflects the P2 card content captured on 11 November 2026; [Trello](https://trello.com/b/GeLevegk/riviveshield) is authoritative for live status, checklists and evidence. It does not mark any work complete or assign dates. Card numbers are identifiers, not a strict execution order: complete dependencies and peer-review gates before dependent work.

The original 4 November B01–B26 planning backlog is preserved in [history](history/backlog-seed-2026-11-04.md). It contains superseded runtime-AI assumptions and must not be treated as the active implementation plan. First-submission reports remain historical records.

| Card | Work | Planned owner/coordinator | Reviewer | Dependencies | Priority |
| --- | --- | --- | --- | --- | --- |
| [RS-01](https://trello.com/c/w8omCA70) | Confirm approved scope and remaining gates | Usama with the team | Samy | None | P1 Core |
| [RS-02](https://trello.com/c/z5E0dAka) | Organize the board and review workflow | Usama | Samy | None | P1 Core |
| [RS-03](https://trello.com/c/ICpHY3nI) | Verify repository access and C++ tooling | All members; Usama coordinates | Samy | [RS-02](https://trello.com/c/z5E0dAka) | P1 Core |
| [RS-04](https://trello.com/c/y3WQWD8N) | Review and preserve existing repository work | Usama | Samy | [RS-03](https://trello.com/c/ICpHY3nI) | P1 Core |
| [RS-05](https://trello.com/c/oBygljPv) | Specify reproducible booking challenges | Samy with the team | Ameen | [RS-01](https://trello.com/c/w8omCA70) | P1 Core |
| [RS-06](https://trello.com/c/3M4GExqE) | Review and freeze exact P2 C++ contracts | Usama with module owners | Samy | [RS-04](https://trello.com/c/y3WQWD8N), [RS-05](https://trello.com/c/oBygljPv) | P1 Core |
| [RS-07](https://trello.com/c/4xui3soD) | Review application and transport architecture | Usama | Samy | [RS-04](https://trello.com/c/y3WQWD8N), [RS-06](https://trello.com/c/3M4GExqE) | P1 Core |
| [RS-08](https://trello.com/c/7SNJxI1s) | Build the real study-room booking app | Samy | Ameen | [RS-05](https://trello.com/c/oBygljPv), [RS-06](https://trello.com/c/3M4GExqE), [RS-07](https://trello.com/c/4xui3soD), [RS-24](https://trello.com/c/vxp2NJIv) | P1 Core |
| [RS-09](https://trello.com/c/w3I0caGJ) | Implement controlled reproducible challenges | Samy | Ameen | [RS-07](https://trello.com/c/4xui3soD), [RS-08](https://trello.com/c/7SNJxI1s) | P1 Core |
| [RS-10](https://trello.com/c/DB4xMMBV) | Persist immutable incident evidence | Ameen | Shehab | [RS-06](https://trello.com/c/3M4GExqE), [RS-07](https://trello.com/c/4xui3soD), [RS-24](https://trello.com/c/vxp2NJIv) | P1 Core |
| [RS-11](https://trello.com/c/kBQBuDyX) | Detect faults with independent observations | Ameen | Shehab | [RS-05](https://trello.com/c/oBygljPv), [RS-09](https://trello.com/c/w3I0caGJ), [RS-10](https://trello.com/c/DB4xMMBV) | P1 Core |
| [RS-12](https://trello.com/c/f8GbjHt9) | Implement bounded rule-based diagnosis | Shehab | Yazan | [RS-06](https://trello.com/c/3M4GExqE), [RS-11](https://trello.com/c/kBQBuDyX), [RS-13](https://trello.com/c/prRaLAlP), [RS-25](https://trello.com/c/Z1RVq0IV) | P1 Core |
| [RS-13](https://trello.com/c/prRaLAlP) | Validate exact bounded repair plans | Shehab | Yazan | [RS-06](https://trello.com/c/3M4GExqE), [RS-11](https://trello.com/c/kBQBuDyX) | P1 Core |
| [RS-14](https://trello.com/c/1ANfiflb) | Build CLI and exact approval flow | Shehab | Yazan | [RS-13](https://trello.com/c/prRaLAlP), [RS-18](https://trello.com/c/mOcmXRih) | P1 Core |
| [RS-15](https://trello.com/c/V5RXnBmH) | Implement durable bounded repair commands | Yazan | Usama | [RS-07](https://trello.com/c/4xui3soD), [RS-13](https://trello.com/c/prRaLAlP), [RS-24](https://trello.com/c/vxp2NJIv) | P1 Core |
| [RS-16](https://trello.com/c/18kqnrdC) | Verify recovery independently and preserve data | Yazan | Usama | [RS-08](https://trello.com/c/7SNJxI1s), [RS-11](https://trello.com/c/kBQBuDyX), [RS-15](https://trello.com/c/V5RXnBmH) | P1 Core |
| [RS-17](https://trello.com/c/dUmYOt3G) | Review complete incident state transitions | Usama | Samy | [RS-06](https://trello.com/c/3M4GExqE) | P1 Core |
| [RS-18](https://trello.com/c/mOcmXRih) | Implement the serialized incident core | Usama | Samy | [RS-07](https://trello.com/c/4xui3soD), [RS-17](https://trello.com/c/dUmYOt3G), [RS-24](https://trello.com/c/vxp2NJIv) | P1 Core |
| [RS-19](https://trello.com/c/cIJG5Pa4) | Stop safely on failed or uncertain work | Yazan with Usama | Usama; Samy reviews Usama-authored changes | [RS-15](https://trello.com/c/V5RXnBmH), [RS-16](https://trello.com/c/18kqnrdC), [RS-18](https://trello.com/c/mOcmXRih) | P1 Core |
| [RS-20](https://trello.com/c/FUbPhX18) | Integrate the real end-to-end application | Usama with all members | Samy | [RS-09](https://trello.com/c/w3I0caGJ), [RS-10](https://trello.com/c/DB4xMMBV), [RS-11](https://trello.com/c/kBQBuDyX), [RS-12](https://trello.com/c/f8GbjHt9), [RS-13](https://trello.com/c/prRaLAlP), [RS-14](https://trello.com/c/1ANfiflb), [RS-15](https://trello.com/c/V5RXnBmH), [RS-16](https://trello.com/c/18kqnrdC), [RS-17](https://trello.com/c/dUmYOt3G), [RS-18](https://trello.com/c/mOcmXRih), [RS-19](https://trello.com/c/cIJG5Pa4), [RS-24](https://trello.com/c/vxp2NJIv), [RS-25](https://trello.com/c/Z1RVq0IV) | P1 Core |
| [RS-21](https://trello.com/c/8SyozJho) | Prove overlap prevention and valid booking | Samy with Ameen and Yazan | Ameen | [RS-20](https://trello.com/c/FUbPhX18) | P1 Core |
| [RS-22](https://trello.com/c/5BbJScMJ) | Run module and system acceptance tests | All members; Yazan coordinates | Usama | [RS-20](https://trello.com/c/FUbPhX18), [RS-21](https://trello.com/c/8SyozJho) | P1 Core |
| [RS-23](https://trello.com/c/PZHDZwvh) | Fix defects and rerun regression tests | Affected module owner; Usama coordinates | Non-author reviewer under the shared rotation | [RS-22](https://trello.com/c/5BbJScMJ) | P1 Core |
| [RS-24](https://trello.com/c/vxp2NJIv) | Implement mandatory durable SQLite storage | Usama | Samy | [RS-06](https://trello.com/c/3M4GExqE), [RS-07](https://trello.com/c/4xui3soD) | P1 Core |
| [RS-25](https://trello.com/c/Z1RVq0IV) | Prepare predefined diagnosis and repair rules | Usama with Shehab | Samy | [RS-06](https://trello.com/c/3M4GExqE), [RS-07](https://trello.com/c/4xui3soD) | P1 Core |
| [RS-26](https://trello.com/c/ViVBEepG) | Implement Oracle Cloud hosting and GitHub Actions CI | Usama | Samy | [RS-03](https://trello.com/c/ICpHY3nI), [RS-07](https://trello.com/c/4xui3soD) to start; [RS-22](https://trello.com/c/5BbJScMJ), [RS-23](https://trello.com/c/PZHDZwvh) for completion | P1 Core |
| [RS-27](https://trello.com/c/hyIc3RbD) | Complete shared documentation and run guide | Ameen coordinates; all members document their modules | Shehab | [RS-20](https://trello.com/c/FUbPhX18), [RS-26](https://trello.com/c/ViVBEepG) | P1 Core |
| [RS-28](https://trello.com/c/xiGrPpYd) | Review the exact submission package | Usama; all members review | Samy | [RS-01](https://trello.com/c/w8omCA70), [RS-23](https://trello.com/c/PZHDZwvh), [RS-27](https://trello.com/c/hyIc3RbD) | P1 Core |
| [RS-29](https://trello.com/c/JyprATIq) | Prepare the presentation with all members | All members; Usama coordinates | Samy | [RS-20](https://trello.com/c/FUbPhX18), [RS-22](https://trello.com/c/5BbJScMJ), [RS-27](https://trello.com/c/hyIc3RbD) | P1 Core |
| [RS-30](https://trello.com/c/mcCT33ZJ) | Rehearse, submit and verify completion | All members; Usama follows up | Samy | [RS-28](https://trello.com/c/xiGrPpYd), [RS-29](https://trello.com/c/JyprATIq) | P1 Core |

## Scope and handoffs

Use the [P2 baseline and adoption status](design-proposals/README.md). The prototype uses predefined diagnostic/repair rules in code, not an external or local AI model. Rules must follow real observations; the challenge selector must not replace diagnosis. Repairs need bound approval and independent verification. Unknown or uncertain outcomes must remain unresolved until safely reconciled.

Start locally on Windows/macOS. Later only the booking backend and test web app move to Oracle; the controller remains local. Separate SQLite booking/incident stores, native process handling, Oracle remote-control design and deployment gates must follow P2 and the specific card decisions. Oracle and GitHub Actions remain required before the final presentation, not completed capabilities.

Each module owner implements, tests, documents and resolves their integration defects. Usama owns the incident core plus SQLite/SQL, Oracle, CI and integration/web adapters. Review rotation: Samy reviews Usama; Ameen reviews Samy; Shehab reviews Ameen; Yazan reviews Shehab; Usama reviews Yazan. Reviewers' names and roles in descriptions are authoritative; avatar order is not an assignment rule.

## Completion

Complete the card-specific acceptance checks, record appropriate test/output evidence, obtain the named peer review, and update relevant documentation before marking Done. Non-code cards require evidence suitable for their outputs, not irrelevant code tests. User adoption of P2 is not peer review or runtime success. AI development assistance must be human-reviewed and its actual use documented.

No fields, methods, helpers, overloads, visibility or relationships may diverge silently from P2. Review a UML/source change and synchronize affected card attachments before implementing structural changes.
