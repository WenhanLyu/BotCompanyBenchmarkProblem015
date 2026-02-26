#include <iostream>
#include <string>
#include <cstdint>

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
    std::string keys[] = {
        "apple", "banana", "cherry", "date", "elderberry",
        "fig", "grape", "honeydew", "kiwi", "lemon",
        "mango", "nectarine", "orange", "papaya", "quince",
        "raspberry", "strawberry", "tangerine", "watermelon", "apricot"
    };

    std::cout << "Hash Distribution:\n";
    std::cout << "==================\n";
    for (const auto& key : keys) {
        int bucket = hash_bucket(key);
        std::cout << key << " -> bucket " << bucket << " (data_"
                  << (bucket < 10 ? "0" : "") << bucket << ".bin)\n";
    }

    return 0;
}
