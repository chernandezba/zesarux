#!/usr/bin/env python3
"""Generate direct QSound PLAY calls in cancionesql.bas.

This deliberately mirrors the SuperBASIC converter in cancionesqloriginal.bas.
beside the program so future edits to the original Spectrum PLAY strings can
be baked again without hand-transcribing QSound syntax.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path


def split_outside(text: str, separator: str) -> list[str]:
    result, start, quoted, depth = [], 0, False, 0
    for pos, char in enumerate(text):
        if char == '"':
            quoted = not quoted
        elif not quoted:
            if char == "(": depth += 1
            elif char == ")": depth -= 1
            elif char == separator and depth == 0:
                result.append(text[start:pos]); start = pos + 1
    result.append(text[start:])
    return result


def number(expr: str, values: dict[str, str]) -> int:
    expr = expr.strip()
    m = re.fullmatch(r"LEN\((\w+\$)\)(?:\s*([-+])\s*(\d+))?", expr, re.I)
    if m:
        value = len(values[m.group(1).lower()])
        if m.group(2): value += int(m.group(3)) * (1 if m.group(2) == "+" else -1)
        return value
    return int(expr)


def string_expr(expr: str, values: dict[str, str]) -> str:
    parts, start, quoted, depth = [], 0, False, 0
    for pos, char in enumerate(expr):
        if char == '"': quoted = not quoted
        elif not quoted:
            if char == "(": depth += 1
            elif char == ")": depth -= 1
            elif char in "&+" and depth == 0:
                parts.append(expr[start:pos]); start = pos + 1
    parts.append(expr[start:])
    if len(parts) > 1:
        return "".join(string_expr(part, values) for part in parts)
    atom = expr.strip()
    if len(atom) >= 2 and atom[0] == atom[-1] == '"': return atom[1:-1]
    m = re.fullmatch(r"(\w+\$)(?:\((.*)\))?", atom, re.I)
    if not m: raise ValueError(f"unsupported string expression: {expr}")
    value = values[m.group(1).lower()]
    section = m.group(2)
    if section is None: return value
    halves = re.split(r"\s*TO\s*", section.strip(), maxsplit=1, flags=re.I)
    if len(halves) == 1:
        index = number(halves[0], values)
        return value[index - 1:index]
    first = 1 if not halves[0] else number(halves[0], values)
    last = len(value) if not halves[1] else number(halves[1], values)
    return value[first - 1:last]


def expand(source: str) -> tuple[str, bool]:
    output, starts, loop = [], [], False
    for char in source:
        if char == "(":
            output.append("N"); starts.append(len(output))
        elif char == ")" and starts:
            start = starts.pop(); body = output[start:]
            output.extend(["N", *body, "N"])
        else: output.append(char)
    return "".join(output), loop


DURATIONS = {1: 6, 2: 9, 3: 12, 4: 18, 5: 24, 6: 36,
             7: 48, 8: 72, 9: 96, 10: 4, 11: 8, 12: 16}
NOTES = {"C": 0, "D": 2, "E": 4, "F": 5, "G": 7, "A": 9, "B": 11}


def duration(kind: int, tempo: int) -> float:
    return DURATIONS.get(kind, 24) * 125 / tempo


def tempo_of(source: str) -> int:
    comment = False
    for match in re.finditer(r"!|[Tt](\d+)", source):
        if match.group() == "!": comment = not comment
        elif not comment: return int(match.group(1))
    return 120


def qsound(source: str, tempo: int, emit_noise: bool = False) -> tuple[str, int, bool, list[tuple[int, int]]]:
    source, _ = expand(source)
    out, pos, octave, note_len, old_len = ["v15"], 0, 5, 5, 5
    q_octave = q_length = None
    triplet = tie = accidental = frames = 0
    comment = loop = False
    mixer_events = []
    envelope_volume = False
    envelope_shape = 0
    exact_frames = 0.0
    while pos < len(source):
        char = source[pos]
        if comment:
            if char == "!": comment = False
            pos += 1; continue
        if char == "!": comment = True; pos += 1; continue
        if char in " N(": pos += 1; continue
        if char == ")": loop = True; break
        if char == "H": break
        if char == "#": accidental += 1; pos += 1; continue
        if char == "$": accidental -= 1; pos += 1; continue
        if char.isdigit():
            end = pos
            while end < len(source) and source[end].isdigit(): end += 1
            value = int(source[pos:end])
            if 1 <= value <= 12:
                if value >= 10: old_len, triplet = note_len, 3
                note_len = value
                if end < len(source) and source[end] == "_":
                    tie += duration(value, tempo); end += 1
            pos = end; continue
        upper = char.upper()
        m = re.match(r"\d+", source[pos + 1:])
        value = int(m.group()) if m else 0
        end = pos + 1 + (len(m.group()) if m else 0)
        if upper == "T":
            if value > 0: tempo = value
            pos = end; continue
        if upper == "O": octave = value; pos = end; continue
        if upper == "V":
            envelope_volume = False
            out.append("v" + str(max(0, min(15, value)))); pos = end; continue
        if upper == "U": envelope_volume = True; out.append("v16"); pos += 1; continue
        if upper == "W":
            envelope_shape = {1:4, 2:11, 3:13, 4:8, 5:12, 6:14, 7:10}.get(value, 0)
            out.append("w" + str(envelope_shape))
            pos = end; continue
        if upper == "X": out.append("x" + str(min(32767, value))); pos = end; continue
        if upper == "M":
            mixer_events.append((frames, 255 - (value & 63)))
            pos = end; continue
        if upper in "YZ": pos = end; continue
        if char == "&":
            exact_frames += min(255, duration(note_len, tempo) + tie); tie = 0
            length = int(exact_frames + .5) - frames
            # La ROM 1.94 vuelve al interprete un tick despues de llegar a cero.
            q_ticks = max(0, length - 1)
            if q_ticks != q_length: out.append(f"l{q_ticks}"); q_length = q_ticks
            out.append("p"); frames += length
        elif upper in NOTES:
            # Cada nota con U debe reiniciar la envolvente; PLAY solo cambia
            # el periodo del tono al interpretar una letra musical.
            if envelope_volume: out.append("w" + str(envelope_shape))
            spectrum_note = (octave + int(char.isupper())) * 12 + NOTES[upper] + accidental
            noise = ((~spectrum_note) & 127) >> 2
            if emit_noise: out.append("n" + str(noise))
            qo = max(0, min(7, octave - 1 + int(char.isupper())))
            semitone = NOTES[upper] + accidental + 5
            while semitone < 0: semitone += 12; qo -= 1
            while semitone >= 12: semitone -= 12; qo += 1
            qo = max(0, min(7, qo)); accidental = 0
            names = ("C", "#C", "D", "#D", "E", "F", "#F", "G", "#G", "A", "#A", "H")
            exact_frames += min(255, duration(note_len, tempo) + tie); tie = 0
            length = int(exact_frames + .5) - frames
            # n no nulo consume otro tick al escribir el periodo de ruido.
            overhead = 1 + int(emit_noise and noise != 0)
            q_ticks = max(0, length - overhead)
            if qo != q_octave: out.append(f"o{qo}"); q_octave = qo
            if q_ticks != q_length: out.append(f"l{q_ticks}"); q_length = q_ticks
            out.append(names[semitone]); frames += length
        else:
            pos += 1; continue
        if triplet:
            triplet -= 1
            if not triplet and note_len >= 10: note_len = old_len
        pos += 1
    return "".join(out), frames, loop, mixer_events


def mixer_timeline(baked: list[tuple[str, int, bool, list[tuple[int, int]]]], target: int) -> str:
    events = []
    for channel, (_, frames, loop, source_events) in enumerate(baked):
        copies = range((target - 1) // frames + 1) if loop and frames else range(1)
        for copy in copies:
            offset = copy * frames
            for sequence, (frame, mixer) in enumerate(source_events):
                if offset + frame < target:
                    events.append((offset + frame, channel, sequence, mixer))
    collapsed = []
    for frame, _, _, mixer in sorted(events):
        if collapsed and collapsed[-1][0] == frame:
            collapsed[-1] = (frame, mixer)
        else:
            collapsed.append((frame, mixer))
    return "".join(f"{frame:05d}{mixer:03d}" for frame, mixer in collapsed)


def quote(value: str) -> str:
    return '"' + value.replace('"', '""') + '"'


def preserve_initial_envelope(baked):
    # La ROM 1.94 inicializa x0/w0 en CADA lista PLAY. La envolvente es
    # compartida: replica su configuracion inicial tambien en las otras voces.
    envelope = {"x": "0", "w": "0"}
    for music, _, _, _ in baked:
        for token in re.finditer(r"([a-z])([0-9]+)|([A-Hp#])", music):
            if token.group(3):
                break
            if token.group(1) in envelope:
                envelope[token.group(1)] = token.group(2)
    prefix = "x" + envelope["x"] + "w" + envelope["w"]
    # La barrera inicial s permite que todas las listas terminen su
    # inicializacion antes de que la voz A establezca el ruido compartido.
    return [(prefix + music, frames, loop, events)
            for music, frames, loop, events in baked]


def main(path: Path) -> None:
    lines = path.read_text().splitlines()
    targets = set()
    song_targets = set()
    for source_line in lines:
        targets.update(int(value) for value in re.findall(
            r"\b(?:GO\s+TO|GO\s+SUB|RESTORE)\s+(\d+)", source_line, re.I))
        match = re.match(r"(1[6-8]\d\d)\s+DATA\s+(\d+),\s*\"", source_line, re.I)
        if match:
            song_targets.add(int(match.group(2)))
            targets.add(int(match.group(2)))
    values: dict[str, str] = {}
    data340 = []
    converted = 0
    output = []
    for line in lines:
        if not line.strip():
            output.append(line)
            continue
        lineno, body = line.split(" ", 1)
        numeric_line = int(lineno)
        if 8240 <= numeric_line <= 8260 or 9000 <= numeric_line <= 10450 or 10600 <= numeric_line <= 10910:
            continue
        if lineno == "340": data340 = re.findall(r'"([^"]*)"', body)
        statements = split_outside(body, ":")
        rebuilt = []
        for statement in statements:
            stripped = statement.strip()
            if lineno == "360" and "zxreadpair" in stripped.lower():
                values["m$"] = "".join(data340[0::2]); values["j$"] = "".join(data340[1::2])
            assign = re.match(r"LET\s+(\w+\$)(?:\(([^)]*)\))?\s*=\s*(.*)$", stripped, re.I)
            static_assignment = False
            if assign:
                name, section, rhs = assign.group(1).lower(), assign.group(2), assign.group(3)
                try:
                    value = string_expr(rhs, values)
                    if section is None: values[name] = value
                    else:
                        old = values[name]; index = number(section, values) - 1
                        values[name] = old[:index] + value + old[index + len(value):]
                    static_assignment = 68 <= int(lineno) <= 5620
                except (KeyError, ValueError): pass
            play = re.fullmatch(r"zxplay([123])\s+(.+)", stripped, re.I)
            if play:
                count = int(play.group(1)); args = split_outside(play.group(2), ",")
                sources = [string_expr(arg, values) for arg in args]
                tempo = tempo_of(sources[0]); baked = [qsound(source, tempo) for source in sources]
                target = 3000 if count == 1 and baked[0][2] else max(item[1] for item in baked)
                baked[0] = qsound(sources[0], tempo, emit_noise=True)
                baked = preserve_initial_envelope(baked)
                timeline = mixer_timeline(baked, target)
                fields = []
                for music, frames, loop, _ in baked:
                    fields += [quote(music), str(frames), str(int(loop))]
                fields += [str(target), quote(timeline)]
                statement = "zxqplay" + str(count) + " " + ",".join(fields)
                converted += 1
            if not static_assignment:
                rebuilt.append(statement)
        if lineno in ("330", "350", "360", "380"):
            rebuilt = []
        if not rebuilt:
            if numeric_line in targets:
                rebuilt = ["REM QSound data preconverted"]
            else:
                continue
        output.append(lineno + " " + ":".join(rebuilt))
    if converted == 0: raise SystemExit("no zxplay calls found")
    # Conserva las entradas del menu sin REM de relleno: asigna a cada cancion
    # el numero de linea original de su primer PLAY preconvertido.
    for target in sorted(song_targets):
        placeholder = next((i for i, item in enumerate(output)
                            if item.startswith(f"{target} ")), None)
        if placeholder is None:
            continue
        first_play = next((i for i, item in enumerate(output)
                           if i >= placeholder and re.match(r"\d+\s+zxqplay[123]\s", item, re.I)), None)
        if first_play is None or first_play == placeholder:
            continue
        output[first_play] = re.sub(r"^\d+", str(target), output[first_play], count=1)
        del output[placeholder]
    output[0] = output[0].replace("DIM zxpos(32):", "")
    output[0] = re.sub(r"^(\d+\s+)", r"\1zxmixer=248:", output[0], count=1)
    output.extend([
        "", "10400 DEFine FuNction zxqfill$(q$,frames,target)",
        "10410  zcopies=INT(target/frames)+2", "10420  result$=q$",
        "10430  FOR zcopy=2 TO zcopies:result$=result$&q$", "10440  RETurn result$",
        "10450 END DEFine zxqfill$", "",
        "10560 DEFine PROCedure zxmwait(zframes,zmix$)",
        # La ROM Spectrum (0A05) reinicia el mezclador a F8 en cada PLAY.
        "10561  zxmixer=248:zp=1:znext=zframes",
        "10562  IF LEN(zmix$) THEN znext=(CODE(zmix$(zp))-48)*10000+(CODE(zmix$(zp+1))-48)*1000+(CODE(zmix$(zp+2))-48)*100+(CODE(zmix$(zp+3))-48)*10+CODE(zmix$(zp+4))-48",
        "10563  IF LEN(zmix$) THEN znewmix=(CODE(zmix$(zp+5))-48)*100+(CODE(zmix$(zp+6))-48)*10+CODE(zmix$(zp+7))-48:zp=zp+8",
        "10564  FOR zframe=0 TO zframes-1",
        "10565   REPeat zmev",
        "10566    IF zframe<znext THEN EXIT zmev",
        "10567    zxmixer=znewmix:POKE_AY 7,zxmixer",
        "10568    IF zp>LEN(zmix$) THEN znext=zframes:EXIT zmev",
        "10569    znext=(CODE(zmix$(zp))-48)*10000+(CODE(zmix$(zp+1))-48)*1000+(CODE(zmix$(zp+2))-48)*100+(CODE(zmix$(zp+3))-48)*10+CODE(zmix$(zp+4))-48",
        "10570    znewmix=(CODE(zmix$(zp+5))-48)*100+(CODE(zmix$(zp+6))-48)*10+CODE(zmix$(zp+7))-48:zp=zp+8",
        "10571   END REPeat zmev",
        "10572   POKE_AY 7,zxmixer:PAUSE 1:POKE_AY 7,zxmixer",
        "10573  END FOR zframe",
        "10574 END DEFine zxmwait", "",
        "10600 DEFine PROCedure zxqplay1(q1$,f1,l1,target,zmix$)",
        "10610  IF l1 THEN q1$=zxqfill$(q1$,f1,target)",
        "10620  SOUND_AY:PLAY 1,\"s\"&q1$&\"v0s\":zxready 1:RELEASE",
        "10630  zxmwait target,zmix$:IF l1=0 THEN zxready 1",
        "10635  SOUND_AY",
        "10640 END DEFine zxqplay1", "",
        "10700 DEFine PROCedure zxqplay2(q1$,f1,l1,q2$,f2,l2,target,zmix$)",
        "10710  IF l1 THEN q1$=zxqfill$(q1$,f1,target)",
        "10720  IF l2 THEN q2$=zxqfill$(q2$,f2,target)",
        "10730  SOUND_AY:PLAY 1,\"s\"&q1$&\"v0s\":PLAY 2,\"s\"&q2$&\"v0s\":zxready 1:zxready 2:RELEASE",
        "10740  zxmwait target,zmix$:IF l1=0 THEN zxready 1",
        "10742  IF l2=0 THEN zxready 2",
        "10745  SOUND_AY",
        "10750 END DEFine zxqplay2", "",
        "10800 DEFine PROCedure zxqplay3(q1$,f1,l1,q2$,f2,l2,q3$,f3,l3,target,zmix$)",
        "10810  IF l1 THEN q1$=zxqfill$(q1$,f1,target)",
        "10820  IF l2 THEN q2$=zxqfill$(q2$,f2,target)",
        "10830  IF l3 THEN q3$=zxqfill$(q3$,f3,target)",
        "10840  SOUND_AY:PLAY 1,\"s\"&q1$&\"v0s\":PLAY 2,\"s\"&q2$&\"v0s\":PLAY 3,\"s\"&q3$&\"v0s\"",
        "10845  zxready 1:zxready 2:zxready 3:RELEASE",
        "10850  zxmwait target,zmix$:IF l1=0 THEN zxready 1",
        "10852  IF l2=0 THEN zxready 2",
        "10854  IF l3=0 THEN zxready 3",
        "10856  SOUND_AY", "10860 END DEFine zxqplay3",
        "10920 DEFine PROCedure zxready(zchannel)",
        "10930  REPeat zrwait",
        "10940   IF PLAYING(zchannel)=0 THEN EXIT zrwait",
        "10950   PAUSE 1",
        "10960  END REPeat zrwait",
        "10970 END DEFine zxready",
    ])
    output = [line for line in output if line.strip()]
    path.write_text("\n".join(output) + "\n")
    print(f"converted {converted} calls")


if __name__ == "__main__":
    main(Path(sys.argv[1]))
