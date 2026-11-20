# Experimental Spectrum AY at 44.1 kHz on macOS

Enable the prototype explicitly; the default audio path is unchanged:

```sh
cd src
./configure --c-compiler clang --disable-xwindows --disable-curses --disable-sndfile
make -j4
./zesarux --noconfigfile --machine 128k --ao coreaudio --ay-hq-coreaudio --no-spectrum-reduced-core
```

Append a snapshot or tape filename to run a game or demo. With a 48K model,
`--ay-autoenable` enables the external AY when software accesses its ports.
Remove `--ay-hq-coreaudio` for an A/B comparison. Normal AY stereo modes and
`--audiovolume` apply. The prototype uses conservative gain; compare at matched
perceived loudness. With `--verbose 2`, startup reports the selected audio format
and AY clock, and shutdown reports generated frames, peak amplitude, underrun
callbacks and dropped frames.

## Signal path

- A separate AY generator runs at AY master clock / 8. Tone periods remain
  integers in the chip clock domain; they are never rounded to whole hertz.
- Noise uses a 17-bit LFSR. All 16 envelope shapes are implemented, including
  repeated writes to R13 restarting the envelope. Zero periods map to one.
- AY port writes flush elapsed CPU time before changing the register. Scanline
  and frame hooks flush the remaining interval; instruction overshoot across a
  frame boundary is retained. Integer rational accumulators avoid sample drift.
- The AY amplitude curve preserves quiet levels. Its table is from Peter
  Sovietov's MIT-licensed Ayumi; see `licenses/ay_hq_ayumi.txt`.
- A 192-tap, 256-phase Blackman-windowed sinc FIR with a 19 kHz cutoff resamples
  the chip stream to 44,100 samples/second. A 20 Hz DC blocker follows the FIR.
- CoreAudio receives interleaved, signed 16-bit stereo. A separate bounded SPSC
  queue publishes complete stereo frames with acquire/release atomics. The audio
  callback allocates nothing and writes zeroes on underrun. An overrun drops new
  frames; it does not change the consumer's index.

The CPU timebase follows the existing emulator's 50 Hz pacing
(`screen_testados_total * 50`); AY uses the existing nominal 1,773,400 Hz clock (1,750,000 Hz on Pentagon).
This preserves 882 samples per video frame. It does not change the emulator's
existing PAL timing policy or calibrate the nominal AY oscillator.

## Scope and limitations

HQ currently applies to classic 16/48K and 128K/+2/+2A/+3 Spectrum models and Pentagon 128, a single
AY, the normal Spectrum core, 50 Hz, and normal CPU speed. Beeper audio retains
its existing scanline resolution and is mixed through the FIR; only AY receives
the new chip-time synthesis. The FIR adds approximately 0.43 ms of delay. Driver
startup still follows the existing five-video-frame audio batch (about 100 ms),
and the HQ queue has a fixed 16,384-stereo-frame capacity. Existing CoreAudio FIFO
size settings affect the legacy path only.

Unsupported machines, turbo, reduced core, multiple AY chips, Nagravision/1-bit audio filters,
extra DAC/GS/HiLow/Interface 1 audio, real-tape playback/recording, menu/test tone
generators and ZENG streaming use the original mixed signal, resampled to the
44.1 kHz device stream. That compatibility resampler uses sample repetition;
it is not the HQ AY renderer. Switching modes may leave previously queued audio
for a short interval. This prototype is not a complete redesign of those paths.

The legacy audio buffers still feed waveform displays, file recording and other
existing consumers, so built-in WAV recording is still 8-bit/15.6 kHz. It does
not capture the new CoreAudio stream. The prototype has no persistent GUI
setting yet. Default execution without the option uses the original generator
and output format.

## Validation

```sh
CC=clang tests/ay_hq.sh
```

The tests compile with AddressSanitizer and UndefinedBehaviorSanitizer and check:

- tone/noise periods, zero periods, all envelope shapes and R13 retriggers;
- identical synthesis under different partitions of elapsed CPU time;
- exactly 882 samples/frame over 500 frames, for 48K, 128K and Pentagon timings;
- a retained 13.85 kHz tone and volume 1, plus suppression of a selected
  ultrasonic tone that otherwise aliases into the audible range;
- the Spectrum adapter against a direct renderer, including timed writes,
  repeated R13 and instruction overshoot across frame boundaries;
- fallback eligibility for turbo, reduced core and multiple chips.

Optional test PCM output (signed native-endian 16-bit stereo, 44.1 kHz):

```sh
CC=clang tests/ay_hq.sh /tmp/ay-hq-demo.pcm
```

On Apple Silicon macOS, the initial native CoreAudio smoke test with a generated 48K
snapshot producing an AY tone ran for five seconds: 219,617 generated frames,
peak 962, zero underrun callbacks and zero dropped frames. A 128K boot also
initialized HQ successfully. These are runtime and signal checks. Real-music
A/B listening and an independent software reference have since been added;
a hardware capture is still needed before proposing a new default renderer.

### Upstream/default-path regression

`tests/check_ay_upstream.py` generates small, redistributable SNA programs for
AY-only tone, mixed AY+beeper and beeper-only playback. It compares two separately
built source trees: clean upstream and this branch without the HQ option. The
same temporary probes observe CPU registers/T-states/interrupt state, RAM, video,
AY port writes with their timestamps, AY registers and legacy mixer samples
before backend conversion. Only the accelerated run removes wall-clock waits.
Normal executables and source files are not changed by instrumentation.

Configure/build both checkouts with the same options first. For example, from
the repository root, create a detached baseline at the PR's original base:

```sh
git worktree add --detach ../zesarux-ay-baseline d3502387e9fb6d19d8272e1a57fd7598f5730052
```

Build `src/` in each checkout using the configuration above, then from this
checkout's `src/` run:

```sh
python3 tests/check_ay_upstream.py ../../zesarux-ay-baseline/src /tmp/ay-upstream-check
python3 tests/check_ay_upstream.py ../../zesarux-ay-baseline/src /tmp/ay-native-default \
  --realtime --frames 150 --kind tone --machine 48k --machine 128k --machine pentagon
```

The macOS harness uses the architecture of each existing executable and requires
configured Makefiles and built objects. The baseline's tracked sources must be
clean. Default coverage is 18 cases: three fixture types, 500 frames each, across
48K, 128K, +2, +2A, +3 and Pentagon. All matched locally. The three additional
real-time CoreAudio cases also matched for 150 frames each, and both builds
reported the legacy 15,600 Hz format. This is not a hardware audio capture or a
comparison of device-callback scheduling.

### Independent reference and adapter/callback checks

```sh
CC=clang tests/ay_hq_reference.sh
CC=clang tests/ay_hq_coreaudio.sh
CC=clang SANITIZERS=thread tests/ay_hq_coreaudio.sh
python3 tests/check_ay_coreaudio.py /tmp/ay-coreaudio-lifecycle
```

Ayumi is vendored only under `tests/reference/ayumi/`, pinned to commit
`07c08b4874c359169e4a028edf73f046d8b763e2`, with its MIT license and checksums.
Only trailing whitespace is normalized. Its chip-step mixer independently
checks 292,762 pre-filter ticks, including all envelopes, repeated R13 writes,
randomized writes between ticks, all mixer combinations and volume levels.
Three steady-tone output comparisons yield normalized integer-lag correlations
of 0.9962–0.9985. Output filters differ and both generators use the Ayumi DAC
table: these are control-logic/software-output checks, not independent DAC or
hardware measurements.

The adapter suite also checks six machine configurations, all stereo/custom
positions, volume/mute, auxiliary mixing, reset/reinitialization, frozen-time
pause behavior and the complete current fallback eligibility list. CoreAudio
callback tests include the actual production implementation and need no audio
device. ASan/UBSan and ThreadSanitizer runs cover overflow, underrun zero-fill,
pause discard, resume, shutdown, counter wrap and 200,000 concurrent frames.

The optional native lifecycle harness uses localhost ZRCP and generated media,
without a window. It verifies CPU/memory pause and resume, reset/SNA restore and
machine switching through 48K, 128K, +3 and Pentagon. The local run generated
474,185 HQ frames with zero drops; it reported 329 underrun callbacks in the
session containing intentional debugger stops. Headless ZRCP stops do not
always clear `audio_playing`, so this run must not be cited as zero-underrun
playback. Separate three-second steady-HQ, reduced-core fallback and two-AY
fallback runs each reported zero underruns/drops. GUI pause behavior is not
established by these headless tests.

A separate configuration with `--disable-coreaudio --disable-cocoa` built and
ran the generated mixed fixture with null audio/video on Apple Silicon macOS.
Other operating systems/backends and a broad real-software corpus remain
unverified. Generated traces, media and detailed local results are kept outside
the source-only PR patch.

## Pentagon 128 and demo regression

Pentagon HQ uses the same 71,680 CPU T-states/frame and existing 50 Hz pacing as
the emulator's legacy path. The AY clock is nominally 1,750,000 Hz in HQ only;
legacy `ay_chip_frequency` and the other machines' sound remain unchanged. The
result is exactly 882 output samples per video frame, without changing any Z80
cycle accounting, interrupts, paging or demo data. Switching machine rebuilds
the HQ clock/filter state. The earlier WAV comparison used the standard Spectrum
1,773,400 Hz clock; a Pentagon run consequently has a slightly lower pitch.

The FIR renderer is built with `AY_HQ_CFLAGS=-O2` by default, independently of
other emulator objects, to keep resampling within the real-time frame budget.
For debugging it can be overridden explicitly, for example
`make -B ay_hq.o AY_HQ_CFLAGS=-O0`. Normal C floating-point rules apply; fast-math
is not enabled. Underrun diagnostics report missing frames and device times.
During emulator shutdown the callback sends silence once the CPU producer stops.

`tests/check_ay_hq_demo.py` builds a separate instrumented executable, provides
ROMs in its temporary runtime directory, and compares a legacy run against an
HQ run. The production executable is not instrumented or modified by the test.
At every emulated frame it compares CPU register/T-state hashes, all eight RAM
banks, the real-video raster buffer, and kernel state. Music counters and the
kernel ISR counter are checked for skips, duplicates and restarts while playing.
Only wall-clock waits are removed in this accelerated test; emulated CPU timing
and interrupts remain intact. The HQ run writes to a test sink rather than
CoreAudio; the actual CoreAudio driver needs a separate real-time smoke test.

Example for the development snapshot:

```sh
python3 tests/check_ay_hq_demo.py /absolute/path/megademo.sna \
  /absolute/path/megademo_sna.sym /tmp/pentagon-demo-check --frames 16000
```

The complete SNA stops after p04 and the closing cards. For the release's p05
handoff, first boot a copy of the TRD in real TR-DOS and use ZRCP CPU stepping to
stop at the release kernel entry (`PC=8000H`). Save that 128K snapshot, retaining
the initialized BASIC/TR-DOS environment. Then compare both runs with the same
saved entry and disk copy (the test disables persistent TRD writes):

```sh
python3 tests/check_ay_hq_demo.py /absolute/path/release-boot.sna \
  /absolute/path/megademo.sym /tmp/pentagon-release-check \
  --trd /absolute/path/megademo-copy.trd --frames 22000
```

For ordinary interactive playback, launch the built emulator with
`--machine pentagon --snap-no-change-machine --ao coreaudio --ay-hq-coreaudio`.
The `--snap-no-change-machine` option is essential: otherwise loading a 128K SNA
can switch the machine away from Pentagon. Use `--ay-stereo-mode 1` for ACB.

Pentagon HQ also retains one early host-timer deadline when it arrives during
frame computation. The generic Spectrum loop otherwise clears the timer flag
at opcode boundaries, and the end of that same frame can wait for a second
20 ms deadline. The HQ-only credit is consumed exactly once at the frame end;
a tick waking an already waiting frame creates no extra credit. Unit tests
cover both cases. This changes host pacing for Pentagon HQ, not the CPU T-state
budget or emulated interrupt position. Other machines and legacy AY keep their
existing pacing.

The ENCORE release regression covers 22,000 emulated frames including all five
parts and the TR-DOS p05 load, with identical CPU/RAM/video state and no music or
ISR counter anomalies while playing. A separate 45-second production CoreAudio
run crossing p03 -> p04 produced 1,981,851 HQ stereo frames, with zero underrun
callbacks and zero dropped frames. The local report is under
`comparisons/pentagon-demo/` (generated audio/traces are not committed).
