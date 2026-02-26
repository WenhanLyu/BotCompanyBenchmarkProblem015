#ifndef BUCKET_MANAGER_H
#define BUCKET_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <bitset>

// Hash function for std::pair<std::string, int> used in bucket cache
struct PairHash {
    std::size_t operator()(const std::pair<std::string, int>& p) const {
        // Combine hashes using FNV-1a-style mixing
        std::size_t h1 = std::hash<std::string>{}(p.first);
        std::size_t h2 = std::hash<int>{}(p.second);
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

// Entry structure representing a key-value pair
struct Entry {
    std::string index;
    int value;
    bool active;  // false if deleted (tombstone)

    Entry(const std::string& idx, int val, bool act = true)
        : index(idx), value(val), active(act) {}
};

// BucketManager handles file-based storage with hash bucketing
class BucketManager {
public:
    // Constructor: initializes the bucket manager
    BucketManager();

    // Insert an entry into the appropriate bucket
    // Prevents duplicate (index, value) pairs per spec
    // Uses streaming file reads to minimize memory usage
    // Complexity: O(bucket_size) - streams through file to check for duplicates
    void insert_entry(const std::string& index, int value);

    // Find all values for a given index, returned in sorted order
    // Returns empty vector if no matching entries found
    // Uses streaming file reads to minimize memory usage
    // Complexity: O(bucket_size + k log k) where k = number of matches
    std::vector<int> find_values(const std::string& index);

    // Delete an entry with the given index and value
    // If the entry doesn't exist, operation is silently ignored
    // Uses streaming file reads with temporary file for rewrite
    // Complexity: O(bucket_size) - streams through file to rewrite without deleted entry
    void delete_entry(const std::string& index, int value);

private:
    static const int NUM_BUCKETS = 1;  // Single file approach
    static const size_t MAX_INDEX_ENTRIES = 15000;  // Bounded cache limit
    static const size_t BLOOM_FILTER_SIZE = 800000;  // ~100 KB for 1% FP rate

    // In-memory index for O(1) duplicate checking with LRU eviction
    // Maps (index, value) -> file offset for bounded caching
    std::unordered_map<std::pair<std::string, int>, int64_t, PairHash> index_;

    // LRU tracking structures
    std::list<std::pair<std::string, int>> lru_list_;  // Most recent at front
    std::unordered_map<std::pair<std::string, int>,
                       std::list<std::pair<std::string, int>>::iterator,
                       PairHash> lru_pos_;

    // Bloom filter for probabilistic duplicate detection
    // Never has false negatives: if says "not present", guaranteed absent
    // ~1% false positive rate with 3 hash functions
    std::bitset<BLOOM_FILTER_SIZE> bloom_filter_;

    // Get the data file name (always "data.bin")
    std::string get_data_filename() const;

    // Update LRU list when an entry is accessed
    void update_lru(const std::pair<std::string, int>& key);

    // Evict least recently used entry if cache is full
    void evict_lru_if_needed();

    // Check if entry exists in file (for cache misses)
    bool check_file_for_duplicate(const std::string& index, int value);

    // Bloom filter operations
    // Compute hash functions for bloom filter (k=3 for optimal FP rate)
    size_t bloom_hash1(const std::string& index, int value) const;
    size_t bloom_hash2(const std::string& index, int value) const;
    size_t bloom_hash3(const std::string& index, int value) const;

    // Add entry to bloom filter
    void bloom_add(const std::string& index, int value);

    // Check if entry might exist (true = maybe, false = definitely not)
    bool bloom_contains(const std::string& index, int value) const;

    // Load all entries from the data file into memory (for delete operations)
    std::vector<Entry> load_all_entries();

    // Save entries back to the data file (overwrites existing file)
    void save_all_entries(const std::vector<Entry>& entries);
};

#endif // BUCKET_MANAGER_H
