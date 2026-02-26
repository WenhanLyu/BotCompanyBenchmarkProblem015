#ifndef BUCKET_MANAGER_H
#define BUCKET_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

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
    static const int NUM_BUCKETS = 5000;

    // In-memory hash index for O(1) duplicate checking
    // Maps bucket_id -> set of (index, value) pairs
    // Lazy-loaded on first access to each bucket
    std::unordered_map<int, std::unordered_set<std::pair<std::string, int>, PairHash>> bucket_cache_;

    // Compute hash bucket for an index
    int hash_bucket(const std::string& index) const;

    // Get bucket file name for a given bucket ID
    std::string get_bucket_filename(int bucket_id) const;

    // Load bucket cache from file (called lazily on first access)
    void load_bucket_cache(int bucket_id);

    // Load all entries from a bucket file into memory
    std::vector<Entry> load_bucket(int bucket_id);

    // Save entries back to a bucket file (overwrites existing file)
    void save_bucket(int bucket_id, const std::vector<Entry>& entries);

    // Append a single entry to a bucket file
    void append_to_bucket(int bucket_id, const Entry& entry);
};

#endif // BUCKET_MANAGER_H
