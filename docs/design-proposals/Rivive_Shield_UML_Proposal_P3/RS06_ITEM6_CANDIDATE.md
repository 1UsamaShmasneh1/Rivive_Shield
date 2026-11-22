# RS-06 Item 6: design candidate

This branch reconciles the Item 1 review archive (revision 2) with the final Item 5 transition clarification (archive revision 2). It is a design-only candidate, not a final contract freeze, implementation, or migration. The [frozen P3-R2 baseline](https://github.com/1UsamaShmasneh1/Rivive_Shield/tree/406ab2d74e72aa53fe066222dc06d407c6e09980/docs/design-proposals/Rivive_Shield_UML_Proposal_P3) remains available unchanged at that commit. Historical filenames and review text do not describe the status of the whole candidate.

- Item 1 adds one append-only termination value and its repository/history contract. An unclaimed prepared attempt can be retired without erasing history or dispatching it. The committed dispatch claim is the only durable owner of the transition to Executing.
- Item 5 keeps the nine states and the same 24 transition edges. It distinguishes pre-claim guards from the durable claim result and requires confirmed termination before applicable plan invalidation, including an AwaitingApproval manual stop proposal. The final clarification was owner-directed, not independently rereviewed.
- Only affected generated catalogs and UML artifacts have been brought into alignment. Roles, booking routes, booking schema, API wire files and application source are unchanged. The separately reviewed Item 3 wire refinements are not published by this candidate.
- Local verification is limited to Linux GCC C++17 syntax/static traits, exact declaration/catalog/UML consistency, structural state-policy checks, and synthetic SQLite/JSON design fixtures. No linking, native Windows or macOS check, runtime state transition, concurrency stress, crash durability, migration, or application behavior was verified by this candidate.

Publication for review does not close RS-06 or approve the candidate for implementation.
