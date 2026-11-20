#!/usr/bin/env python3
"""Match two stereo s16le/44100 renders with fixed gain and write listening WAVs."""
import argparse
import array
import json
import math
import sys
import wave
from pathlib import Path

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('old_pcm', type=Path)
p.add_argument('new_pcm', type=Path)
p.add_argument('output_dir', type=Path)
a = p.parse_args()
a.output_dir.mkdir(parents=True, exist_ok=True)
items = []
for name, path in [('old', a.old_pcm), ('hq', a.new_pcm)]:
    pcm = array.array('h')
    pcm.frombytes(path.read_bytes())
    if sys.byteorder != 'little':
        pcm.byteswap()
    if len(pcm) % 2:
        raise ValueError('Incomplete stereo frame')
    n = len(pcm) // 2
    means = [sum(pcm[c::2]) / n for c in range(2)]
    energy = sum((x - means[i % 2]) ** 2 for i, x in enumerate(pcm))
    rms = math.sqrt(energy / len(pcm))
    peak = max(abs(x - means[i % 2]) for i, x in enumerate(pcm))
    items.append(dict(name=name, pcm=pcm, frames=n, means=means, rms=rms, peak=peak))
if items[0]['frames'] != items[1]['frames']:
    raise ValueError('Different durations')
# Ganancia constante, sin compresion; mismo RMS y margen minimo de 1 dB.
target = min(32768 * .1, *(x['rms'] * (32767 * 10 ** (-1 / 20)) / x['peak'] for x in items))
report = dict(sample_rate=44100, channels=2, frames=items[0]['frames'],
              seconds=items[0]['frames'] / 44100, target_rms_dbfs=20 * math.log10(target / 32768), outputs=[])
for item in items:
    gain = target / item['rms']
    out = array.array('h', (round((x-item['means'][i % 2]) * gain) for i, x in enumerate(item['pcm'])))
    peak = max(abs(x) for x in out)
    assert peak < 32767
    rms = math.sqrt(sum(x*x for x in out) / len(out))
    filename = f"p03_standin_{item['name']}.wav"
    if sys.byteorder != 'little':
        out.byteswap()
    with wave.open(str(a.output_dir / filename), 'wb') as w:
        w.setparams((2, 2, 44100, 0, 'NONE', 'not compressed'))
        w.writeframes(out.tobytes())
    result = dict(file=filename, gain_db=20*math.log10(gain), rms_dbfs=20*math.log10(rms/32768),
                  peak_dbfs=20*math.log10(peak/32768), clipped_samples=0)
    report['outputs'].append(result)
    print(json.dumps(result))
(a.output_dir / 'levels.json').write_text(json.dumps(report, indent=2) + '\n')
