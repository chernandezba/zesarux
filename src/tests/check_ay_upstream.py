#!/usr/bin/env python3
"""Compare a clean baseline against this branch with HQ disabled (macOS build).

Generates redistributable synthetic SNA fixtures; no demo media needed.
Temporary probes observe identical call sites in both trees; normal binaries
and source files are not modified. Both checkouts must be configured/built.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def fixture(path,extended,kind):
    ram=bytearray(49152)
    code=bytearray.fromhex('f3 31 f0 bf 3e 91 ed 47 ed 5e fb')
    loop=0x8000+len(code)
    code+=bytes.fromhex('76 3a 00 a0 3c 32 00 a0 32 00 40')
    if kind!='tone':code+=bytes.fromhex('d3 fe')
    if kind!='beeper':
        for reg in range(14):
            code+=bytes([0x01,0xfd,0xff,0x3e,reg,0xed,0x79,0x01,0xfd,0xbf])
            if kind=='tone':
                value={0:125,7:0x3e,8:15}.get(reg,0)
                code+=bytes([0x3e,value])
            else:code+=bytes([0x3a,0x00,0xa0,0xee,reg*17&255])
            code+=bytes([0xed,0x79])
    code+=bytes([0xc3,loop&255,loop>>8])
    ram[0x4000:0x4000+len(code)]=code
    ram[0x5090:0x5093]=bytes.fromhex('fb ed 4d')
    ram[0x5100:0x5201]=bytes([0x90])*257
    header=bytearray(27);header[0]=0x91;header[19]=4;header[25]=2
    if extended:
        header[23:25]=(0xbff0).to_bytes(2,'little')
        image=header+ram+bytes.fromhex('00 80 10 00')+bytearray(5*16384)
    else:
        header[23:25]=(0xbfec).to_bytes(2,'little')
        ram[0x7fec:0x7fee]=(0x8000).to_bytes(2,'little')
        image=header+ram
    path.write_bytes(image)
def build(root,tmp,modified):
    tmp.mkdir()
    values={}
    for line in (root/'Makefile').read_text().splitlines():
        if '=' in line and not line.startswith('\t'):
            k,v=line.split('=',1);values[k.strip()]=v.strip()
    arch='arm64' if 'arm64' in subprocess.check_output(['file',str(root/'zesarux')],text=True) else 'x86_64'
    cc=shlex.split(values['CC'])+['-arch',arch];flags=shlex.split(values['CFLAGS'])
    overrides={}
    for obj,source in [('core_spectrum.o','cores/core_spectrum.c'),('ay38912.o','soundchips/ay38912.c'),('audio.o','audio/audio.c')]:
        text=(root/source).read_text()
        if obj=='ay38912.o':
            signature='void out_port_ay(z80_int puerto,z80_byte value)\n{'
            assert text.count(signature)==1
            text=text.replace(signature,signature+'\n    extern void ay_audit_write(unsigned,unsigned);\n    ay_audit_write(puerto,value);')
        if obj=='audio.o':
            signature='void audio_send_stereo_sample(char valor_sonido_izquierdo,char valor_sonido_derecho)\n{'
            assert text.count(signature)==1
            text=text.replace(signature,signature+'\n    extern void ay_audit_pcm(char,char);\n    ay_audit_pcm(valor_sonido_izquierdo,valor_sonido_derecho);')
        copy=tmp/Path(source).name;copy.write_text(text)
        target=tmp/obj
        extra=['-finstrument-functions'] if obj=='core_spectrum.o' else []
        subprocess.run(cc+flags+extra+['-c',str(copy),'-o',str(target)],cwd=root,check=True)
        overrides[obj]=str(target)
    probe=tmp/'probe.o'
    subprocess.run(cc+flags+['-O2']+(['-DAUDIT_WITH_HQ'] if modified else [])+['-c',str(ROOT/'tests/ay_regression_probe.c'),'-o',str(probe)],cwd=root,check=True)
    dry=subprocess.check_output(['make','-n','-W','core_spectrum.o','zesarux'],cwd=root,text=True)
    link=next(shlex.split(l) for l in dry.splitlines() if ' -o zesarux ' in l)
    overrides['zesarux']=str(tmp/'zesarux')
    subprocess.run([overrides.get(x,x) for x in link]+['-arch',arch,str(probe)],cwd=root,check=True)
    for rom in root.glob('*.rom'):(tmp/rom.name).symlink_to(rom)
    return tmp/'zesarux'
def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('baseline',type=Path);p.add_argument('out',type=Path)
    p.add_argument('--frames',type=int,default=500)
    p.add_argument('--realtime',action='store_true')
    p.add_argument('--machine',action='append')
    p.add_argument('--kind',action='append',choices=['tone','mixed','beeper'])
    a=p.parse_args();a.baseline=a.baseline.resolve();a.out=a.out.resolve();a.out.mkdir(parents=True,exist_ok=True)
    status=subprocess.check_output(['git','status','--porcelain','--untracked-files=no'],cwd=a.baseline.parent,text=True)
    if status:raise SystemExit('Baseline tracked sources must be clean: '+status)
    cases=[]
    with tempfile.TemporaryDirectory(prefix='ay-upstream-') as td:
        tmp=Path(td)
        exes={name:build(root,tmp/name,name=='modified') for name,root in [('upstream',a.baseline),('modified',ROOT)]}
        for machine in a.machine or ['48k','128k','P2','P2A40','P340','pentagon']:
            for kind in a.kind or ['tone','mixed','beeper']:
                tag=machine+'-'+kind;media=a.out/(tag+'.sna');fixture(media,machine!='48k',kind)
                for name,exe in exes.items():
                    env=os.environ.copy();env.update(AY_AUDIT_TRACE=str(a.out/(tag+'-'+name+'.trace')),AY_AUDIT_FRAMES=str(a.frames))
                    if a.realtime:env['AY_AUDIT_REALTIME']='1'
                    cmd=[str(exe),'--noconfigfile','--machine',machine,'--snap-no-change-machine','--vo','null','--ao','coreaudio' if a.realtime else 'null','--realvideo','--frameskip','0','--disable-autoframeskip','--nochangeslowparameters','--ay-autoenable','--nowelcomemessage','--nosplash','--verbose','2','--exit-after','120',str(media)]
                    with (a.out/(tag+'-'+name+'.log')).open('w') as log:subprocess.run(cmd,cwd=exe.parent,env=env,stdout=log,stderr=log,check=True,timeout=150)
                    log=(a.out/(tag+'-'+name+'.log')).read_text()
                    assert 'Unable to open rom' not in log and 'Unknown machine' not in log,log
                old=(a.out/(tag+'-upstream.trace')).read_text().splitlines();new=(a.out/(tag+'-modified.trace')).read_text().splitlines()
                assert len(old)==len(new)==a.frames,(tag,len(old),len(new))
                for n,(x,y) in enumerate(zip(old,new),1):
                    if x!=y:raise AssertionError(f'{tag}, frame {n}:\n upstream {x}\n modified {y}')
                counts=dict(x.split('=') for x in old[-1].split()[1:])
                assert int(counts['samples'])>a.frames*300
                if kind!='beeper':assert int(counts['writes'])>a.frames*20
                cases.append(dict(machine=machine,kind=kind,frames=a.frames,identical=True,last=counts,fixture_sha256=sha(media)))
                print(tag,'identical',flush=True)
    result=dict(probe_sha256=sha(ROOT/'tests/ay_regression_probe.c'),runner_sha256=sha(Path(__file__)),baseline_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=a.baseline,text=True).strip(),hq_enabled=False,realtime=a.realtime,cases=cases)
    (a.out/'result.json').write_text(json.dumps(result,indent=2)+'\n')
if __name__=='__main__':main()
