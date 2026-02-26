#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

// Reproduce the hash function from bucket_manager.cpp
int hash_bucket(const std::string& index) {
    const int NUM_BUCKETS = 20;
    uint32_t hash = 0;
    for (char c : index) {
        hash = hash * 31u + static_cast<uint32_t>(static_cast<unsigned char>(c));
    }
    return static_cast<int>(hash % NUM_BUCKETS);
}

int main() {
    // Find keys that all hash to bucket 0 to test worst-case performance
    std::vector<std::string> bucket_0_keys;

    // Generate candidate keys and test which ones map to bucket 0
    for (int i = 0; i < 50000; i++) {
        std::string key = "key_" + std::to_string(i);
        if (hash_bucket(key) == 0) {
            bucket_0_keys.push_back(key);
            if (bucket_0_keys.size() >= 5000) {
                break;  // Get 5000 keys for bucket 0
            }
        }
    }

    std::cerr << "Found " << bucket_0_keys.size() << " keys that hash to bucket 0" << std::endl;

    // Generate test: insert all these keys (worst case for O(n^2))
    int num_ops = bucket_0_keys.size() + 100;  // inserts + 100 finds
    std::cout << num_ops << std::endl;

    // Insert all keys to bucket 0
    for (size_t i = 0; i < bucket_0_keys.size(); i++) {
        std::cout << "insert " << bucket_0_keys[i] << " " << i << std::endl;
    }

    // Find operations
    for (int i = 0; i < 100; i++) {
        std::cout << "find " << bucket_0_keys[i] << std::endl;
    }

    return 0;
}
