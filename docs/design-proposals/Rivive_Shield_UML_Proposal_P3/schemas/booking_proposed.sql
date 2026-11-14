-- P3 TECHNICAL PROPOSAL: new test database only. No migration or runtime PRAGMA policy.
CREATE TABLE rooms (id TEXT PRIMARY KEY NOT NULL, name TEXT NOT NULL);
CREATE TABLE bookings (
 request_id TEXT PRIMARY KEY NOT NULL,
 room_id TEXT NOT NULL REFERENCES rooms(id),
 starts_at_ms INTEGER NOT NULL,
 ends_at_ms INTEGER NOT NULL,
 synthetic_label TEXT NOT NULL,
 created_at_ms INTEGER NOT NULL,
 revision INTEGER NOT NULL CHECK(revision > 0),
 CHECK(starts_at_ms < ends_at_ms)
);
CREATE INDEX booking_intervals ON bookings(room_id, starts_at_ms, ends_at_ms);
CREATE TRIGGER prevent_booking_overlap BEFORE INSERT ON bookings
WHEN EXISTS (SELECT 1 FROM bookings WHERE room_id=NEW.room_id
 AND starts_at_ms < NEW.ends_at_ms AND ends_at_ms > NEW.starts_at_ms)
BEGIN SELECT RAISE(ABORT,'booking_overlap'); END;
-- No update/cancel route in this milestone. Writes only through repository transaction.
-- BEGIN IMMEDIATE is a proposed implementation choice, not a user-approved journal policy.
-- Inside transaction: compare existing request_id first; identical -> old result; changed -> conflict.
-- Otherwise INSERT and COMMIT. Any precommit failure -> explicit rollback if transaction active.
-- SQLITE_BUSY gets bounded handling; do not reinterpret uncertain COMMIT as safe to duplicate.
