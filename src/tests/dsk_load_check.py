#!/usr/bin/env python3
"""Prueba de carga de un DSK de Spectrum +3 mediante ZRCP.

Con una captura SCR conocida de un menú, --reference puede confirmarlo. Sin
ella solo se infiere una carga probable a partir de pantalla, CPU y disco:
una pantalla de error elaborada podría parecer un juego.

Resultados (en mayúsculas en la salida normal; en minúsculas con --json):
  CARGA_PROBABLE (código 0): pantalla, CPU y disco sugieren que cargó.
  MENU_CONFIRMADO (código 0): coincide con la captura SCR de --reference.
  INDETERMINADO (código 1): no hay señales suficientes para decidir.
  ERROR (código 2): falló la prueba o se detectó un error del emulador.

Los argumentos posteriores a -- se pasan literalmente a ZEsarUX.
Ejemplo desde src/: python3 tests/dsk_load_check.py codex_additional/MutanZone.dsk -- --verbose 3

Si una pantalla de título parece esperar una tecla, el script prueba Espacio
hasta tres veces. --no-auto-space desactiva esos intentos.
"""

import argparse
import json
import os
import pty
import re
import socket
import subprocess
import sys
import tempfile
import threading
import time
from collections import deque
from pathlib import Path


READ_RE = re.compile(r"End receiving command parameters for READ_DATA.*Current track: ([0-9A-F]+)H")
PC_RE = re.compile(r"\bPC=([0-9A-Fa-f]+)\b")
ERROR_RE = re.compile(r"Error getting PD765 buffer beyond limit|Error putting PD765 buffer beyond limit|AddressSanitizer|runtime error")
BITS = tuple(bin(value).count("1") for value in range(256))


def parse_key(spec):
    try:
        when, key = spec.split(":", 1)
        when = float(when)
        if when < 0:
            raise ValueError
        codes = {"enter": 13, "space": 32}
        code = codes[key.lower()] if key.lower() in codes else int(key)
        if not 0 <= code <= 255:
            raise ValueError
        return when, code
    except (ValueError, KeyError) as exc:
        raise argparse.ArgumentTypeError("Use SEGUNDOS:enter, SEGUNDOS:space o SEGUNDOS:ASCII") from exc


def free_port():
    with socket.socket() as listener:
        listener.bind(("127.0.0.1", 0))
        return listener.getsockname()[1]


class DiskActivity:
    def __init__(self):
        self.lock = threading.Lock()
        self.reads = 0
        self.nonzero_reads = 0
        self.last_read = None
        self.errors = deque(maxlen=8)

    def consume(self, descriptor):
        # El emulador almacena stdout en un búfer si se conecta a una tubería.
        # Un PTY permite recibir las líneas de actividad mientras se ejecuta.
        pending = b""
        try:
            while True:
                try:
                    data = os.read(descriptor, 65536)
                except OSError:
                    break
                if not data:
                    break
                pending += data
                lines = pending.split(b"\n")
                pending = lines.pop()
                for raw in lines:
                    line = raw.decode("utf-8", "replace")
                    match = READ_RE.search(line)
                    with self.lock:
                        if match:
                            self.reads += 1
                            if int(match.group(1), 16) > 0:
                                self.nonzero_reads += 1
                            self.last_read = time.monotonic()
                        if ERROR_RE.search(line):
                            self.errors.append(line.strip())
        finally:
            os.close(descriptor)

    def snapshot(self):
        with self.lock:
            return self.reads, self.nonzero_reads, self.last_read, list(self.errors)


class Remote:
    def __init__(self, sock):
        self.sock = sock
        self.read_prompt()

    def read_prompt(self):
        data = bytearray()
        while not data.endswith(b"command> "):
            part = self.sock.recv(65536)
            if not part:
                raise RuntimeError("ZRCP cerró la conexión")
            data.extend(part)
            if len(data) > 2_000_000:
                raise RuntimeError("Respuesta ZRCP demasiado grande")
        return data.decode("utf-8", "replace")[:-len("command> ")]

    def command(self, command):
        self.sock.sendall((command + "\r\n").encode("ascii"))
        return self.read_prompt()


def connect(port, process, deadline):
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise RuntimeError(f"ZEsarUX terminó durante el arranque ({process.returncode})")
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=1)
            sock.settimeout(10)
            return Remote(sock)
        except OSError:
            time.sleep(0.1)
    raise RuntimeError("ZRCP no empezó a escuchar")


def changed_fraction(first, second, length=6144):
    # La pantalla SCR guarda 6144 bytes de píxeles y 768 atributos de color.
    return sum(BITS[a ^ b] for a, b in zip(first[:length], second[:length])) / (length * 8)


def ink_fraction(screen):
    return sum(BITS[byte] for byte in screen[:6144]) / (6144 * 8)


def has_large_paper_panel(screen):
    # Menús como Daley Thompson usan un panel rectangular claro con texto.
    # Los atributos SCR indican el color de fondo de cada celda de 8x8.
    paper = [(attribute >> 3) & 7 for attribute in screen[6144:]]
    colored = sum(color != 0 for color in paper)
    wide_rows = 0
    for row in range(24):
        run = longest = 0
        for color in paper[row * 32:(row + 1) * 32]:
            run = run + 1 if color else 0
            longest = max(longest, run)
        if longest >= 8:
            wide_rows += 1
    return colored >= 150 and wide_rows >= 6


def run(args):
    emulator = args.emulator.resolve()
    disk = args.dsk.resolve()
    if not emulator.is_file() or not disk.is_file():
        raise RuntimeError("No existe el ejecutable o el DSK indicado")
    reference = args.reference.read_bytes() if args.reference else None
    if reference is not None and len(reference) != 6912:
        raise RuntimeError("La referencia debe ser una captura SCR de 6912 bytes")

    port = free_port()
    actions = sorted(args.key or [])
    auto_space = not args.key and not args.no_auto_space
    activity = DiskActivity()
    samples = deque(maxlen=4)
    first_screen = None
    best = None
    started = time.monotonic()
    with tempfile.TemporaryDirectory(prefix="zesarux-dsk-check-") as tempdir:
        screen_path = Path(tempdir) / "screen.scr"
        command = [str(emulator), "--noconfigfile", "--ao", "null",
                   "--fastautoload", "--accelerate-loading",
                   "--machine", "p3s", "--dsk-no-persistent-writes", str(disk),
                   "--enable-remoteprotocol",
                   "--remoteprotocol-port", str(port), "--exit-after", str(int(args.timeout + 15)),
                   "--verbose", "2", "--verbose-always-console"]
        command.extend(args.zesarux_args)
        master, slave = pty.openpty()
        try:
            process = subprocess.Popen(command, cwd=emulator.parent, stdout=slave,
                                       stderr=slave)
        finally:
            os.close(slave)
        reader = threading.Thread(target=activity.consume, args=(master,), daemon=True)
        reader.start()
        remote = None
        try:
            remote = connect(port, process, started + 12)
            next_action = 0
            last_action = None
            last_action_screen = None
            auto_space_tries = 0
            while time.monotonic() - started < args.timeout:
                elapsed = time.monotonic() - started
                while next_action < len(actions) and elapsed >= actions[next_action][0]:
                    remote.command(f"send-keys-ascii 100 {actions[next_action][1]}")
                    last_action = time.monotonic()
                    last_action_screen = samples[-1] if samples else None
                    next_action += 1
                # Algunos títulos, como Buggy Boy y Mot, esperan Espacio tras
                # la autocarga. Reintentar evita pulsar antes de que acepten
                # teclado; las teclas indicadas por --key sustituyen esta ayuda.
                if (auto_space and auto_space_tries < 3 and elapsed >= 12 and samples and
                    (last_action is None or time.monotonic() - last_action >= 6)):
                    probe_reads, _, probe_last_read, _ = activity.snapshot()
                    probe_idle = elapsed if probe_last_read is None else time.monotonic() - probe_last_read
                    if probe_reads >= 1 and probe_idle >= args.idle_seconds and ink_fraction(samples[-1]) >= 0.05:
                        remote.command("send-keys-ascii 100 32")
                        last_action = time.monotonic()
                        last_action_screen = samples[-1]
                        auto_space_tries += 1
                remote.command(f"save-screen {screen_path}")
                screen = screen_path.read_bytes()
                if len(screen) != 6912:
                    raise RuntimeError(f"Captura SCR inesperada: {len(screen)} bytes")
                if first_screen is None:
                    first_screen = screen
                samples.append(screen)
                registers = remote.command("get-registers")
                pc_match = PC_RE.search(registers)
                pc = int(pc_match.group(1), 16) if pc_match else None
                reads, nonzero_reads, last_read, errors = activity.snapshot()
                if errors:
                    return {"resultado": "error", "motivo": errors[-1], "segundos": round(elapsed, 1)}
                idle = elapsed if last_read is None else time.monotonic() - last_read
                novelty = changed_fraction(first_screen, screen)
                movement = max((changed_fraction(a, b) for a, b in zip(samples, list(samples)[1:])), default=1.0)
                post_key_change = (changed_fraction(last_action_screen, screen)
                                   if last_action_screen is not None else 0)
                menu_panel = has_large_paper_panel(screen)
                evidence = {"segundos": round(elapsed, 1), "lecturas": reads,
                            "lecturas_fuera_pista_0": nonzero_reads,
                            "sin_lecturas_segundos": round(idle, 1),
                            "cambio_pantalla": round(novelty, 3),
                            "movimiento_pantalla": round(movement, 3),
                            "cambio_tras_tecla": round(post_key_change, 3),
                            "espacios_automaticos": auto_space_tries,
                            "contenido_pantalla": round(ink_fraction(screen), 3),
                            "panel_de_menu": menu_panel,
                            "pc": f"{pc:04X}" if pc is not None else None}
                if reference is not None:
                    evidence["diferencia_referencia"] = round(changed_fraction(reference, screen, 6912), 3)
                best = evidence
                # Exigir carga desde el disco, salida de la pantalla inicial,
                # CPU en RAM y actividad visual persistente tras cesar la lectura.
                enough_disk_reads = (nonzero_reads >= 1 or
                                     (reads >= 3 and novelty >= 0.15) or
                                     (reads >= 2 and post_key_change >= 0.03) or
                                     (reads >= 2 and menu_panel))
                # Una pantalla de menú puede ser muy escueta, como Artura.
                # Solo rebajar el umbral visual si ya hubo muchas lecturas
                # fuera de la pista de arranque.
                minimum_ink = 0.05 if nonzero_reads >= 10 else 0.08 if menu_panel else 0.10
                keys_complete = next_action == len(actions) and (
                    last_action is None or time.monotonic() - last_action >= 4)
                if (reference is not None and keys_complete and reads >= 1 and
                    elapsed >= args.min_seconds and
                    changed_fraction(reference, screen, 6912) <= args.reference_difference):
                    if args.screenshot:
                        args.screenshot.write_bytes(screen)
                    return {"resultado": "menu_confirmado", "evidencia": evidence}
                if (elapsed >= args.min_seconds and len(samples) == samples.maxlen and
                    reference is None and keys_complete and enough_disk_reads and
                    idle >= args.idle_seconds and
                    novelty >= 0.05 and ink_fraction(screen) >= minimum_ink and
                    pc is not None and pc >= 0x4000 and movement <= 0.35):
                    if args.screenshot:
                        args.screenshot.write_bytes(screen)
                    return {"resultado": "carga_probable", "evidencia": evidence}
                if process.poll() is not None:
                    break
                time.sleep(args.interval)
            if args.screenshot and screen_path.exists():
                args.screenshot.write_bytes(screen_path.read_bytes())
            return {"resultado": "indeterminado", "evidencia": best,
                    "motivo": "No se reunieron suficientes señales para reconocer un menú o juego"}
        finally:
            if remote is not None:
                remote.sock.close()
            process.terminate()
            try:
                process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
            reader.join(timeout=1)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("dsk", type=Path, help="Imagen DSK que se va a probar")
    parser.add_argument("--emulator", type=Path,
                        default=Path(__file__).resolve().parents[1] / "zesarux")
    parser.add_argument("--timeout", type=float, default=100, help="Límite en segundos (100)")
    parser.add_argument("--interval", type=float, default=2, help="Intervalo entre capturas (2)")
    parser.add_argument("--min-seconds", type=float, default=10, help="Tiempo mínimo antes de aceptar carga (10)")
    parser.add_argument("--idle-seconds", type=float, default=6, help="Tiempo sin lecturas antes de aceptar carga (6)")
    parser.add_argument("--key", type=parse_key, action="append",
                        help="Tecla programada, por ejemplo 3:enter o 30:space; se puede repetir")
    parser.add_argument("--no-auto-space", action="store_true",
                        help="No pulsar Espacio automáticamente ante una pantalla de espera")
    parser.add_argument("--screenshot", type=Path, help="Guardar la última pantalla en formato SCR")
    parser.add_argument("--reference", type=Path, help="SCR de un menú conocido para confirmación visual")
    parser.add_argument("--reference-difference", type=float, default=0.10,
                        help="Diferencia máxima de píxeles y colores frente a la referencia (0.10)")
    parser.add_argument("--json", action="store_true", help="Mostrar resultado en JSON")
    # El separador impide que argparse interprete opciones propias de ZEsarUX.
    argv = sys.argv[1:]
    if "--" in argv:
        separator = argv.index("--")
        args = parser.parse_args(argv[:separator])
        args.zesarux_args = argv[separator + 1:]
    else:
        args = parser.parse_args(argv)
        args.zesarux_args = []
    if (args.timeout <= 0 or args.interval <= 0 or args.idle_seconds < 0 or
        not 0 <= args.reference_difference <= 1):
        parser.error("Los tiempos deben ser positivos")
    try:
        result = run(args)
    except (OSError, RuntimeError, ValueError) as exc:
        result = {"resultado": "error", "motivo": str(exc)}
    if args.json:
        print(json.dumps(result, ensure_ascii=False))
    else:
        print(result["resultado"].upper())
        if "motivo" in result:
            print(result["motivo"])
        if "evidencia" in result:
            print(json.dumps(result["evidencia"], ensure_ascii=False, indent=2))
    return 0 if result["resultado"] in ("carga_probable", "menu_confirmado") else 2 if result["resultado"] == "error" else 1


if __name__ == "__main__":
    sys.exit(main())
