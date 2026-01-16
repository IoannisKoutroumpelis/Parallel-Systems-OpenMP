#!/bin/bash

mkdir -p test

echo "Running tests..."

# =========================
# Test για Άσκηση 2.1 (q1)
# =========================
echo "Running q1..."

echo "q1 tests" > test/q1_test.txt
./bin/q1 100000 3 >> test/q1_test.txt
./bin/q1 100000 4 >> test/q1_test.txt
./bin/q1 100000 5 >> test/q1_test.txt
./bin/q1 100000 7 >> test/q1_test.txt

# =========================
# Test για Άσκηση 2.2 (q2)
# =========================
echo "Running q2..."

echo "q2 tests" > test/q2_test.txt
./bin/q2 1000 70 10 4 >> test/q2_test.txt
./bin/q2 1000 70 10 6 >> test/q2_test.txt
./bin/q2 1000 70 10 8 >> test/q2_test.txt
./bin/q2 1000 70 10 11 >> test/q2_test.txt
./bin/q2 1000 10 10 4 >> test/q2_test.txt
./bin/q2 1000 30 10 4 >> test/q2_test.txt
./bin/q2 1000 60 10 4 >> test/q2_test.txt
./bin/q2 1000 80 10 4 >> test/q2_test.txt
./bin/q2 1000 99 10 4 >> test/q2_test.txt
./bin/q2 10000 10 10 4 >> test/q2_test.txt
./bin/q2 10000 50 10 4 >> test/q2_test.txt
./bin/q2 10000 75 10 4 >> test/q2_test.txt
./bin/q2 10000 99 10 4 >> test/q2_test.txt

# =========================
# Test για Άσκηση 2.3 (q3)
# =========================
echo "Running q3..."

echo "q3 tests" > test/q3_test.txt
./bin/q3 10000000 serial 1 >> test/q3_test.txt
./bin/q3 10000000 parallel 4 >> test/q3_test.txt
./bin/q3 10000000 parallel 6 >> test/q3_test.txt
./bin/q3 10000000 parallel 8 >> test/q3_test.txt
./bin/q3 10000000 parallel 10 >> test/q3_test.txt
./bin/q3 10000000 parallel 12 >> test/q3_test.txt
./bin/q3 100000000 parallel 4 >> test/q3_test.txt
./bin/q3 100000000 parallel 6 >> test/q3_test.txt
./bin/q3 100000000 parallel 8 >> test/q3_test.txt
./bin/q3 100000000 parallel 10 >> test/q3_test.txt
./bin/q3 100000000 parallel 12 >> test/q3_test.txt
./bin/q3 100000000 serial 1 >> test/q3_test.txt

echo "All tests completed."
echo "Results stored in ./test directory"
