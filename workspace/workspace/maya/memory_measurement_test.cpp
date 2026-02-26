/**
 * Memory Measurement Test for Issue #41
 *
 * This program measures actual memory usage of the bucket cache
 * to validate theoretical calculations.
 */

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <sstream>

// Hash function for std::pair<std::string, int>
struct PairHash {
    std::size_t operator()(const std::pair<std::string, int>& p) const {
        std::size_t h1 = std::hash<std::string>{}(p.first);
        std::size_t h2 = std::hash<int>{}(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

// Function to get current process memory usage (macOS)
size_t getCurrentRSS() {
    FILE* fp = popen("ps -o rss= -p $(echo $$)", "r");
    if (!fp) return 0;

    char buffer[128];
    size_t rss = 0;
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        rss = std::atoll(buffer) * 1024; // Convert KB to bytes
    }
    pclose(fp);
    return rss;
}

int main() {
    std::cout << "=== Memory Measurement Test (Issue #41) ===" << std::endl;
    std::cout << std::endl;

    // Test parameters
    const int NUM_ENTRIES = 100000;
    const int AVG_INDEX_LENGTH = 10;

    // Measure baseline memory
    size_t baseline = getCurrentRSS();
    std::cout << "Baseline memory: " << (baseline / 1024 / 1024) << " MB" << std::endl;

    // Create cache structure (simulating 20-bucket implementation)
    std::unordered_map<int, std::unordered_set<std::pair<std::string, int>, PairHash>> bucket_cache;

    std::cout << "Building cache with " << NUM_ENTRIES << " entries..." << std::endl;

    // Simulate uniform distribution across 20 buckets
    const int NUM_BUCKETS = 20;
    for (int i = 0; i < NUM_ENTRIES; i++) {
        int bucket_id = i % NUM_BUCKETS;

        // Generate index string of average length
        std::ostringstream oss;
        oss << "key_" << (i / NUM_BUCKETS);
        std::string index = oss.str();

        // Ensure minimum length
        while (index.length() < AVG_INDEX_LENGTH) {
            index += "x";
        }

        int value = i;

        bucket_cache[bucket_id].insert({index, value});

        // Report progress every 10K entries
        if ((i + 1) % 10000 == 0) {
            size_t current = getCurrentRSS();
            size_t usage = current - baseline;
            std::cout << "  " << (i + 1) << " entries: "
                      << (usage / 1024 / 1024) << " MB" << std::endl;
        }
    }

    // Final measurement
    size_t final = getCurrentRSS();
    size_t usage = final - baseline;

    std::cout << std::endl;
    std::cout << "=== Results ===" << std::endl;
    std::cout << "Total entries: " << NUM_ENTRIES << std::endl;
    std::cout << "Number of buckets: " << NUM_BUCKETS << std::endl;
    std::cout << "Entries per bucket: " << (NUM_ENTRIES / NUM_BUCKETS) << std::endl;
    std::cout << "Average index length: " << AVG_INDEX_LENGTH << " chars" << std::endl;
    std::cout << std::endl;
    std::cout << "Total memory usage: " << (usage / 1024 / 1024) << " MB" << std::endl;
    std::cout << "Memory per entry: " << (usage / NUM_ENTRIES) << " bytes" << std::endl;
    std::cout << std::endl;

    // Calculate theoretical values
    size_t theoretical_per_entry = 64; // From audit report
    size_t theoretical_total = NUM_ENTRIES * theoretical_per_entry;

    std::cout << "=== Comparison ===" << std::endl;
    std::cout << "Theoretical per entry: " << theoretical_per_entry << " bytes" << std::endl;
    std::cout << "Theoretical total: " << (theoretical_total / 1024 / 1024) << " MB" << std::endl;
    std::cout << "Actual total: " << (usage / 1024 / 1024) << " MB" << std::endl;
    std::cout << "Difference: " << (int)((usage - theoretical_total) * 100.0 / theoretical_total) << "%" << std::endl;

    // Test single bucket (worst-case collision)
    std::cout << std::endl;
    std::cout << "=== Single Bucket Test (Worst-Case Collision) ===" << std::endl;

    size_t baseline2 = getCurrentRSS();
    std::unordered_set<std::pair<std::string, int>, PairHash> single_bucket;

    for (int i = 0; i < NUM_ENTRIES; i++) {
        std::ostringstream oss;
        oss << "key_" << i;
        std::string index = oss.str();

        while (index.length() < AVG_INDEX_LENGTH) {
            index += "x";
        }

        single_bucket.insert({index, i});
    }

    size_t final2 = getCurrentRSS();
    size_t usage2 = final2 - baseline2;

    std::cout << "Single bucket with " << NUM_ENTRIES << " entries: "
              << (usage2 / 1024 / 1024) << " MB" << std::endl;
    std::cout << "Per entry: " << (usage2 / NUM_ENTRIES) << " bytes" << std::endl;

    return 0;
}
