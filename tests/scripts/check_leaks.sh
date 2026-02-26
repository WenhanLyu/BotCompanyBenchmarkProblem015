#!/bin/bash

# Run the program with test input in background
./code < test_input.txt &
PID=$!

# Wait a moment for program to do some work
sleep 0.2

# Check for leaks
leaks $PID 2>&1

# Wait for program to finish
wait $PID
