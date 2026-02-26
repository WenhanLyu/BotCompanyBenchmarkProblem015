#include <iostream>
#include <string>
#include <functional>

int main() {
    const int NUM_BUCKETS = 20;

    // Try various keys to find one that hashes to bucket 0
    for (int i = 0; i < 1000; i++) {
        std::string key = "key" + std::to_string(i);
        int bucket = std::hash<std::string>{}(key) % NUM_BUCKETS;
        if (bucket == 0) {
            std::cout << key << " -> bucket " << bucket << std::endl;
        }
    }

    return 0;
}
