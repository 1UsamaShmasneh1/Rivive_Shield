# Native macOS feasibility evidence, reviewed 2026-11-15

## Source and scope

This record summarizes Ameen's experiment reports supplied by Usama. It is a review of reported output, not an independent rerun. The project baseline remains P3-R2. No source contract, UML, schema, assignment or frozen package checksum is changed by these results.

The general results, separate `GUI_RESULTS` report and final capture-only `RESULTS(2).tsv` must be interpreted together. Earlier missing-tool or permission results describe earlier attempts; a later scoped success does not retrospectively make those attempts PASS.

## Reported outcomes

| Area | Reported outcome | Boundary |
| --- | --- | --- |
| Native environment | arm64 compiler/SDK and bundle integrity checks passed | Not the full project build |
| Compiler correction | Explicit `-isysroot "$(xcrun --show-sdk-path)"` resolved the direct-compiler `sys/types.h` lookup failure | Retain SDK resolution in experiment build commands; no project build-system decision implied |
| wxWidgets | 3.2.12 build passed; open/resize/button/close manually confirmed | Smoke-test window, not integrated controller UI |
| Owned-child lifecycle | Normal test exit and explicitly requested SIGKILL observed | No proof of arbitrary-process ownership or crash/user intent |
| SQLite | Normal, DELETE-mode process crash, WAL-mode process crash, missing-file/no-create, invalid-file rejection and foreign-key violation detection passed | No power-loss, booking semantics, production schema or complete recovery proof |
| HTTP parser | Four synthetic cases passed | Not unrestricted HTTP support |
| TCP/HTTP | Isolated PcapPlusPlus build and nine offline reassembly/session cases passed | Offline loss/reorder tests are not live-network coverage |
| Passive capture | Six bounded localhost scenarios passed in the final capture-only rerun | No packet-to-process attribution, repair authorization or full-project verification |

## Final capture-only report

The final report contains **13 PASS rows**: `offline`, six `*-fixture` rows and six `*-passive` rows. The offline row represents nine synthetic cases, not nine additional report rows.

| Scenario | Fixture | Passive experiment |
| --- | --- | --- |
| Complete response | PASS: synthetic HTTP 200 | PASS: bounded HTTP observation |
| Split response | PASS: synthetic HTTP 200 | PASS: bounded reassembly/HTTP observation |
| Incomplete response | PASS: expected incomplete result | PASS: expected handling; not a complete successful response |
| HTTP 500 | PASS: synthetic status 500 | PASS: observed error status; not application health |
| No response | PASS: expected timeout | PASS: expected handling; not proof of fault cause |
| Serial session | PASS: serial requests on one connection | PASS: bounded session observation |

The final `project-readiness` row remains **INCONCLUSIVE**: no PID attribution, repair, HTTPS/HTTP2 or full-project verification.

The capture-only experiment was scoped to synthetic IPv4 loopback traffic on `lo0`, `127.0.0.1`, TCP port `18765`. The earlier `CAPTURE_UNAVAILABLE` BPF permission denial and accidental consent refusal were followed by the successful scoped rerun. Do not describe packet capture as generally available without permission, broaden access, or change system permissions based on this result. Permission to observe is not permission to terminate or repair a process. No request-body replay or payload archive is introduced.

## Consequences for work

- RS-03 can reference the reported tooling feasibility and SDK correction; full-project native build/run verification remains separate.
- RS-11 can reference passive HTTP feasibility, including expected negative cases. Packet-time ownership, per-request timeout classification and implementation acceptance remain open.
- RS-14 can reference the GUI smoke test, not integrated GUI completion.
- RS-15/RS-16/RS-24 can reference bounded lifecycle and SQLite tests, not an approved end-to-end recovery or booking-rule proof.
- RS-20/RS-22 still require actual application integration and cross-platform acceptance. RS-26 deployment/CI is not established by these local tests.
- RS-27 and the work-phases tracker should link this evidence without marking implementation or acceptance tasks complete.

The approved owners, reviewers, dependencies and design remain unchanged. Historical P2 and frozen P3-R2 artifacts retain their original bytes; use this companion for later evidence rather than rewriting preparation-time statements inside those snapshots.
