# P2 provenance and decision record
Status: proposed for review, not approved or implemented.

## Sources
- Original P1 source ZIP: Rivive_Shield_UML_Proposal_P1_Sources.zip. Original header SHA256: 88b5025d9d2ff0e8372f1a4a1d384dc450d94f00e2e9871b787563d18a202ef8.
- Latest README read via GitHub: https://github.com/1UsamaShmasneh1/Rivive_Shield/blob/main/README.md ; blob cc2fbe0f07ece79bc3cdb2ec4d5b2ff67ce5e32e . Read on 2026-11-11. It records predefined rules/no model and says detailed contracts still need reconciliation.
- User explicitly chose rules inside diagnosis code, local Windows/macOS first, Oracle backend/web later with controller local. These later decisions override incompatible P1 runtime assumptions but do not approve this specific API design.
- Trello board https://trello.com/b/GeLevegk/riviveshield ; relevant cards RS-03 https://trello.com/c/ICpHY3nI , RS-06 https://trello.com/c/3M4GExqE , RS-07 https://trello.com/c/4xui3soD , RS-12 https://trello.com/c/f8GbjHt9 , RS-24 https://trello.com/c/vxp2NJIv , RS-25 https://trello.com/c/Z1RVq0IV . Card summaries were provided by the parent review; this package does not claim a new live audit of every card.

## Accepted direction, not blanket API approval
Compiled-in predefined rules, no runtime model/provider, local native Windows/macOS first, preserve explicit human approval and independent verification. The submitted report remains unchanged historical material.

## Proposed implementation contracts requiring review
RuleAudit and its persistence; private DiagnosticAgent helpers; OwnedDemoProcess interface and Windows/macOS declarations; immutable configuration match and lifetime; ProcessStatus distinct from application health. All inherited P1 refinements remain proposals unless separately approved. Do not treat unchanged P1 contracts as automatically approved.

## Open decisions
Native launch/stop/cleanup/error sequences and libraries; exact fixed commands and HTTP transport; deadlines/read/data/queue limits; schemas/migrations/durability and fixture initialization; rule version identifiers and evidence freshness policy; Oracle deployment OS, endpoint authentication, remote management channel, host of incident store, multi-controller coordination and independent stop proof. Oracle hosting and CI remain later pre-presentation requirements, not verified deployment.

## Compatibility
No implementation is supplied, and no claim is made that the latest README or Trello already reflects every proposed signature. RS-06 and RS-07 must reconcile and approve a specific revision before member implementation. New helpers/fields require another UML/card change. Do not automatically overwrite the original report, push this package, or mark design cards Done.
