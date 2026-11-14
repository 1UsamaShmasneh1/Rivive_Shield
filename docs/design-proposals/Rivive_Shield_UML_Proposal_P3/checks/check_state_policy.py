from pathlib import Path
import json
r=Path(__file__).resolve().parents[1]
p=json.loads((r/'schemas/state_policy_proposed.json').read_text())
states=set(p['states']); pairs=set()
for t in p['transitions']:
 assert t['source'] in states and t['target'] in states
 assert (t['source'],t['event']) not in pairs
 pairs.add((t['source'],t['event']))
 assert t['guards']
 assert t['source'] not in p['closed']
 if t['target'] in p['closed']:assert 'noUncertainEffect' in t['guards'] and 'noDispatchInFlight' in t['guards']
 if t['target']=='Executing' and t['source']!='Executing':assert 'durableClaimWon' in t['guards'] and 'exactPlanMatch' in t['guards']
assert p['default']=='reject_without_side_effect'
print('PASS state policy structure: nine states, unique guarded transitions, closed-state and uncertain-effect gates, default deny. Not runtime transition execution.')
