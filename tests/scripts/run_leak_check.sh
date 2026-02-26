#!/bin/bash

# Run the program with malloc stack logging
export MallocStackLogging=1
export MallocStackLoggingNoCompact=1

# Run the program and capture its PID
./code < valgrind_test_input.txt &
PID=$!

# Wait a bit for the program to start
sleep 0.5

# Run leaks while program is running
leaks $PID > leaks_during.txt 2>&1

# Wait for program to finish
wait $PID
EXIT_CODE=$?

# Check leaks after program exits (using the executable)
leaks ./code > leaks_after.txt 2>&1

echo "Exit code: $EXIT_CODE"
echo ""
echo "=== Leaks check during execution ==="
cat leaks_during.txt
echo ""
echo "=== Leaks check after execution ==="
cat leaks_after.txt
