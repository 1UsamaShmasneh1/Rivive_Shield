from pathlib import Path
import sqlite3,json,jsonschema
from datetime import datetime
r=Path(__file__).resolve().parents[1]
schema=json.loads((r/'schemas/booking_request.schema.json').read_text())
jsonschema.Draft202012Validator.check_schema(schema)
checker=jsonschema.FormatChecker()
@checker.checks('date-time', raises=(ValueError,TypeError))
def check_time(value):
    if not isinstance(value,str) or not value.endswith('Z'):return False
    datetime.fromisoformat(value[:-1]+'+00:00')
    return 'T' in value
v=jsonschema.Draft202012Validator(schema,format_checker=checker)
p={'requestId':'q','roomId':'r','startsAt':'2026-11-13T10:00:00Z','endsAt':'2026-11-13T11:00:00Z','label':'test'}
v.validate(p)
assert list(v.iter_errors(dict(p,extra='no')))
assert list(v.iter_errors(dict(p,startsAt='invalidZ')))
c=sqlite3.connect(':memory:');c.execute('PRAGMA foreign_keys=ON');c.executescript((r/'schemas/booking_proposed.sql').read_text());c.execute("insert into rooms values('r','Room')");c.execute("insert into bookings values('1','r',10,20,'a',1,1)");c.execute("insert into bookings values('2','r',20,30,'b',1,2)")
try:c.execute("insert into bookings values('3','r',15,25,'c',1,3)");raise AssertionError('overlap accepted')
except sqlite3.IntegrityError:pass
assert c.execute('select count(*) from bookings').fetchone()[0]==2
# Demonstrates ABORT statement failure did not auto-rollback prior writes.
c.rollback();assert c.execute('select count(*) from bookings').fetchone()[0]==0
print('PASS JSON schema meta/format/additional-fields; SQL adjacency/overlap/explicit rollback smoke. Not concurrent/crash/API implementation tests.')
