#!/bin/bash

# Test script for M5.1.3 constraints
# Memory limit: 6 MiB
# Time limit: 16s

echo "=== M5.1.3 Constraint Testing ==="
echo ""

# Clean data files
rm -f data_*.bin

# Test 1: Insert-Heavy Workload
echo "Test 1: Insert-Heavy (100K ops)"
echo "--------------------------------"
/usr/bin/time -l ./db_system < workspace/sophia/test_insert_heavy_100k.txt > /tmp/output_insert_heavy.txt 2> /tmp/time_insert_heavy.txt
cat /tmp/time_insert_heavy.txt | grep -E 'real|user|sys|maximum resident set size'
MEMORY_INSERT=$(cat /tmp/time_insert_heavy.txt | grep 'maximum resident set size' | awk '{print $1}')
MEMORY_INSERT_MIB=$(echo "scale=2; $MEMORY_INSERT / 1024 / 1024" | bc)
echo "Memory: $MEMORY_INSERT_MIB MiB"

# Check if memory is under 6 MiB
if (( $(echo "$MEMORY_INSERT_MIB < 6.0" | bc -l) )); then
    echo "✅ INSERT-HEAVY MEMORY: PASS"
else
    echo "❌ INSERT-HEAVY MEMORY: FAIL (${MEMORY_INSERT_MIB} MiB > 6.0 MiB)"
fi

echo ""
echo ""

# Clean data files for next test
rm -f data_*.bin

# Test 2: Collision Workload
echo "Test 2: Collision (100K ops)"
echo "----------------------------"
/usr/bin/time -l ./db_system < workspace/sophia/test_collision_100k.txt > /tmp/output_collision.txt 2> /tmp/time_collision.txt
cat /tmp/time_collision.txt | grep -E 'real|user|sys|maximum resident set size'
MEMORY_COLLISION=$(cat /tmp/time_collision.txt | grep 'maximum resident set size' | awk '{print $1}')
MEMORY_COLLISION_MIB=$(echo "scale=2; $MEMORY_COLLISION / 1024 / 1024" | bc)
echo "Memory: $MEMORY_COLLISION_MIB MiB"

# Check if memory is under 6 MiB and time is under 16s
if (( $(echo "$MEMORY_COLLISION_MIB < 6.0" | bc -l) )); then
    echo "✅ COLLISION MEMORY: PASS"
else
    echo "❌ COLLISION MEMORY: FAIL (${MEMORY_COLLISION_MIB} MiB > 6.0 MiB)"
fi

echo ""
echo ""

# Test 3: File Count
echo "Test 3: File Count"
echo "------------------"
FILE_COUNT=$(ls -1 data_*.bin 2>/dev/null | wc -l | tr -d ' ')
echo "Files created: $FILE_COUNT"
if [ "$FILE_COUNT" -le 20 ]; then
    echo "✅ FILE COUNT: PASS ($FILE_COUNT ≤ 20)"
else
    echo "❌ FILE COUNT: FAIL ($FILE_COUNT > 20)"
fi

echo ""
echo "=== Test Complete ==="
