# Test-only Ayumi reference

`ayumi.c`, `ayumi.h` and `LICENSE` (trailing whitespace normalized; code unchanged) from Peter Sovietov's Ayumi:
https://github.com/true-grue/ayumi/tree/07c08b4874c359169e4a028edf73f046d8b763e2

Original and vendored checksums are recorded in `SOURCE.json`.

Pinned commit: `07c08b4874c359169e4a028edf73f046d8b763e2` (MIT).
These sources are compiled only into the reference test, never the emulator.
The test calls Ayumi's chip-step mixer within the test translation unit and
compares pre-filter output and oscillator states after identical timed writes.
The HQ amplitude table already originates from Ayumi, so this is an independent
control-logic implementation comparison, not independent hardware/DAC evidence.
Output filters differ; PCM is compared by normalized correlation for steady
tones, not byte equality. A hardware recording remains a separate requirement.
