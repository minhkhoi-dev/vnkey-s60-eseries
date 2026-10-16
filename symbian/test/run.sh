#!/usr/bin/env bash
# Builds and runs the Telex engine's tests with the host's C++ compiler (clang++ on a Mac,
# g++ on Linux or WSL), in C++98 as GCCE would see the code.
set -euo pipefail
cd "$(dirname "$0")/.."
out="${TMPDIR:-/tmp}/vntelex_test"
"${CXX:-c++}" -std=c++98 -Wall -Wextra -pedantic -O1 -I test/shim -I inc \
    src/VnTelex.cpp test/VnTelexTest.cpp -o "$out"
"$out"
