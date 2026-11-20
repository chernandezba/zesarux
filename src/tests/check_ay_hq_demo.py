#!/usr/bin/env python3
"""Compare every CPU/RAM/video frame of a Pentagon demo with legacy and HQ AY.

Builds a separate instrumented test executable. The production binary is unchanged.
The probe removes wall-clock waits only; Z80 cycles/interrupt generation are retained.
Requires an already configured and built macOS checkout.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def digest(p):
    return hashlib.sha256(Path(p).read_bytes()).hexdigest()

def analyze(old, new, sym, expected):
    symbols = {m[1]: int(m[2], 16) for m in re.finditer(r'^(\S+): EQU 0x([\da-fA-F]+)', sym.read_text(), re.M)}
    count = 0
    transitions = []
    prev = None
    for a, b in zip(old.open(), new.open(), strict=True):
        count += 1
        if a != b:
            fields_a, fields_b = a.split(), b.split()
            fields = [i for i, (x, y) in enumerate(zip(fields_a, fields_b)) if x != y]
            raise AssertionError(f'Frame {count}: CPU/RAM/video/kernel differ, fields {fields}')
        data = bytes.fromhex(a.split()[11])
        def byte(k):
            return data[symbols[k]-0x8000]
        def word(k):
            n = symbols[k]-0x8000
            return data[n] | (data[n+1] << 8)
        state = dict(frame=count, isr_frame=word('frames'), music_on=byte('music_on'),
                     music_bank=byte('music_bank'), left=word('music_left'), total=word('music_total'))
        if not prev or (state['music_on'], state['music_bank']) != (prev['music_on'], prev['music_bank']):
            transitions.append(state)
        if prev and state['music_on'] and prev['music_on'] and state['music_bank']==prev['music_bank']:
            assert prev['left']-state['left']==1, f'Missing/duplicate/restarted music tick at frame {count}'
            assert (state['isr_frame']-prev['isr_frame']) & 65535 == 1, f'Missing ISR at frame {count}'
        prev = state
    assert count == expected, (count, expected)
    assert any(x['music_on'] for x in transitions), 'Demo never started music'
    return dict(frames=count, cpu_ram_video_identical=True, music_tick_anomalies=0,
                isr_anomalies_while_playing=0, music_transitions=transitions)

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('snapshot',type=Path);p.add_argument('symbols',type=Path);p.add_argument('out',type=Path)
    p.add_argument('--frames',type=int,default=16000);p.add_argument('--trd',type=Path)
    a=p.parse_args()
    a.snapshot=a.snapshot.resolve();a.symbols=a.symbols.resolve();a.out=a.out.resolve()
    if a.trd:a.trd=a.trd.resolve()
    a.out.mkdir(parents=True,exist_ok=True)
    vals={}
    for l in (ROOT/'Makefile').read_text().splitlines():
        if '=' in l and not l.startswith('\t'):
            k,v=l.split('=',1);vals[k.strip()]=v.strip()
    arch='arm64' if 'arm64' in subprocess.check_output(['file',str(ROOT/'zesarux')],text=True) else 'x86_64'
    cc=shlex.split(vals['CC'])+['-arch',arch];flags=shlex.split(vals['CFLAGS'])
    with tempfile.TemporaryDirectory(prefix='ay-hq-demo-') as temporary:
        tmp=Path(temporary);core=tmp/'core.o';probe=tmp/'probe.o';exe=tmp/'zesarux-trace'
        subprocess.run(cc+flags+['-finstrument-functions','-c','cores/core_spectrum.c','-o',str(core)],cwd=ROOT,check=True)
        subprocess.run(cc+flags+['-c','tests/ay_hq_demo_trace.c','-o',str(probe)],cwd=ROOT,check=True)
        output=subprocess.check_output(['make','-n','-W','core_spectrum.o','zesarux'],cwd=ROOT,text=True)
        link=next(shlex.split(l) for l in output.splitlines() if ' -o zesarux ' in l)
        link=[str(core) if x=='core_spectrum.o' else str(exe) if x=='zesarux' else x for x in link]
        subprocess.run(link+['-arch',arch,str(probe)],cwd=ROOT,check=True)
        for rom in ROOT.glob('*.rom'):
            (tmp/rom.name).symlink_to(rom)
        media_hash=digest(a.trd) if a.trd else None
        for name in ('legacy','hq'):
            env=os.environ.copy();env['AY_DEMO_TRACE']=str(a.out/(name+'.trace'));env['AY_DEMO_FRAMES']=str(a.frames)
            env.pop('AY_DEMO_HQ',None)
            if name=='hq':env['AY_DEMO_HQ']='1'
            cmd=[str(exe),'--noconfigfile','--machine','pentagon','--vo','null','--ao','null','--realvideo',
                 '--frameskip','0','--disable-autoframeskip','--nochangeslowparameters','--snap-no-change-machine',
                 '--nowelcomemessage','--nosplash','--verbose','2','--exit-after','600']
            if a.trd:cmd+=['--enable-betadisk','--enable-trd','--trd-file',str(a.trd),'--trd-no-persistent-writes']
            with (a.out/(name+'.log')).open('w') as log:
                subprocess.run(cmd+[str(a.snapshot)],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=700)
            assert 'Unable to open rom' not in (a.out/(name+'.log')).read_text(), 'Missing test ROM'
            print(name,'finished',flush=True)
        if a.trd:assert digest(a.trd)==media_hash,'Disk changed'
        result=analyze(a.out/'legacy.trace',a.out/'hq.trace',a.symbols,a.frames)
        result.update(snapshot=str(a.snapshot),snapshot_sha256=digest(a.snapshot),trd_sha256=media_hash)
        assert 'HQ active=1' in (a.out/'hq.log').read_text(),'HQ not exercised'
        (a.out/'result.json').write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps(result,indent=2))

if __name__=='__main__':main()
