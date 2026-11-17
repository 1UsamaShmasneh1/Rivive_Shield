# Rivive Shield: Updated Project Proposal

**Date:** 17 November 2026  
**Status:** Updated proposal approved by the team leader for publication; based on the [P3-R2 working design](design-proposals/P3_STATUS.md). This is not a claim of completed implementation or tested acceptance.

This document is separate from the [historical proposal](01-proposal.md) and preserves the earlier documents unchanged.

## 1. Problem and need

When an application fails, an error message alone does not establish the cause, identify a safe action or prove that the action worked. The operator of an educational experiment needs to connect evidence with the application's state, understand the available actions, control recovery and verify the outcome.

## 2. Proposed solution

**Rivive Shield** is a C++ desktop application that monitors an independent study-room booking application within an authorized, bounded environment. It uses observations and predefined rules to diagnose supported conditions, presents bounded recovery actions for human approval, then verifies the outcome and records the incident. When evidence is insufficient, it reports an inconclusive result instead of asserting a cause.

The experiment consists of a wxWidgets desktop interface and an independent booking application with a C++ backend, SQLite database and React interface. Development starts locally. This describes the adopted design, not a completed integrated application.

## 3. Users and stakeholders

- **Operator:** needs clear evidence, control over approval and a reviewable outcome.
- **Booking application user:** needs to try booking with synthetic data.
- **Development team:** needs clear tasks and contracts, and repeatable tests.
- **Lecturer:** needs traceability of requirements, design, team decisions and individual contributions.

## 4. Current approach and alternatives

Failures can be handled manually by inspecting logs and service status, taking an action and retesting. Specialized monitoring, alerting and automation tools also exist. Our project does not aim to replace them generally. Instead, it proposes a small educational experiment that brings evidence, approval, recovery and verification into one workflow. The benefit to users is a hypothesis to test, not an established result.

## 5. Goal and measurement

The goal is to help the operator make an evidence-supported decision and verify its outcome without unauthorized action or loss of valid data.

After the RS-05 scenarios are approved, the team plans **at least 10 documented runs**, covering normal, failure and inconclusive cases. The expected outcome of each run will be defined beforehand and compared with the actual outcome. The targets are zero state-changing actions without the required approval, zero duplicate executions of the same approval, and zero loss of valid data during testing. Timing will be measured without setting a performance threshold before initial measurements.

The measurement plan and allocation of runs will be finalized **by 7 December 2026, or earlier if team progress permits**. The application evaluation date depends on development progress. These are future targets and plans, not achieved results.

## 6. First version and boundaries

The first version includes an isolated booking application, SQLite storage, bounded monitoring, predefined diagnostic and recovery rules, human approval and independent verification. The project targets Windows and macOS; each platform will be tested independently.

Booking cancellation and additional scenarios are deferred. Oracle deployment follows a successful local version and a review of the remote-operation design. The scope excludes monitoring unauthorized systems, decrypting HTTPS, executing arbitrary commands and arbitrarily modifying bookings as a repair.

## 7. Feasibility, constraints and risks

Bounded feasibility experiments exist on Windows and macOS, and the initial build setup succeeded on Usama's computer. These results do not establish success of the integrated application or compatibility with the rest of the team's computers.

Risks include incomplete observation evidence, differences in tools and permissions across platforms, identifying the correct process, protecting data and limited time. Insufficient evidence will be treated as inconclusive, and actions will be constrained by appropriate approval and verification. Effort and cost estimates and remote-operation details still require review.

## 8. Value and AI use

The project is currently educational and noncommercial. Its proposed value is a repeatable experiment that explains safe handling of failures. No revenue model has been established.

The first version does not run an AI model. Its future usefulness may be studied without a commitment to add one. For AI tools used during development, the team will document actual use, tool and model, human review and corrections, using synthetic or public data in accordance with the course instructions.

## 9. Open questions

- How will runs be allocated, and what is the expected outcome of each scenario after RS-05 approval?
- What time and size limits are appropriate after measurement?
- Can the operator understand the evidence, action and verification outcome in a usability exercise?
- What remote-operation design, cost and permissions are appropriate when moving to Oracle?

**Formatting check pending:** The final rendered submission must be checked against the three-page limit. This Markdown document has not been verified as a paginated PDF.
