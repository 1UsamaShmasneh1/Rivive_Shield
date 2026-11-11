from pathlib import Path
import json,re,html
R=Path(__file__).resolve().parents[1]
h=(R/'contracts/rivive_shield_proposed.hpp').read_text()
x=json.loads((R/'contracts/contract_inventory.json').read_text())
parsed={m.group(2):m.group(0) for m in re.finditer(r'^(struct|class) (\w+)([^;{]*)\{\n(.*?)^};',h,re.M|re.S)}
assert set(parsed)=={c['name'] for c in x['classes']}
for c in x['classes']:
 assert c['declaration']==parsed[c['name']],c['name']
 body=c['body']; visibility='private' if c['kind']=='class' else 'public'; expected=[]
 for line in body.splitlines():
  line=line.strip()
  if not line:continue
  if line in ['public:','private:','protected:']:visibility=line[:-1];continue
  expected.append((visibility,line))
 assert sorted(expected)==sorted((v['visibility'],v['declaration']) for k in ['fields','methods'] for v in c[k]),c['name']
 dot=(R/'diagrams/source/classes'/(c['name']+'.dot')).read_text()
 # Line wrapping adds BR tags; compare whitespace-normalized unescaped text.
 plain=html.unescape(re.sub('<[^>]+>',' ',dot))
 plain=' '.join(plain.split())
 for k in ['fields','methods']:
  for v in c[k]:assert ' '.join(v['declaration'].split()) in plain,(c['name'],v['declaration'])
 assert (R/'diagrams/png/classes'/(c['name']+'.png')).is_file()
assert set(p.stem for p in (R/'diagrams/source/classes').glob('*.dot'))==set(parsed)
assert not re.search(r'\b(AiProvider|RuntimeAiProvider|AiRequest|AiResponse|AiAudit|AiConfiguration|ToolRequest|DiagnosticToolId)\b',h)
print('PASS: '+str(len(parsed))+' types; header/inventory/member visibility/UML field and method parity; no obsolete AI protocol.')
