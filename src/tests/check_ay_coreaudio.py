#!/usr/bin/env python3
"""Native, headless CoreAudio HQ lifecycle/fallback smoke test (macOS).
Uses generated fixtures only. Does not edit configuration or supplied media.
"""
import argparse
import json
from pathlib import Path
import re
import socket
import subprocess
import sys
import time
sys.dont_write_bytecode=True
from check_ay_upstream import fixture

ROOT=Path(__file__).resolve().parents[1]
class Remote:
    def __init__(self,port):
        deadline=time.monotonic()+10
        while True:
            try:self.socket=socket.create_connection(('127.0.0.1',port),timeout=1);break
            except OSError:
                if time.monotonic()>deadline:raise
                time.sleep(.05)
        self.socket.settimeout(15);self.read()
    def read(self):
        data=b''
        while not (data.endswith(b'command> ') or data.endswith(b'command@cpu-step> ')):
            part=self.socket.recv(65536)
            if not part:raise RuntimeError('ZRCP disconnected: '+repr(data))
            data+=part
        return data.decode(errors='replace')
    def call(self,command):
        self.socket.sendall(command.encode()+b'\r\n');result=self.read()
        if 'ERROR' in result or 'Error.' in result:raise RuntimeError(command+': '+result)
        return result
    def count(self):
        text=self.call('read-memory A000H 1')
        value=re.search(r'\b([0-9a-fA-F]{2})\b',text)
        assert value,text
        return int(value[1],16)
    def exit(self):self.socket.sendall(b'exit-emulator\r\n');self.socket.close()
def free_port():
    with socket.socket() as s:s.bind(('127.0.0.1',0));return s.getsockname()[1]
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('out',type=Path);a=p.parse_args()
    a.out=a.out.resolve();a.out.mkdir(parents=True,exist_ok=True)
    paths={}
    for name,extended in [('48k',False),('128k',True)]:
        paths[name]=a.out/(name+'.sna');fixture(paths[name],extended,'mixed')
    common=[str(ROOT/'zesarux'),'--noconfigfile','--vo','null','--ao','coreaudio','--ay-hq-coreaudio',
        '--audiovolume','20','--ay-autoenable','--realvideo','--frameskip','0','--disable-autoframeskip',
        '--nochangeslowparameters','--snap-no-change-machine','--nowelcomemessage','--nosplash','--verbose','2']
    port=free_port();log=a.out/'lifecycle.log';checks=[]
    with log.open('w') as output:
        process=subprocess.Popen(common+['--machine','pentagon','--enable-remoteprotocol','--remoteprotocol-port',str(port),'--exit-after','90',str(paths['128k'])],cwd=ROOT,stdout=output,stderr=output)
        try:
            r=Remote(port);time.sleep(2)
            before=r.count();time.sleep(.3);assert r.count()!=before;checks.append('initial playback executes fixture')
            r.call('enter-cpu-step');paused=r.count();registers=r.call('get-registers');time.sleep(.25)
            assert r.count()==paused and r.call('get-registers')==registers;checks.append('pause preserves CPU and memory')
            saved=a.out/'saved.sna';r.call('snapshot-save '+str(saved));assert saved.exists()
            r.call('exit-cpu-step');time.sleep(.3);assert r.count()!=paused;checks.append('resume advances fixture')
            r.call('enter-cpu-step');r.call('reset-cpu');r.call('snapshot-load '+str(saved));assert r.count()==paused
            assert 'IM2' in r.call('get-registers');checks.append('reset and snapshot restore recover fixture state')
            r.call('exit-cpu-step');time.sleep(.4);assert r.count()!=paused
            for machine,media in [('48k',paths['48k']),('128k',paths['128k']),('P340',paths['128k']),('pentagon',paths['128k'])]:
                # set-machine sale de cpu-step; dejar que se cierre su menú remoto.
                r.call('set-machine '+machine);time.sleep(.3)
                r.call('snapshot-load '+str(media));time.sleep(.3)
                start=r.count();time.sleep(.5)
                assert r.count()!=start and 'IM2' in r.call('get-registers');checks.append('machine switch/reload '+machine)
            time.sleep(2);r.exit();process.wait(timeout=10)
        finally:
            if process.poll() is None:process.terminate();process.wait(timeout=5)
    text=log.read_text();assert 'CoreAudio HQ: 44100 Hz' in text
    stats=re.search(r'CoreAudio HQ: (\d+) frames, peak (\d+); (\d+) underrun callbacks, (\d+) dropped frames',text)
    assert stats and int(stats[1])>44100 and int(stats[2])>0,text[-3000:]
    assert int(stats[4])==0,stats.group(0)
    assert 'AY HQ: 44100 Hz' in text
    playback_modes=[]
    for name,options in [('steady',[]),('reduced',['--spectrum-reduced-core']),('two-ay',['--totalaychips','2'])]:
        target=a.out/(name+'.log')
        with target.open('w') as output:
            subprocess.run(common+['--machine','128k','--exit-after','3']+options+[str(paths['128k'])],cwd=ROOT,stdout=output,stderr=output,timeout=15,check=True)
        text=target.read_text()
        assert ('AY HQ: 44100 Hz' in text)==(name=='steady')
        stats2=re.search(r'CoreAudio HQ: (\d+) frames, peak (\d+); (\d+) underrun callbacks, (\d+) dropped frames',text)
        assert stats2 and int(stats2[1])>44100 and int(stats2[2])>0,text[-2000:]
        assert int(stats2[3])==int(stats2[4])==0,stats2.group(0)
        playback_modes.append(dict(mode=name,frames=int(stats2[1]),peak=int(stats2[2]),underruns=int(stats2[3]),drops=int(stats2[4])))
    result=dict(checks=checks,hq_frames=int(stats[1]),peak=int(stats[2]),underrun_callbacks_in_lifecycle_run=int(stats[3]),dropped_frames=int(stats[4]),playback_modes=playback_modes)
    (a.out/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
if __name__=='__main__':main()
