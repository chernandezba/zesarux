#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -Isoundchips soundchips/ay_hq.c tests/ay_hq_test.c -lm -o "$test_dir/ay_hq_test"
"$test_dir/ay_hq_test" "$@"
"${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -fsigned-char \
    -fsanitize=address,undefined -I. -Iaudio -Isoundchips -Ivideo -Icores \
    -Istorage -Izrcp -Imachines -Icpus -Icopy_interfaces -Imenu -Izxvision -Isnap -Ivideo_chips -Ithird_party soundchips/ay_hq.c audio/audio_ay_hq.c \
    tests/ay_hq_integration_test.c -lm -o "$test_dir/ay_hq_integration_test"
"$test_dir/ay_hq_integration_test"
