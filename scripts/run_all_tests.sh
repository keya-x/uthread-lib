#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

echo "=== Building uthread-lib ==="
make clean
make

echo ""
echo "=== Running Unit Tests ==="
for test in bin/test_*; do
    if [ -x "$test" ]; then
        echo "Running $test..."
        ./"$test"
    fi
done

echo ""
echo "=== Running Stress Tests ==="
for test in bin/stress_*; do
    if [ -x "$test" ]; then
        echo "Running $test..."
        ./"$test"
    fi
done

echo ""
echo "=== All tests completed successfully ==="
