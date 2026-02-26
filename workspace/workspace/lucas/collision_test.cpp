// Generate a small collision test to verify insert_entry O(n²) behavior
// This test creates keys that all hash to the same bucket
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// Copy of hash function from bucket_manager.cpp
int hash_bucket(const std::string& index) {
    uint32_t hash = 0;
    for (char c : index) {
        hash = hash * 31u + static_cast<uint32_t>(static_cast<unsigned char>(c));
    }
    return static_cast<int>(hash % 20);
}

int main() {
    const int TARGET_BUCKET = 0;
    const int NUM_KEYS = 100; // Small test for quick verification

    std::vector<std::string> collision_keys;

    // Find keys that all hash to bucket 0
    for (int i = 0; collision_keys.size() < NUM_KEYS; i++) {
        std::string candidate = "key" + std::to_string(i);
        if (hash_bucket(candidate) == TARGET_BUCKET) {
            collision_keys.push_back(candidate);
        }
    }

    // Generate test file
    std::ofstream out("collision_test_input.txt");

    // All inserts to same bucket
    out << (NUM_KEYS * 10) << std::endl;

    for (int i = 0; i < NUM_KEYS; i++) {
        for (int v = 0; v < 10; v++) {
            out << "insert " << collision_keys[i] << " " << v << std::endl;
        }
    }

    out.close();

    std::cout << "Generated collision test with " << NUM_KEYS << " keys, all hashing to bucket " << TARGET_BUCKET << std::endl;
    std::cout << "Total operations: " << (NUM_KEYS * 10) << std::endl;

    return 0;
}
