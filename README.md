# Rivive Shield

Rivive Shield is a software-development course project for demonstrating how a desktop controller can detect and help recover from supported failures in an independent study-room booking application. It is designed for the demonstration operator and course participants working with fictitious data.

The booking application serves users through a React frontend and a C++ backend. The separate Windows desktop controller observes the designated local application, evaluates evidence using predefined rules, requests explicit operator approval for a bounded recovery action, and verifies the outcome independently. The prototype does not run an AI model.

## Midterm submission: Proposal, Requirements and System Design

**Status: In preparation.** This is a documentation submission. The final report, verified artifact links and submission commit have not yet been designated.

| Material | Current reference or status |
| --- | --- |
| Unified midterm report | In preparation |
| Proposal and scope | [English working proposal, 17 November 2026](docs/01-proposal-updated-2026-11-17.md) |
| Requirements | User stories, acceptance cases and worked use case in preparation |
| System design | [Approved P3-R2 design baseline](docs/design-proposals/P3_STATUS.md); midterm design section in preparation |
| UML diagrams and main-screen sketch | Current submission artifacts to be linked once reviewed |
| Backlog and delivery checklist | [Trello backlog](https://trello.com/b/GeLevegk/riviveshield) · [Midterm delivery card](https://trello.com/c/89GvPD42) |

After the team reviews the package, each member submits the **same exact repository commit URL** with the names of actual contributors through the individual course submission box. A link to this README, a branch or a tag does not replace the commit URL.

## Project status

- **Design baseline:** P3-R2 is the adopted design. The [reviewed RS-06 contract consolidation](https://github.com/1UsamaShmasneh1/Rivive_Shield/pull/3) is merged. The [RS-07 architecture review](https://trello.com/c/4xui3soD) remains in progress.
- **Verified so far:** [RS-03](https://trello.com/c/ICpHY3nI) records basic configuration, build and launch checks for the setup workspace. Separate localhost experiments show bounded feasibility; they do not verify integrated booking, recovery or deployment.
- **Current platform:** Local development and acceptance target Windows x64. Prior macOS checks are historical. Oracle hosting and GitHub Actions verification remain separate final-project work.

## Windows x64 workstation setup

1. Install **Desktop development with C++** in the full Visual Studio IDE, including MSVC x64, a Windows SDK, CMake and Ninja. Install and bootstrap vcpkg if needed.
2. Open `RiviveShieldWorkspace/` as a folder. Copy `CMakeUserPresets.example.json` to `CMakeUserPresets.json` and set the local vcpkg toolchain path if it differs from the example. Keep the `x64-windows` triplet. Do not commit the personal preset or build outputs.
3. Select `team-x64-debug`, configure and build, then launch `RiviveShield` and `BookingBackend` separately. This checks the setup targets, not an integrated application. See [RS-03](https://trello.com/c/ICpHY3nI) for the recorded setup evidence.

## Team and planning

**Course:** Software Development Methods  
**Team lead:** Usama Shmasneh  
**Team:** Samy Mansor, Ameen Abd Alkader, Shehab Shehab, Yazan Ganem and Rawaa Hagrs.

See the [work allocation and dependency index](docs/backlog-seed.md) and the [live Trello board](https://trello.com/b/GeLevegk/riviveshield) for current ownership and progress.

## Design references and history

- **Current design:** [P3-R2 adoption status](docs/design-proposals/P3_STATUS.md) · [Exact C++ declarations](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/contracts/p3_draft.hpp) · [Behavior and review gates](docs/design-proposals/Rivive_Shield_UML_Proposal_P3/docs/BEHAVIOR_AND_GATES.md).
- **Historical submissions and evidence:** [4 November first-submission report](docs/first-submission-report.md) · [Earlier design packages](docs/design-proposals/README.md) · [Previous full README and evidence index](https://github.com/1UsamaShmasneh1/Rivive_Shield/blob/81ba5c3a6928a6c2dd46c1683e8e697e3e645716/README.md). These references preserve the earlier record without mixing it into the current submission.
