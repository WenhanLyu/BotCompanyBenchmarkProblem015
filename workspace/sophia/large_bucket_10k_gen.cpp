#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

int hash_bucket(const std::string& index) {
    const int NUM_BUCKETS = 20;
    uint32_t hash = 0;
    for (char c : index) {
        hash = hash * 31u + static_cast<uint32_t>(static_cast<unsigned char>(c));
    }
    return static_cast<int>(hash % NUM_BUCKETS);
}

int main() {
    std::vector<std::string> bucket_0_keys;

    // Find 10,000 keys that all hash to bucket 0
    for (int i = 0; i < 500000; i++) {
        std::string key = "key_" + std::to_string(i);
        if (hash_bucket(key) == 0) {
            bucket_0_keys.push_back(key);
            if (bucket_0_keys.size() >= 10000) {
                break;
            }
        }
    }

    std::cerr << "Found " << bucket_0_keys.size() << " keys for bucket 0" << std::endl;

    int num_ops = bucket_0_keys.size() + 10;
    std::cout << num_ops << std::endl;

    // Insert all keys
    for (size_t i = 0; i < bucket_0_keys.size(); i++) {
        std::cout << "insert " << bucket_0_keys[i] << " " << i << std::endl;
    }

    // Find operations
    for (int i = 0; i < 10; i++) {
        std::cout << "find " << bucket_0_keys[i * 100] << std::endl;
    }

    return 0;
}
