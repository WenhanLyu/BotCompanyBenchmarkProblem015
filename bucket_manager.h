#ifndef BUCKET_MANAGER_H
#define BUCKET_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <cstdint>

// Compact entry structure (16 bytes) for in-memory cache
struct CompactEntry {
    uint32_t key_hash;     // hash(index) for fast lookup
    int32_t value;         // the value
    int64_t file_offset;   // offset in file for full string verification

    CompactEntry(uint32_t hash, int32_t val, int64_t offset)
        : key_hash(hash), value(val), file_offset(offset) {}
};

// BucketManager handles file-based storage with hash bucketing
class BucketManager {
public:
    // Constructor: initializes the bucket manager
    BucketManager();

    // Insert an entry into the appropriate bucket
    // Prevents duplicate (index, value) pairs per spec
    // Uses unbounded cache with lazy-loading
    // Complexity: O(1) after bucket is loaded
    void insert_entry(const std::string& index, int value);

    // Find all values for a given index, returned in sorted order
    // Returns empty vector if no matching entries found
    // Uses unbounded cache with lazy-loading
    // Complexity: O(1) lookup + O(k log k) sort where k = number of matches
    std::vector<int> find_values(const std::string& index);

    // Delete an entry with the given index and value
    // If the entry doesn't exist, operation is silently ignored
    // Uses in-place tombstone marking
    // Complexity: O(1) cache lookup + O(1) file write
    void delete_entry(const std::string& index, int value);

private:
    static const int NUM_BUCKETS = 10;  // Multi-bucket architecture

    // Per-bucket unbounded cache: maps key_hash -> vector of CompactEntry
    // Lazy-loaded on first access to each bucket
    std::array<std::unordered_map<uint32_t, std::vector<CompactEntry>>, NUM_BUCKETS> bucket_cache_;

    // Track which buckets have been loaded
    std::array<bool, NUM_BUCKETS> bucket_loaded_;

    // Compute hash of a string for bucketing and cache lookup
    uint32_t compute_hash(const std::string& index) const;

    // Get bucket number for an index
    int get_bucket_number(const std::string& index) const;

    // Get the data file name for a bucket
    std::string get_bucket_filename(int bucket_num) const;

    // Load a bucket file into cache (lazy-load on first access)
    void load_bucket(int bucket_num);

    // Verify full string at file offset (for hash collision handling)
    bool verify_entry_at_offset(int bucket_num, int64_t offset, const std::string& index) const;
};

#endif // BUCKET_MANAGER_H
