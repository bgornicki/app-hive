#!/usr/bin/env bash
# Generate code coverage reports from fuzzing results

set -e

SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BUILDDIR="$SCRIPTDIR/cmake-build-fuzz-coverage"
CORPUSDIR="$SCRIPTDIR/corpus"
HTMLCOVDIR="$SCRIPTDIR/html-coverage"

# Compile the fuzzer with code coverage support
rm -rf "$BUILDDIR" "$HTMLCOVDIR"
mkdir "$BUILDDIR"
cd "$BUILDDIR"
cmake -DCMAKE_C_COMPILER=clang -DCODE_COVERAGE=1 -B"$BUILDDIR" ..
cmake --build "$BUILDDIR" --target fuzz_hive

# Run the fuzzer on the corpus files
export LLVM_PROFILE_FILE="$BUILDDIR/fuzz_hive.profraw"
"$BUILDDIR/fuzz_hive" "$CORPUSDIR"/*
llvm-profdata merge --sparse "$LLVM_PROFILE_FILE" -o "$BUILDDIR/fuzz_hive.profdata"
llvm-cov show "$BUILDDIR/fuzz_hive" -instr-profile="$BUILDDIR/fuzz_hive.profdata" -show-line-counts-or-regions -output-dir="$HTMLCOVDIR" -format=html
llvm-cov report "$BUILDDIR/fuzz_hive" -instr-profile="$BUILDDIR/fuzz_hive.profdata"
