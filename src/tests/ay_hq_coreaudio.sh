#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
if [ "$(uname -s)" != Darwin ]; then
    echo 'CoreAudio callback test requires macOS' >&2
    exit 1
fi
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
"${CC:-clang}" -std=c11 -O2 -Wall -Wextra -Werror -Wno-unused-but-set-variable \
    -fsigned-char -fsanitize="${SANITIZERS:-address,undefined}" -Wl,-dead_strip \
    -I. -Iaudio -Isoundchips -Ivideo -Icores -Istorage -Izrcp -Imachines \
    -Icpus -Icopy_interfaces -Imenu -Izxvision -Isnap -Ivideo_chips -Ithird_party \
    tests/ay_hq_coreaudio_test.c -o "$test_dir/coreaudio" \
    -framework CoreAudio -framework AudioUnit -framework AudioToolbox \
    -framework CoreServices -framework CoreMIDI -lpthread
"$test_dir/coreaudio"
