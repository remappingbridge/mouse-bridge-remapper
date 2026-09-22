"""Compile the canonical Markdown literals; --check rejects stale generated code."""
from pathlib import Path
import re,json,sys
root=Path(__file__).resolve().parents[1]
spec=(root/'docs/manual/06-screen-reference.md').read_text()
items=[]
for name,body in re.findall(r'^## ([\w-]+)\n(.*?)(?=^## |\Z)',spec,re.M|re.S):
    match=re.search(r'```text\n(.*?)\n```',body,re.S)
    if not match: continue
    rows=match[1].split('\n')
    assert len(rows)<=9,(name,rows)
    rows+=['']*(9-len(rows))
    # Explicit frozen 1-based columns prevail over illustrative Markdown spacing.
    if name in ('searching-first','first-mouse-connected','learn-the-keys'):
        r=3 if name=='searching-first' else 1
        rows[r]='       JOY UP'; rows[r+1]='  JOY    JOY    JOY'
        # JOY tokens at columns 3/10/17, RIGHT at 16.
        rows[r+1]='  JOY    JOY    JOY'
        rows[r+2]='  LEFT  PRESS  RIGHT'; rows[r+3]='      JOY DOWN'
    assert all(len(x)<=21 for x in rows),(name,rows)
    items.append((name,rows))
assert len(items)==30
h='#pragma once\n#include "mbr/domain/domain.h"\ntypedef enum {\n'+''.join(' MBR_SCREEN_'+n.upper().replace('-','_')+',\n' for n,_ in items)+' MBR_SCREEN_COUNT\n} MbrScreen;\ntypedef struct { const char *id; const char *rows[9]; } MbrScreenTemplate;\nextern const MbrScreenTemplate mbr_screens[MBR_SCREEN_COUNT];\n'
c='#include "mbr/domain/screens.h"\nconst MbrScreenTemplate mbr_screens[MBR_SCREEN_COUNT]={\n'+''.join(' {'+json.dumps(n)+', {'+', '.join(json.dumps(x) for x in rows)+'}},\n' for n,rows in items)+'};\n'
for path,content in [(root/'include/mbr/domain/screens.h',h),(root/'src/domain/screens.c',c)]:
    if '--check' in sys.argv: assert path.read_text()==content,path
    else: path.write_text(content)
