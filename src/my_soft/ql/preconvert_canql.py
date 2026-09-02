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


def duration(kind: int, tempo: int) -> int:
    return max(1, min(255, int(DURATIONS.get(kind, 24) * 125 / tempo + .5)))


def tempo_of(source: str) -> int:
    comment = False
    for match in re.finditer(r"!|[Tt](\d+)", source):
        if match.group() == "!": comment = not comment
        elif not comment: return int(match.group(1))
    return 120


def qsound(source: str, tempo: int) -> tuple[str, int, bool]:
    source, _ = expand(source)
    out, pos, octave, note_len, old_len = ["v15"], 0, 5, 5, 5
    q_octave = q_length = None
    triplet = tie = accidental = frames = 0
    comment = loop = False
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
        if upper == "V": out.append("v" + str(max(0, min(15, value)))); pos = end; continue
        if upper == "U": out.append("v16"); pos += 1; continue
        if upper == "W":
            out.append("w" + str({1:4, 2:11, 3:13, 4:8, 5:12, 6:14, 7:10}.get(value, 0)))
            pos = end; continue
        if upper == "X": out.append("x" + str(min(32767, value))); pos = end; continue
        if upper in "MYZ": pos = end; continue
        if char == "&":
            length = min(255, duration(note_len, tempo) + tie); tie = 0
            if length != q_length: out.append(f"l{length}"); q_length = length
            out.append("p"); frames += length
        elif upper in NOTES:
            qo = max(0, min(7, octave - 1 + int(char.isupper())))
            semitone = NOTES[upper] + accidental + 5
            while semitone < 0: semitone += 12; qo -= 1
            while semitone >= 12: semitone -= 12; qo += 1
            qo = max(0, min(7, qo)); accidental = 0
            names = ("C", "#C", "D", "#D", "E", "F", "#F", "G", "#G", "A", "#A", "H")
            length = min(255, duration(note_len, tempo) + tie); tie = 0
            if qo != q_octave: out.append(f"o{qo}"); q_octave = qo
            if length != q_length: out.append(f"l{length}"); q_length = length
            out.append(names[semitone]); frames += length
        else:
            pos += 1; continue
        if triplet:
            triplet -= 1
            if not triplet and note_len >= 10: note_len = old_len
        pos += 1
    return "".join(out), frames, loop


def quote(value: str) -> str:
    return '"' + value.replace('"', '""') + '"'


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
                fields = []
                for music, frames, loop in baked:
                    fields += [quote(music), str(frames), str(int(loop))]
                fields.append(str(target))
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
    # Keep menu entry points without placeholder REM lines: assign each song's
    # first baked PLAY call the original entry line number.
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
    output.extend([
        "", "10400 DEFine FuNction zxqfill$(q$,frames,target)",
        "10410  copies=INT(target/frames)+2", "10420  result$=q$",
        "10430  FOR copy=2 TO copies:result$=result$&q$", "10440  RETurn result$",
        "10450 END DEFine zxqfill$", "",
        "10600 DEFine PROCedure zxqplay1(q1$,f1,l1,target)",
        "10610  IF l1 THEN q1$=zxqfill$(q1$,f1,target)",
        "10620  SOUND_AY:HOLD:PLAY 1,q1$:RELEASE", "10630  zxwait target:SOUND_AY",
        "10640 END DEFine zxqplay1", "",
        "10700 DEFine PROCedure zxqplay2(q1$,f1,l1,q2$,f2,l2,target)",
        "10710  IF l1 THEN q1$=zxqfill$(q1$,f1,target)",
        "10720  IF l2 THEN q2$=zxqfill$(q2$,f2,target)",
        "10730  SOUND_AY:HOLD:PLAY 1,q1$:PLAY 2,q2$:RELEASE", "10740  zxwait target:SOUND_AY",
        "10750 END DEFine zxqplay2", "",
        "10800 DEFine PROCedure zxqplay3(q1$,f1,l1,q2$,f2,l2,q3$,f3,l3,target)",
        "10810  IF l1 THEN q1$=zxqfill$(q1$,f1,target)",
        "10820  IF l2 THEN q2$=zxqfill$(q2$,f2,target)",
        "10830  IF l3 THEN q3$=zxqfill$(q3$,f3,target)",
        "10840  SOUND_AY:HOLD:PLAY 1,q1$:PLAY 2,q2$:PLAY 3,q3$:RELEASE",
        "10850  zxwait target:SOUND_AY", "10860 END DEFine zxqplay3",
    ])
    output = [line for line in output if line.strip()]
    path.write_text("\n".join(output) + "\n")
    print(f"converted {converted} calls")


if __name__ == "__main__":
    main(Path(sys.argv[1]))
