"""Synthetic SQL contract fixture; not a C++ repository, restart or race test."""
from pathlib import Path
import json
import sqlite3
import tempfile

base = Path(__file__).resolve().parents[1]
schema = (base / 'schemas/incident_proposed.sql').read_text()
fixture = """
INSERT INTO incidents VALUES('i','target',1,1,'AwaitingApproval','fixture',1,NULL,0);
INSERT INTO plans VALUES('i','p',1,'ForceStopOwnedTarget','{}',1,100,'fixture');
INSERT INTO approvals VALUES('a','i','p',1,1,'Approve',2,'fixture');
INSERT INTO attempts VALUES('t','i','p',1,'a','Prepared',3,NULL);
"""
claim = "INSERT INTO dispatch_claims VALUES('t',1,5)"
close = "UPDATE incidents SET record_version=record_version+1,status='Unresolved',closed_at_ms=7,unresolved_effects=0 WHERE incident_id='i'"
receipt_query = """
SELECT t.status, x.reason_code, x.terminated_at_ms, x.expected_record_version+1, h.canonical_record_json
FROM attempts t JOIN prepared_attempt_terminations x USING(attempt_id)
JOIN record_history h ON h.incident_id=t.incident_id AND h.record_version=x.expected_record_version+1
"""
def open_db(path):
    c = sqlite3.connect(path)
    c.execute('PRAGMA foreign_keys=ON')
    return c
def seed(path):
    c = open_db(path)
    c.executescript(schema + fixture)
    return c
def termination(c, version=1):
    c.execute("INSERT INTO prepared_attempt_terminations VALUES('t',5,?,'PlanCancelled')", (version,))
def rejected(call, contains):
    try:
        call()
    except sqlite3.IntegrityError as e:
        assert contains in str(e), e
    else:
        raise AssertionError(contains)
with tempfile.TemporaryDirectory() as d:
    db = Path(d) / 'test.sqlite'
    c = seed(db)
    rejected(lambda: c.execute(close), 'unresolved_attempt_blocks_closure')
    rejected(lambda: termination(c, 2), 'termination_requires_unclaimed')
    c.rollback()
    c.execute('BEGIN IMMEDIATE')
    termination(c)
    v = c.execute("SELECT status,record_version,incident_revision,unresolved_effects FROM incidents").fetchone()
    assert v == ('AwaitingApproval', 2, 1, 0)
    snap = json.dumps({'recordVersion': 2, 'preparedTerminations': [
        {'attemptId': 't', 'terminatedAtMs': 5, 'reasonCode': 'PlanCancelled'}]})
    c.execute("INSERT INTO record_history VALUES('i',2,5,?)", (snap,))
    c.commit()
    c.close()
    c = open_db(db)
    assert c.execute(receipt_query).fetchone() == ('Prepared', 'PlanCancelled', 5, 2, snap)
    rejected(lambda: c.execute(claim), 'claim_rejected')
    rejected(lambda: c.execute("DELETE FROM prepared_attempt_terminations"), 'immutable_history')
    c.execute(close)
    c.close()
    c = seed(':memory:')
    c.execute(claim)
    rejected(lambda: termination(c, 2), 'termination_requires_unclaimed')
    rejected(lambda: c.execute(close), 'unresolved_attempt_blocks_closure')
    c.close()
    c = seed(':memory:')
    c.execute('BEGIN IMMEDIATE')
    termination(c)
    c.execute("INSERT INTO record_history VALUES('i',2,5,'{}')")
    rejected(lambda: c.execute("INSERT INTO record_history VALUES('i',2,5,'{}')"), 'UNIQUE')
    c.rollback()
    assert c.execute("SELECT record_version FROM incidents").fetchone() == (1,)
    assert c.execute("SELECT COUNT(*) FROM prepared_attempt_terminations").fetchone() == (0,)
    assert c.execute("SELECT COUNT(*) FROM record_history").fetchone() == (0,)
    c.close()
print('PASS synthetic SQLite: stale version, both sequential orderings, reopen history, closure, and rollback')
