# Rivive Shield design navigation

## Current working baseline: P3-R2 adopted

The owner adopted P3-R2 as the working design baseline on 2026-11-13. [P3 adoption status](P3_STATUS.md) is authoritative for adoption; original review/proposal labels inside the frozen package describe its preparation state. They do not negate the later adoption.

Start with the [P3-R2 guide](Rivive_Shield_UML_Proposal_P3/README_AR.md), [67-type exact declarations](Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp), [exact catalog](Rivive_Shield_UML_Proposal_P3/docs/EXACT_CATALOG.md), and [validation limits](Rivive_Shield_UML_Proposal_P3/checks/VALIDATION.md). Read the [approved assignments](Rivive_Shield_UML_Proposal_P3/APPROVED_ASSIGNMENTS_AR.md) and [backlog index](../backlog-seed.md).

Adoption is not completed independent review or implementation. macOS verification, MSVC validation of this package, actual linking, runtime recovery, GUI integration and Oracle operation remain subject to their stated acceptance gates. Explicitly open settings must be resolved through review, not guessed.

No silent additions or changes to classes, fields, methods or private helpers are permitted. Review and synchronize exact source, diagrams and card attachments for structural changes.

## Historical P2 reference

[P2 adoption status](P2_APPROVAL_STATUS.txt) is preserved as the record of its earlier adoption. Its baseline language describes that earlier state, not the current adoption record for P3. P2 is not the exact implementation contract for the changed direction.

The P2 directory and its companion adoption record are unchanged. Preserve its filenames, preparation labels, checksums and original bytes. Do not modify historical reports or original diagrams to make them appear to describe P3.

- [Arabic memorandum](Rivive_Shield_UML_Proposal_P2/docs/review_ar.md)
- [Change log](Rivive_Shield_UML_Proposal_P2/docs/change_log.md)
- [Exact historical header](Rivive_Shield_UML_Proposal_P2/contracts/rivive_shield_proposed.hpp)
- [Historical contract notes](Rivive_Shield_UML_Proposal_P2/contracts/contract_notes.md)
- [Historical signature catalog](Rivive_Shield_UML_Proposal_P2/contracts/signature_catalog.md)
- [Checks and untested gates](Rivive_Shield_UML_Proposal_P2/checks/verification.txt)

Full P2 class diagrams remain under `Rivive_Shield_UML_Proposal_P2/diagrams/png/classes/`. Overview diagrams are navigation aids, not exact contracts.

## Change and acceptance control

Review the replacement UML and exact C++ declarations together, including private helpers, visibility, overloads and relationships. Synchronize affected Trello content and attachments before implementing structural changes. No item becomes Done merely because a planning document was revised.

P2 declaration-only checks remain useful only for P2. They do not link or run the new application, validate P3, or establish Windows/macOS compatibility.
