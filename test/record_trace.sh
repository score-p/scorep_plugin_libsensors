#!/usr/bin/env bash
set -euo pipefail

# verify preconditions
test -x ./sleep5 || (echo '[!!] sleep5 program not available, run make' >&2 && exit 1)
otf2-print --help >/dev/null 2>&1 || (echo '[!!] otf2-print not available' >&2 && exit 1)

# ensure empty playground
test ! -e trace_no_plugin || (echo '[!!] trace_no_plugin must not exist, run rm' >&2 && exit 1)
test ! -e trace_with_plugin || (echo '[!!] trace_with_plugin must not exist, run rm' >&2 && exit 1)

# general settings
export SCOREP_ENABLE_PROFILING=0
export SCOREP_ENABLE_TRACING=1
export SCOREP_TOTAL_MEMORY=4095M

export TEST_METRIC_NAME=$(scorep-plugin-libsensors-print-sensors | \
                              tail -n+2 | \
                              head -n1 | \
                              cut -d ';' -f 2 | \
                              sed 's/^"//;s/"$//g')

echo "selected metric $TEST_METRIC_NAME"

echo "setup completed, running tests"

# reference run (without plugin)
echo "[1/2] reference run (no plugin)..."
SCOREP_EXPERIMENT_DIRECTORY=trace_no_plugin ./sleep5
test -d trace_no_plugin || (echo '[!!] scorep did not create trace (without plugin), did you compile with scorep-gcc?' >&2 && exit 1)
test -f trace_no_plugin/traces.otf2 || (echo '[!!] no trace file found (run without plugin)' >&2 && exit 1)

( ! (otf2-print trace_no_plugin/traces.otf2 | sed -n -e '/^METRIC/p' | grep "$TEST_METRIC_NAME" >/dev/null)) || (echo '[!!] reference trace contains unexpected metric for '"$TEST_METRIC_NAME" >&2 && exit 1)

echo "  OK"

# run with plugin
echo "[2/2] validation run (with plugin)..."
export SCOREP_METRIC_PLUGINS=sensors_plugin
export SCOREP_METRIC_SENSORS_PLUGIN="$TEST_METRIC_NAME"

SCOREP_EXPERIMENT_DIRECTORY=trace_with_plugin ./sleep5
test -d trace_with_plugin || (echo '[!!] scorep did not create trace (with plugin)' >&2 && exit 1)
test -f trace_with_plugin/traces.otf2 || (echo '[!!] no trace file found (run with plugin)' >&2 && exit 1)

(otf2-print trace_with_plugin/traces.otf2 | sed -n -e '/^METRIC/p' | grep "$TEST_METRIC_NAME" >/dev/null) || (echo '[!!] trace does not contain metric '"$TEST_METRIC_NAME" >&2 && exit 1)


echo "  OK"
echo ""
echo "all tests completed"
echo "maybe clean up manually (?)"
