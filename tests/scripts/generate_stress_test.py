#!/usr/bin/env python3
"""
Generate comprehensive stress tests for file storage system
Tests memory limits (5-6 MiB), time limits, and edge cases
"""

import sys

def generate_max_memory_test(filename, num_entries=100000):
    """
    Generate test that approaches 100K entries limit
    This tests the memory constraint most directly
    """
    with open(filename, 'w') as f:
        # Total operations: inserts + finds + deletes
        total_ops = num_entries + 100  # 100 extra for find/delete operations
        f.write(f"{total_ops}\n")

        # Insert entries with diverse keys to test hash distribution
        for i in range(num_entries):
            # Use shorter keys to fit more data
            key = f"k{i}"
            value = i
            f.write(f"insert {key} {value}\n")

        # Perform some finds
        for i in range(0, num_entries, num_entries//50):
            f.write(f"find k{i}\n")

        # Perform some deletes
        for i in range(0, num_entries, num_entries//50):
            f.write(f"delete k{i} {i}\n")

def generate_worst_case_bucket_test(filename):
    """
    Test worst-case scenario: many entries in same bucket
    This could trigger MLE (Memory Limit Exceeded)
    """
    with open(filename, 'w') as f:
        # Try to create keys that hash to the same bucket
        # Using similar prefixes might increase collision likelihood
        num_entries = 10000  # Large enough to stress one bucket

        f.write(f"{num_entries + 100}\n")

        # Keys with same prefix (might hash similarly)
        for i in range(num_entries):
            key = f"bucket_collision_test_{i:06d}"
            value = i
            f.write(f"insert {key} {value}\n")

        # Find operations on colliding keys
        for i in range(0, num_entries, 100):
            f.write(f"find bucket_collision_test_{i:06d}\n")

def generate_max_string_length_test(filename):
    """
    Test maximum string length (64 bytes) with many entries
    """
    with open(filename, 'w') as f:
        num_entries = 5000
        f.write(f"{num_entries * 2}\n")

        for i in range(num_entries):
            # 64-byte strings (maximum allowed)
            # Format: "key_" + 60 characters
            key = f"key_{i:060d}"  # Total: 4 + 60 = 64 bytes
            value = i * 2
            f.write(f"insert {key} {value}\n")

            # Also insert another value for same key
            f.write(f"insert {key} {value + 1}\n")

def generate_many_values_per_key_test(filename):
    """
    Test many values for the same key (stress sorting)
    """
    with open(filename, 'w') as f:
        num_keys = 100
        values_per_key = 500

        total_ops = (num_keys * values_per_key) + num_keys
        f.write(f"{total_ops}\n")

        # Insert many values for each key
        for key_id in range(num_keys):
            for value in range(values_per_key):
                f.write(f"insert key{key_id} {value}\n")

        # Find all keys (will need to sort 500 values each)
        for key_id in range(num_keys):
            f.write(f"find key{key_id}\n")

def generate_alternating_ops_test(filename):
    """
    Test alternating insert/find/delete operations
    This stresses file I/O and bucket rewriting
    """
    with open(filename, 'w') as f:
        num_cycles = 10000
        total_ops = num_cycles * 3

        f.write(f"{total_ops}\n")

        for i in range(num_cycles):
            f.write(f"insert key{i % 1000} {i}\n")
            f.write(f"find key{i % 1000}\n")
            if i > 0:
                f.write(f"delete key{(i-1) % 1000} {i-1}\n")

def generate_large_value_test(filename):
    """
    Test with maximum int values
    """
    with open(filename, 'w') as f:
        num_entries = 5000
        f.write(f"{num_entries * 2}\n")

        max_int = 2147483647  # INT_MAX

        for i in range(num_entries):
            key = f"key{i}"
            # Use large values near INT_MAX
            value = max_int - i
            f.write(f"insert {key} {value}\n")
            f.write(f"find {key}\n")

if __name__ == "__main__":
    print("Generating stress tests...")

    print("1. Maximum memory test (100K entries)...")
    generate_max_memory_test("stress_100k.txt", 100000)

    print("2. Worst-case bucket collision test...")
    generate_worst_case_bucket_test("stress_bucket_collision.txt")

    print("3. Maximum string length test...")
    generate_max_string_length_test("stress_max_string.txt")

    print("4. Many values per key test...")
    generate_many_values_per_key_test("stress_many_values.txt")

    print("5. Alternating operations test...")
    generate_alternating_ops_test("stress_alternating.txt")

    print("6. Large value test...")
    generate_large_value_test("stress_large_values.txt")

    print("\nAll stress tests generated successfully!")
    print("\nTest files created:")
    print("  - stress_100k.txt (100K operations)")
    print("  - stress_bucket_collision.txt (collision scenario)")
    print("  - stress_max_string.txt (64-byte strings)")
    print("  - stress_many_values.txt (sorting stress)")
    print("  - stress_alternating.txt (I/O stress)")
    print("  - stress_large_values.txt (INT_MAX values)")
