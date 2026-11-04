"""Regenerate the sequence diagram and UI sketch with Pillow.
Run from any directory. Pillow and DejaVu Sans fonts are required.
All screen values are illustrative, not measured project results.
"""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parents[2]
FONT = '/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf'
BOLD = '/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf'
NAVY = '#17324d'
INK = '#243d53'
MUTED = '#536c82'
LINE = '#b5c4d2'

def font(size, bold=False):
    return ImageFont.truetype(BOLD if bold else FONT, size)

def text(draw, xy, value, size=25, fill=INK, bold=False, anchor=None):
    draw.text(xy, value, font=font(size,bold), fill=fill, anchor=anchor)

def box(draw, xy, fill='white', outline=LINE, radius=12, width=2):
    draw.rounded_rectangle(xy, radius=radius, fill=fill, outline=outline, width=width)

def arrow(draw, a, b, y, label, dashed=False, size=21):
    if dashed:
        lo, hi = sorted([a,b])
        for x in range(int(lo),int(hi),13):
            draw.line((x,y,min(x+7,hi),y),fill=MUTED,width=2)
    else:
        draw.line((a,y,b,y),fill=NAVY,width=3)
    sign = 1 if b > a else -1
    if dashed:
        draw.line((b-sign*12,y-7,b,y,b-sign*12,y+7),fill=MUTED,width=2)
    else:
        draw.polygon([(b,y),(b-sign*13,y-7),(b-sign*13,y+7)],fill=NAVY)
    width = draw.textlength(label,font=font(size))
    tx=(a+b)/2-width/2
    draw.rectangle((tx-5,y-33,tx+width+5,y-5),fill='white')
    text(draw,(tx,y-33),label,size=size)

def sequence():
    im=Image.new('RGB',(2380,1800),'white'); d=ImageDraw.Draw(im)
    text(d,(65,32),'Rivive Shield | Core-controlled incident sequence',38,bold=True)
    text(d,(65,87),"DESIGN - C++ orchestrator owns lifecycle; teammate modules remain separate",24,fill=MUTED)
    names=['Operator','UI / CLI','Orchestrator','Diagnosis','Executor','Demo target','Verifier','SQLite store']
    xs=[135+i*300 for i in range(8)]
    for x,name in zip(xs,names):
        box(d,(x-108,142,x+108,205),fill='#dceaf7' if name=='Orchestrator' else '#eef4fa')
        text(d,(x,174),name,23,bold=True,anchor='mm')
        for y in range(205,1715,17):d.line((x,y,x,y+9),fill=LINE,width=2)
    def a(i,j,y,l,ret=False):arrow(d,xs[i],xs[j],y,l,ret,size=23)
    a(1,2,255,'Open detected incident')
    a(2,7,310,'Load incident, evidence and current revision')
    a(2,3,365,'Request bounded diagnostic plan')
    a(3,5,420,'Approved read-only checks')
    a(5,3,475,'Observed evidence',True)
    a(3,2,530,'Validated plan or unresolved result',True)
    a(2,7,585,'Save AwaitingApproval + plan revision')
    a(2,1,640,'Display plan and authoritative state',True)
    d.rectangle((30,680,2345,1715),outline=MUTED,width=2)
    d.rectangle((30,680,760,728),fill='#e7eef5',outline=MUTED,width=2)
    text(d,(48,691),'alt [supported plan and current operator approval]',23,bold=True)
    a(0,1,775,'Approve displayed plan')
    a(1,2,830,'Approval event: plan ID / revision')
    a(2,7,885,'Save Repairing + approved attempt BEFORE dispatch')
    a(7,2,940,'Saved; otherwise stop without mutation',True)
    a(2,4,995,'Dispatch once for this approved attempt')
    a(4,5,1050,'Registered command after precondition check')
    a(5,4,1105,'Execution evidence',True)
    a(4,2,1160,'Known success / failure / uncertain result',True)
    a(2,6,1215,'After known success: verify independently')
    a(6,5,1270,'Health, normal use, preservation and invariant')
    a(5,6,1325,'Observed results',True)
    a(6,2,1380,'Pass / fail with evidence',True)
    a(2,7,1435,'Persist final state and full audit record')
    a(2,1,1490,'Success only after execution, checks AND save pass',True)
    for x in range(30,2345,16):d.line((x,1540,x+8,1540),fill=MUTED,width=2)
    text(d,(48,1556),'[unsupported, declined, stale or interrupted outcome]',23,bold=True)
    a(2,7,1620,'Record non-success state when storage is available')
    a(2,1,1690,'Show reason / NeedsReview; no blind repair replay',True)
    text(d,(65,1755),'Serialized events reject duplicates and late callbacks. Uncertain execution is reconciled, not automatically retried.',22,fill=MUTED)
    im.save(ROOT/'diagrams'/'sequence.png')

def ui():
    im=Image.new('RGB',(1800,1270),'#f4f7fb'); d=ImageDraw.Draw(im)
    d.rectangle((0,0,1800,115),fill=NAVY)
    text(d,(50,35),'Rivive Shield',42,fill='white',bold=True)
    text(d,(1735,51),'OPERATOR DASHBOARD',23,fill='#cedfeb',anchor='ra')
    box(d,(40,145,1760,215),fill='#fff2cd',outline='#d8b660')
    text(d,(65,167),'WIREFRAME | Example data only. No live service or measured results.',25,bold=True)
    box(d,(40,245,535,1190))
    text(d,(65,275),'Incident',30,bold=True)
    text(d,(65,330),'DEMO-001  |  Illustrative ID',22,fill=MUTED)
    box(d,(65,375,505,430),fill='#e6eef7',outline='#afc3d9')
    text(d,(85,391),'Awaiting operator decision',23,bold=True)
    text(d,(65,475),'Observed effect',24,bold=True)
    text(d,(65,525),'Booking service unavailable',23)
    text(d,(65,575),'Evidence',24,bold=True)
    text(d,(65,625),'E1  Health check: failed',22)
    text(d,(65,670),'E2  Process status: stopped',22)
    text(d,(65,715),'E3  Recent events: inspect',22)
    text(d,(65,790),'Cause status',24,bold=True)
    text(d,(65,840),'Candidate: stopped service',22)
    text(d,(65,885),'Explanation must cite evidence.',21,fill=MUTED)
    text(d,(65,965),'Scope',24,bold=True)
    text(d,(65,1015),'Fixed isolated demo target',22)
    text(d,(65,1060),'Fictitious booking data only',22)
    text(d,(65,1120),'No device or attacker identity',21,fill=MUTED)
    box(d,(565,245,1760,720))
    text(d,(600,275),'Proposed repair',30,bold=True)
    text(d,(600,335),'Registered action: restart the designated demo service',25)
    text(d,(600,385),'Expected effect: restore service availability',23)
    text(d,(600,435),'Preconditions: approved plan; fixed target; unchanged state',23)
    text(d,(600,485),'Evidence: E1 and E2. No arbitrary shell command.',23)
    box(d,(600,550,1100,620),fill='#205d85',outline='#205d85')
    text(d,(850,585),'Approve this plan',27,fill='white',bold=True,anchor='mm')
    box(d,(1130,550,1550,620),fill='white',outline='#8197ad')
    text(d,(1340,585),'Reject / leave unresolved',24,bold=True,anchor='mm')
    text(d,(600,660),'No repair has run. Approval is separate from diagnosis.',23,fill=MUTED)
    box(d,(565,750,1760,1190))
    text(d,(600,780),'Independent verification',30,bold=True)
    labels=['Service health','Legitimate booking','Unaffected data preserved','Booking invariant','Incident record saved']
    for i,label in enumerate(labels):
        y=850+i*53
        text(d,(610,y),label,24)
        text(d,(1705,y),'NOT RUN',22,fill=MUTED,anchor='ra')
    text(d,(600,1140),'Recovery is successful only after every required check passes.',23,bold=True)
    text(d,(45,1220),'UI requirement: distinguish observed evidence, proposed cause, human decision and verified result.',23,fill=MUTED)
    im.save(ROOT/'ui-sketch.png')

if __name__=='__main__':
    sequence(); ui()
