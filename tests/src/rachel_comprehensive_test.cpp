#include <iostream>
#include <string>
#include <cstdint>
#include <vector>
#include <set>
#include <fstream>

// Same hash function as in bucket_manager.cpp
int hash_bucket(const std::string& index) {
    const int NUM_BUCKETS = 20;
    uint64_t hash = 14695981039346656037ULL;
    const uint64_t fnv_prime = 1099511628211ULL;

    for (char c : index) {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        hash *= fnv_prime;
    }

    return static_cast<int>(hash % NUM_BUCKETS);
}

int main() {
    // Generate keys that will hit all 20 buckets
    std::vector<std::string> keys_for_all_buckets;
    std::set<int> covered_buckets;

    // Try different keys until we cover all 20 buckets
    for (int i = 0; i < 10000 && covered_buckets.size() < 20; i++) {
        std::string key = "key" + std::to_string(i);
        int bucket = hash_bucket(key);
        if (covered_buckets.find(bucket) == covered_buckets.end()) {
            covered_buckets.insert(bucket);
            keys_for_all_buckets.push_back(key);
        }
    }

    std::cout << "Found keys for all " << covered_buckets.size() << " buckets\n\n";

    // Generate test input file
    std::ofstream out("rachel_all_buckets_test.txt");
    out << keys_for_all_buckets.size() << "\n";
    for (size_t i = 0; i < keys_for_all_buckets.size(); i++) {
        out << "insert " << keys_for_all_buckets[i] << " " << (i * 100) << "\n";
    }
    out.close();

    std::cout << "Test Inputs:\n";
    std::cout << "============\n";
    for (size_t i = 0; i < keys_for_all_buckets.size(); i++) {
        int bucket = hash_bucket(keys_for_all_buckets[i]);
        std::cout << "insert " << keys_for_all_buckets[i] << " " << (i * 100)
                  << " -> bucket " << bucket << "\n";
    }

    return 0;
}
