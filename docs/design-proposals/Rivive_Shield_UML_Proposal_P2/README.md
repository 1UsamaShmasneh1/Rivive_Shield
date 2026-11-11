# Rivive Shield P2 review package
**PROPOSED. NOT APPROVED. NOT IMPLEMENTED.**

Start with docs/review_ar.md (Arabic review memorandum), then docs/change_log.md and contracts/contract_notes.md.

- contracts/rivive_shield_proposed.hpp: C++17 declarations only.
- contracts/compile_contracts.cpp: syntax/inheritance declaration checks; not an executable.
- contracts/contract_inventory.json and signature_catalog.md: every declared class/struct member, visibility, enum and alias.
- diagrams/source/classes and diagrams/png/classes: exact per-class UML, including private fields/helpers. Overview diagrams are navigation aids, not substitutes for full signatures.
- docs/P1_to_P2_header.diff: exact changes from original P1.
- checks/verification.txt: actual checks and explicit untested gates.
- SHA256SUMS: integrity list, excluding itself.

Native Windows/macOS is the target, not an accomplished portability result. No runtime repair, SQLite, race safety, remote Oracle or CI tests have run. The local smoke check uses GCC on a Linux review computer only. Obtain RS-06/RS-07 review before implementation. Keep main controller local when later moving booking backend/web to Oracle; remote control design is open.
