"""Fresh temporary SQLite-only smoke checks. No real project DB is opened.
Not an implementation, crash-durability test, or proof of exactly-once OS effects.
"""
import pathlib
import sqlite3
import tempfile

schema = pathlib.Path(__file__).resolve().parents[1] / 'schemas/incident_proposed.sql'
passed = 0

def check(name, predicate):
    global passed
    assert predicate, name
    passed += 1
    print('PASS:', name)

def rejected(name, db, sql, params=()):
    try:
        db.execute(sql, params)
    except sqlite3.IntegrityError:
        check(name, True)
    else:
        raise AssertionError('Expected rejection: ' + name)

def prepare(db, ident='i1', target='t1', expires=1000, decision='Approve'):
    db.execute('INSERT INTO incidents VALUES (?,?,1,1,\'AwaitingApproval\',\'ProcessExited\',0,NULL,0)',(ident,target))
    db.execute('INSERT INTO plans VALUES (?,?,1,\'StartStoppedOwnedTarget\',\'{}\',0,?,\'test\')',(ident,'p'+ident,expires))
    db.execute('INSERT INTO approvals VALUES (?,?,?,1,1,?,1,\'test\')',('a'+ident,ident,'p'+ident,decision))
    if decision=='Approve':
        db.execute('INSERT INTO attempts VALUES (?,?,?,1,?,\'Prepared\',2,NULL)',('x'+ident,ident,'p'+ident,'a'+ident))

with tempfile.TemporaryDirectory(prefix='rivive_incident_schema_') as temp:
    path = str(pathlib.Path(temp)/'test.db')
    db=sqlite3.connect(path, isolation_level=None)
    db.executescript(schema.read_text())
    other=sqlite3.connect(path, isolation_level=None)
    other.execute('PRAGMA foreign_keys=ON')
    prepare(db)
    rejected('one active incident per target', db, "INSERT INTO incidents VALUES ('i2','t1',1,1,'Detected','Unknown',0,NULL,0)")
    rejected('stale record version claim', db, "INSERT INTO dispatch_claims VALUES ('xi1',99,3)")
    db.execute('BEGIN IMMEDIATE')
    db.execute("INSERT INTO dispatch_claims VALUES ('xi1',1,3)")
    db.execute("INSERT INTO record_history VALUES ('i1',2,3,'{}')")
    db.execute('COMMIT')
    check('atomic dispatch updates attempt and incident', db.execute("SELECT t.status,i.status,i.record_version,i.unresolved_effects FROM attempts t JOIN incidents i USING(incident_id) WHERE t.attempt_id='xi1'").fetchone()==('Dispatching','Executing',2,1))
    rejected('second connection cannot claim same attempt',other,"INSERT INTO dispatch_claims VALUES ('xi1',2,4)")
    rejected('pending side effects forbid closure', db, "UPDATE incidents SET status='Recovered',closed_at_ms=5,record_version=3 WHERE incident_id='i1'")
    rejected('clearing uncertainty cannot bypass in-flight attempt',db,"UPDATE incidents SET status='Recovered',closed_at_ms=5,unresolved_effects=0,record_version=3 WHERE incident_id='i1'")
    db.execute("UPDATE attempts SET status='Uncertain' WHERE attempt_id='xi1'")
    rejected('uncertain attempt cannot silently become succeeded',db,"UPDATE attempts SET status='Succeeded' WHERE attempt_id='xi1'")
    rejected('uncertain attempt cannot be closed',db,"UPDATE incidents SET status='Unresolved',closed_at_ms=5,unresolved_effects=0,record_version=3 WHERE incident_id='i1'")
    rejected('immutable approval',db,"UPDATE approvals SET decision='Reject' WHERE approval_id='ai1'")
    rejected('immutable history',db,"DELETE FROM record_history")
    rejected('history cannot be deleted via incident',db,"DELETE FROM incidents WHERE incident_id='i1'")
    rejected('plan cannot cross incidents',db,"INSERT INTO approvals VALUES ('bad','absent','pi1',1,1,'Approve',1,'test')")
    prepare(db,'expired','t2',expires=3)
    rejected('expired plan claim',db,"INSERT INTO dispatch_claims VALUES ('xexpired',1,3)")
    prepare(db,'rejected','t3',decision='Reject')
    rejected('rejection cannot authorize prepared attempt',db,"INSERT INTO attempts VALUES ('bad','rejected','prejected',1,'arejected','Prepared',2,NULL)")
    prepare(db,'rollback','t4')
    db.execute('BEGIN IMMEDIATE')
    db.execute("INSERT INTO dispatch_claims VALUES ('xrollback',1,3)")
    db.execute('ROLLBACK')
    check('rolled-back claim consumes no authority',db.execute("SELECT status FROM attempts WHERE attempt_id='xrollback'").fetchone()==('Prepared',) and db.execute("SELECT count(*) FROM dispatch_claims WHERE attempt_id='xrollback'").fetchone()==(0,))
    db.execute("INSERT INTO accepted_events VALUES ('e1','i1','hash1',1)")
    rejected('event ID cannot be reused with changed payload',db,"INSERT INTO accepted_events VALUES ('e1','i1','hash2',2)")
    check('foreign keys valid',not db.execute('PRAGMA foreign_key_check').fetchall())
    check('structural integrity valid',db.execute('PRAGMA integrity_check').fetchone()==('ok',))
    other.close(); db.close()
print(f'{passed} checks passed. Temporary test DB removed.')
print('LIMIT: single-dispatch durable claim only, not exactly-once external execution; no power-loss/concurrent-writer stress.')
