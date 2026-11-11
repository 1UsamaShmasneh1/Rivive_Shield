# Current Rivive Shield design: P2

Usama adopted P2 for the project baseline. This is a design adoption, not a record of completed Samy/module-owner review or successful implementation. RS-06 and RS-07 reviews and the runtime acceptance gates remain outstanding.

## Start here

1. Read [adoption status](P2_APPROVAL_STATUS.txt).
2. Read the [Arabic memorandum](Rivive_Shield_UML_Proposal_P2/docs/review_ar.md) and [change log](Rivive_Shield_UML_Proposal_P2/docs/change_log.md).
3. Use the [exact header](Rivive_Shield_UML_Proposal_P2/contracts/rivive_shield_proposed.hpp), [contract notes](Rivive_Shield_UML_Proposal_P2/contracts/contract_notes.md) and [signature catalog](Rivive_Shield_UML_Proposal_P2/contracts/signature_catalog.md).
4. Consult full class diagrams in `Rivive_Shield_UML_Proposal_P2/diagrams/png/classes/`; overview diagrams are navigation aids, not full contracts.
5. Read [checks and untested gates](Rivive_Shield_UML_Proposal_P2/checks/verification.txt).

## Why the package says PROPOSED

The P2 directory preserves the reviewed preparation artifact byte-for-byte, including its earlier labels, filenames and SHA256SUMS. The labels describe its preparation status, superseded only as to Usama's adoption by the companion status above. Do not silently rewrite the package or interpret adoption as peer-review approval. Proposed behaviors still need implementation and verification.

The original report and diagrams under `docs/` remain the first-submission historical record. They are not the active class baseline. Structural changes, including private helper members, require an approved UML/source update and synchronized card attachments. No application code is supplied by this document import.

## Declaration-only checks

From the P2 package directory, run `python checks/check_consistency.py`. With GCC available, from `contracts/` run `g++ -std=c++17 -Wall -Wextra -Wpedantic -Werror -fsyntax-only compile_contracts.cpp`. These checks do not link or run an application. Use native MSVC and Apple Clang checks separately before claiming Windows/macOS compatibility.
