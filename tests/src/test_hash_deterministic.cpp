#include <iostream>
#include <string>
#include <cstdint>

int hash_bucket(const std::string& index) {
    const int NUM_BUCKETS = 20;
    // FNV-1a hash - deterministic and portable across platforms
    uint64_t hash = 14695981039346656037ULL;
    const uint64_t fnv_prime = 1099511628211ULL;
    
    for (char c : index) {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(c));
        hash *= fnv_prime;
    }
    
    return static_cast<int>(hash % NUM_BUCKETS);
}

int main() {
    std::string test_keys[] = {"book1", "book2", "book3", "test", "a", "b", "CppPrimer", "Dune"};
    
    for (const auto& key : test_keys) {
        int bucket = hash_bucket(key);
        std::cout << "Key: " << key << " -> Bucket: " << bucket << std::endl;
    }
    
    // Test multiple times to ensure determinism
    std::cout << "\nTesting determinism (book1 should always go to same bucket):" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << "Run " << i+1 << ": bucket = " << hash_bucket("book1") << std::endl;
    }
    
    return 0;
}
