-- P3 TECHNICAL PROPOSAL, not user-approved persistence or migration policy.
-- Fresh DB only; caller enables foreign_keys ON on EVERY connection before transactions.
-- UTC integer milliseconds. Canonical JSON/hashes are generated internally, never trusted UI input.
-- One target_key denotes the reviewed immutable configuration for one target/session lineage.
PRAGMA foreign_keys=ON;
CREATE TABLE incidents (
 incident_id TEXT PRIMARY KEY NOT NULL,
 target_key TEXT NOT NULL,
 record_version INTEGER NOT NULL CHECK(record_version > 0),
 incident_revision INTEGER NOT NULL CHECK(incident_revision > 0),
 status TEXT NOT NULL CHECK(status IN ('Detected','Diagnosing','AwaitingApproval','Executing','Verifying','Recovered','Declined','Unresolved','NeedsReview')),
 kind TEXT NOT NULL,
 detected_at_ms INTEGER NOT NULL,
 closed_at_ms INTEGER,
 unresolved_effects INTEGER NOT NULL DEFAULT 0 CHECK(unresolved_effects IN (0,1)),
 CHECK(closed_at_ms IS NULL OR (status IN ('Recovered','Declined','Unresolved') AND unresolved_effects=0))
);
CREATE UNIQUE INDEX one_active_incident_per_target ON incidents(target_key) WHERE closed_at_ms IS NULL;
CREATE TABLE evidence (
 incident_id TEXT NOT NULL REFERENCES incidents(incident_id),
 evidence_id TEXT NOT NULL,
 observed_at_ms INTEGER NOT NULL,
 kind TEXT NOT NULL,
 metadata_json TEXT NOT NULL,
 PRIMARY KEY(incident_id,evidence_id)
);
CREATE TABLE plans (
 incident_id TEXT NOT NULL REFERENCES incidents(incident_id),
 plan_id TEXT NOT NULL,
 plan_revision INTEGER NOT NULL CHECK(plan_revision > 0),
 action TEXT NOT NULL CHECK(action IN ('StartStoppedOwnedTarget','ForceStopOwnedTarget')),
 target_identity_json TEXT NOT NULL,
 created_at_ms INTEGER NOT NULL,
 expires_at_ms INTEGER NOT NULL CHECK(expires_at_ms > created_at_ms),
 explanation TEXT NOT NULL,
 PRIMARY KEY(incident_id,plan_id,plan_revision)
);
CREATE TABLE plan_evidence (
 incident_id TEXT NOT NULL, plan_id TEXT NOT NULL, plan_revision INTEGER NOT NULL, evidence_id TEXT NOT NULL,
 PRIMARY KEY(incident_id,plan_id,plan_revision,evidence_id),
 FOREIGN KEY(incident_id,plan_id,plan_revision) REFERENCES plans(incident_id,plan_id,plan_revision),
 FOREIGN KEY(incident_id,evidence_id) REFERENCES evidence(incident_id,evidence_id)
);
CREATE TABLE approvals (
 approval_id TEXT PRIMARY KEY NOT NULL,
 incident_id TEXT NOT NULL, plan_id TEXT NOT NULL, plan_revision INTEGER NOT NULL,
 incident_revision INTEGER NOT NULL CHECK(incident_revision > 0),
 decision TEXT NOT NULL CHECK(decision IN ('Approve','Reject')),
 decided_at_ms INTEGER NOT NULL, operator_label TEXT NOT NULL,
 UNIQUE(incident_id,plan_id,plan_revision,approval_id),
 FOREIGN KEY(incident_id,plan_id,plan_revision) REFERENCES plans(incident_id,plan_id,plan_revision)
);
CREATE TABLE attempts (
 attempt_id TEXT PRIMARY KEY NOT NULL,
 incident_id TEXT NOT NULL, plan_id TEXT NOT NULL, plan_revision INTEGER NOT NULL,
 approval_id TEXT NOT NULL UNIQUE,
 status TEXT NOT NULL CHECK(status IN ('Prepared','Dispatching','Succeeded','Failed','Uncertain')),
 prepared_at_ms INTEGER NOT NULL,
 dispatch_claimed_at_ms INTEGER,
 UNIQUE(incident_id,plan_id,plan_revision),
 FOREIGN KEY(incident_id,plan_id,plan_revision,approval_id) REFERENCES approvals(incident_id,plan_id,plan_revision,approval_id),
 CHECK((status='Prepared' AND dispatch_claimed_at_ms IS NULL) OR (status<>'Prepared' AND dispatch_claimed_at_ms IS NOT NULL))
);
CREATE TABLE dispatch_claims (
 attempt_id TEXT PRIMARY KEY NOT NULL REFERENCES attempts(attempt_id),
 expected_record_version INTEGER NOT NULL,
 claimed_at_ms INTEGER NOT NULL
);
CREATE TABLE execution_observations (
 attempt_id TEXT PRIMARY KEY NOT NULL REFERENCES attempts(attempt_id),
 outcome TEXT NOT NULL CHECK(outcome IN ('Applied','KnownFailure','Uncertain')),
 resulting_process_json TEXT, failure_json TEXT,
 finished_at_ms INTEGER NOT NULL
);
CREATE TABLE verification_batches (
 verification_id TEXT PRIMARY KEY NOT NULL,
 attempt_id TEXT NOT NULL REFERENCES attempts(attempt_id),
 checked_at_ms INTEGER NOT NULL,
 process_check TEXT NOT NULL CHECK(process_check IN ('Passed','Failed','Inconclusive','NotRun')),
 database_check TEXT NOT NULL CHECK(database_check IN ('Passed','Failed','Inconclusive','NotRun')),
 functional_check TEXT NOT NULL CHECK(functional_check IN ('Passed','Failed','Inconclusive','NotRun')),
 metadata_json TEXT NOT NULL
);
CREATE TABLE rule_audits (
 audit_id TEXT PRIMARY KEY NOT NULL,
 incident_id TEXT NOT NULL REFERENCES incidents(incident_id),
 ruleset_version TEXT NOT NULL, matched_rule_id TEXT, evaluated_at_ms INTEGER NOT NULL,
 metadata_json TEXT NOT NULL
);
CREATE TABLE accepted_events (
 event_id TEXT PRIMARY KEY NOT NULL,
 incident_id TEXT NOT NULL REFERENCES incidents(incident_id),
 canonical_payload_hash TEXT NOT NULL,
 accepted_at_ms INTEGER NOT NULL
);
-- Full immutable committed aggregate snapshots permit exact reconstruction of old history.
CREATE TABLE record_history (
 incident_id TEXT NOT NULL REFERENCES incidents(incident_id),
 record_version INTEGER NOT NULL,
 committed_at_ms INTEGER NOT NULL,
 canonical_record_json TEXT NOT NULL,
 PRIMARY KEY(incident_id,record_version)
);
CREATE TRIGGER incident_no_identity_change BEFORE UPDATE ON incidents
WHEN NEW.incident_id<>OLD.incident_id OR NEW.target_key<>OLD.target_key OR OLD.closed_at_ms IS NOT NULL
BEGIN SELECT RAISE(ABORT,'closed_or_identity_changed'); END;
CREATE TRIGGER incident_version_cas BEFORE UPDATE ON incidents
WHEN NEW.record_version<>OLD.record_version+1 OR NEW.incident_revision<OLD.incident_revision
BEGIN SELECT RAISE(ABORT,'invalid_record_revision'); END;
CREATE TRIGGER no_uncertain_closure BEFORE UPDATE ON incidents
WHEN NEW.closed_at_ms IS NOT NULL AND EXISTS(
 SELECT 1 FROM attempts WHERE incident_id=OLD.incident_id AND status IN ('Prepared','Dispatching','Uncertain'))
BEGIN SELECT RAISE(ABORT,'unresolved_attempt_blocks_closure'); END;
CREATE TRIGGER attempts_prepare_only BEFORE INSERT ON attempts
WHEN NEW.status<>'Prepared' OR NOT EXISTS(
 SELECT 1 FROM approvals a JOIN incidents i USING(incident_id)
 WHERE a.approval_id=NEW.approval_id AND a.decision='Approve'
 AND i.closed_at_ms IS NULL AND i.status='AwaitingApproval' AND i.unresolved_effects=0
 AND a.incident_revision=i.incident_revision)
BEGIN SELECT RAISE(ABORT,'invalid_prepared_attempt'); END;
CREATE TRIGGER attempt_transition BEFORE UPDATE ON attempts
WHEN NEW.attempt_id<>OLD.attempt_id OR NEW.incident_id<>OLD.incident_id
 OR NEW.plan_id<>OLD.plan_id OR NEW.plan_revision<>OLD.plan_revision OR NEW.approval_id<>OLD.approval_id
 OR NEW.prepared_at_ms<>OLD.prepared_at_ms
 OR NOT ((OLD.status='Prepared' AND NEW.status='Dispatching' AND EXISTS(SELECT 1 FROM dispatch_claims WHERE attempt_id=OLD.attempt_id))
 OR (OLD.status='Dispatching' AND NEW.status IN ('Succeeded','Failed','Uncertain') AND NEW.dispatch_claimed_at_ms=OLD.dispatch_claimed_at_ms))
BEGIN SELECT RAISE(ABORT,'invalid_attempt_transition'); END;
-- INSERT of a claim is the only dispatch acquisition path. BEGIN IMMEDIATE wraps this
-- plus immutable record_history save; COMMIT must succeed BEFORE the external call.
CREATE TRIGGER claim_validate BEFORE INSERT ON dispatch_claims
WHEN NOT EXISTS(
 SELECT 1 FROM attempts t JOIN incidents i USING(incident_id)
 JOIN approvals a ON a.approval_id=t.approval_id
 JOIN plans p ON p.incident_id=t.incident_id AND p.plan_id=t.plan_id AND p.plan_revision=t.plan_revision
 WHERE t.attempt_id=NEW.attempt_id AND t.status='Prepared'
 AND i.record_version=NEW.expected_record_version AND i.status='AwaitingApproval'
 AND i.closed_at_ms IS NULL AND i.unresolved_effects=0
 AND a.decision='Approve' AND a.incident_revision=i.incident_revision
 AND p.expires_at_ms>NEW.claimed_at_ms AND a.decided_at_ms<=NEW.claimed_at_ms
 AND NOT EXISTS(SELECT 1 FROM attempts other WHERE other.incident_id=i.incident_id AND other.status IN ('Dispatching','Uncertain')))
BEGIN SELECT RAISE(ABORT,'claim_rejected'); END;
CREATE TRIGGER claim_apply AFTER INSERT ON dispatch_claims
BEGIN
 UPDATE attempts SET status='Dispatching',dispatch_claimed_at_ms=NEW.claimed_at_ms WHERE attempt_id=NEW.attempt_id;
 UPDATE incidents SET status='Executing',unresolved_effects=1,record_version=record_version+1,incident_revision=incident_revision+1
 WHERE incident_id=(SELECT incident_id FROM attempts WHERE attempt_id=NEW.attempt_id);
END;
-- Uncertain attempts are immutable and prevent closure until a reviewed reconciliation
-- model exists. This deliberately blocks restart/closure, not a silent override.
-- Allowed normal completion is Dispatching->Succeeded/Failed with execution evidence.
-- Repository transaction separately persists that evidence + aggregate history/revisions.
CREATE TRIGGER immutable_evidence_update BEFORE UPDATE ON evidence BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_evidence_delete BEFORE DELETE ON evidence BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_plans_update BEFORE UPDATE ON plans BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_plans_delete BEFORE DELETE ON plans BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_plan_evidence_update BEFORE UPDATE ON plan_evidence BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_plan_evidence_delete BEFORE DELETE ON plan_evidence BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_approvals_update BEFORE UPDATE ON approvals BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_approvals_delete BEFORE DELETE ON approvals BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_dispatch_claims_update BEFORE UPDATE ON dispatch_claims BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_dispatch_claims_delete BEFORE DELETE ON dispatch_claims BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_execution_observations_update BEFORE UPDATE ON execution_observations BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_execution_observations_delete BEFORE DELETE ON execution_observations BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_verification_batches_update BEFORE UPDATE ON verification_batches BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_verification_batches_delete BEFORE DELETE ON verification_batches BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_rule_audits_update BEFORE UPDATE ON rule_audits BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_rule_audits_delete BEFORE DELETE ON rule_audits BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_accepted_events_update BEFORE UPDATE ON accepted_events BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_accepted_events_delete BEFORE DELETE ON accepted_events BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_record_history_update BEFORE UPDATE ON record_history BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER immutable_record_history_delete BEFORE DELETE ON record_history BEGIN SELECT RAISE(ABORT,'immutable_history'); END;
CREATE TRIGGER no_delete_incidents BEFORE DELETE ON incidents BEGIN SELECT RAISE(ABORT,'preserve_history'); END;
CREATE TRIGGER no_delete_attempts BEFORE DELETE ON attempts BEGIN SELECT RAISE(ABORT,'preserve_history'); END;

-- Repository mapping / remaining integration obligations:
-- IncidentRecord maps to incidents + evidence/plans/approvals/audits/attempts/
-- execution_observations/verification_batches/accepted_events. Aggregate snapshots
-- are archival, NOT a competing mutable source of action authority.
-- target_key, closed_at_ms, unresolved_effects, verification_id and audit_id are
-- storage metadata; define generation/configuration canonicalization and ID allocation
-- in the repository codec. Do not add hidden C++ object fields outside approved UML.
-- save() must BEGIN IMMEDIATE, CAS expected record_version, persist aggregate rows and
-- history in one transaction, and report success only after COMMIT. Existing immutable
-- rows are compared for exact equality, not overwritten; same event ID + same canonical
-- payload is an idempotent no-op, different payload is Conflict (never INSERT OR REPLACE).
-- claimDispatch() appends record_history in the SAME transaction as claim insertion;
-- SQLite triggers alone cannot validate the serialized C++ aggregate/metadata format.
-- A failed/uncertain COMMIT blocks dispatch; reload/reconcile, never blindly invoke.
-- Complete execution persistence must match attempt/action/process generation, and must
-- run exhaustive state guards before clearing unresolved_effects or closing. No direct
-- SQL console or GUI writes are an authorized repository path. SQL enums are not the
-- complete state machine, schema does not make arbitrary updates semantically valid.
-- Target handle authority, plan/config identity, JSON limits and metadata-only privacy
-- are mandatory C++ validation. Foreign keys must be enabled on EVERY connection.
-- Uncertain attempts currently remain blocked indefinitely: deliberate conservative
-- minimum. Any later adjudication needs an append-only resolution record and reviewed
-- transitions; it must never erase evidence or reuse this attempt/approval/claim.
-- Schema_version/migrations, retention, power-loss durability and journal policy are
-- not established by this proposal or its SQLite smoke checks.
