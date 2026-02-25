#ifndef BUCKET_MANAGER_H
#define BUCKET_MANAGER_H

#include <string>
#include <vector>

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
    // Complexity: O(bucket_size) - needs to check for duplicates
    void insert_entry(const std::string& index, int value);

    // Find all values for a given index, returned in sorted order
    // Returns empty vector if no matching entries found
    // Complexity: O(bucket_size + k log k) where k = number of matches
    std::vector<int> find_values(const std::string& index);

    // Delete an entry with the given index and value
    // If the entry doesn't exist, operation is silently ignored
    // Complexity: O(bucket_size) - needs to load, modify, and rewrite bucket
    void delete_entry(const std::string& index, int value);

private:
    static const int NUM_BUCKETS = 20;

    // Compute hash bucket for an index
    int hash_bucket(const std::string& index) const;

    // Get bucket file name for a given bucket ID
    std::string get_bucket_filename(int bucket_id) const;

    // Load all entries from a bucket file into memory
    std::vector<Entry> load_bucket(int bucket_id);

    // Save entries back to a bucket file (overwrites existing file)
    void save_bucket(int bucket_id, const std::vector<Entry>& entries);

    // Append a single entry to a bucket file
    void append_to_bucket(int bucket_id, const Entry& entry);
};

#endif // BUCKET_MANAGER_H
