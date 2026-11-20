#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -Isoundchips soundchips/ay_hq.c tests/ay_hq_reference_test.c -lm -o "$test_dir/reference"
"$test_dir/reference"
