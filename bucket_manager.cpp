#include "bucket_manager.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <cstring>
#include <cstdint>

BucketManager::BucketManager() {
    // Bucket files are created on-demand when first written
    // No initialization needed here
}

int BucketManager::hash_bucket(const std::string& index) const {
    // Polynomial rolling hash with prime 31 - deterministic and fast
    // Simple multiplication version for optimal performance
    uint32_t hash = 0;

    for (char c : index) {
        hash = hash * 31u + static_cast<uint32_t>(static_cast<unsigned char>(c));
    }

    return static_cast<int>(hash % NUM_BUCKETS);
}

std::string BucketManager::get_bucket_filename(int bucket_id) const {
    char filename[32];
    snprintf(filename, sizeof(filename), "data_%04d.bin", bucket_id);
    return std::string(filename);
}

void BucketManager::append_to_bucket(int bucket_id, const Entry& entry) {
    std::string filename = get_bucket_filename(bucket_id);

    // Open in binary append mode, create if doesn't exist
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file) {
        // If file doesn't exist, it will be created automatically
        // by ofstream with ios::app
        return;
    }

    // Binary format: [1 byte length][N bytes index][4 bytes value][1 byte flags]
    uint8_t idx_length = static_cast<uint8_t>(entry.index.length());
    uint8_t flags = entry.active ? 0x01 : 0x00;
    int32_t value = entry.value;

    // Write entry
    file.write(reinterpret_cast<const char*>(&idx_length), 1);
    file.write(entry.index.c_str(), idx_length);
    file.write(reinterpret_cast<const char*>(&value), sizeof(int32_t));
    file.write(reinterpret_cast<const char*>(&flags), 1);

    file.close();
}

void BucketManager::load_bucket_cache(int bucket_id) {
    // Check if cache already loaded for this bucket
    if (bucket_cache_.find(bucket_id) != bucket_cache_.end()) {
        return;
    }

    // Initialize empty cache for this bucket
    bucket_cache_[bucket_id] = std::unordered_set<std::pair<std::string, int>, PairHash>();

    std::string filename = get_bucket_filename(bucket_id);
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // File doesn't exist yet, cache is empty
        return;
    }

    // Configure larger buffer for better I/O performance
    char buffer[65536];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));

    // Read all active entries and add to cache
    uint8_t idx_length;
    while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        // Read index string
        std::string index(idx_length, '\0');
        if (!file.read(&index[0], idx_length)) {
            break;
        }

        // Read value
        int32_t value;
        if (!file.read(reinterpret_cast<char*>(&value), sizeof(int32_t))) {
            break;
        }

        // Read flags
        uint8_t flags;
        if (!file.read(reinterpret_cast<char*>(&flags), 1)) {
            break;
        }

        bool active = (flags == 0x01);

        // Only add active entries to cache
        if (active) {
            bucket_cache_[bucket_id].insert({index, value});
        }
    }

    file.close();
}

std::vector<Entry> BucketManager::load_bucket(int bucket_id) {
    std::vector<Entry> entries;
    std::string filename = get_bucket_filename(bucket_id);

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // File doesn't exist yet, return empty vector
        return entries;
    }

    // Configure larger buffer for better I/O performance
    char buffer[65536];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));

    // Read all entries from the bucket file
    uint8_t idx_length;
    while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        // Read index string
        std::string index(idx_length, '\0');
        if (!file.read(&index[0], idx_length)) {
            break;
        }

        // Read value
        int32_t value;
        if (!file.read(reinterpret_cast<char*>(&value), sizeof(int32_t))) {
            break;
        }

        // Read flags
        uint8_t flags;
        if (!file.read(reinterpret_cast<char*>(&flags), 1)) {
            break;
        }

        bool active = (flags == 0x01);
        entries.emplace_back(index, value, active);
    }

    file.close();
    return entries;
}

void BucketManager::insert_entry(const std::string& index, int value) {
    int bucket_id = hash_bucket(index);

    // Load bucket cache if not already loaded (lazy loading)
    load_bucket_cache(bucket_id);

    // O(1) duplicate check using cache
    std::pair<std::string, int> key = {index, value};
    if (bucket_cache_[bucket_id].find(key) != bucket_cache_[bucket_id].end()) {
        // Duplicate found, do not insert
        return;
    }

    // No duplicate, append to file
    std::string filename = get_bucket_filename(bucket_id);
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file) {
        return;
    }

    uint8_t idx_length = static_cast<uint8_t>(index.length());
    uint8_t flags = 0x01;
    int32_t val = value;

    file.write(reinterpret_cast<const char*>(&idx_length), 1);
    file.write(index.c_str(), idx_length);
    file.write(reinterpret_cast<const char*>(&val), sizeof(int32_t));
    file.write(reinterpret_cast<const char*>(&flags), 1);

    file.close();

    // Update cache with the new entry
    bucket_cache_[bucket_id].insert(key);
}

std::vector<int> BucketManager::find_values(const std::string& index) {
    int bucket_id = hash_bucket(index);
    std::string filename = get_bucket_filename(bucket_id);
    std::vector<int> values;

    // Stream through the file to find matching entries without loading entire bucket
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // File doesn't exist yet, return empty vector
        return values;
    }

    // Configure larger buffer for better I/O performance
    char buffer[65536];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));

    uint8_t idx_length;
    while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        // Read index string
        std::string entry_index(idx_length, '\0');
        if (!file.read(&entry_index[0], idx_length)) {
            break;
        }

        // Read value
        int32_t entry_value;
        if (!file.read(reinterpret_cast<char*>(&entry_value), sizeof(int32_t))) {
            break;
        }

        // Read flags
        uint8_t flags;
        if (!file.read(reinterpret_cast<char*>(&flags), 1)) {
            break;
        }

        bool active = (flags == 0x01);

        // Collect matching values
        if (active && entry_index == index) {
            values.push_back(entry_value);
        }
    }

    file.close();

    // Sort values in ascending order
    std::sort(values.begin(), values.end());

    return values;
}

void BucketManager::save_bucket(int bucket_id, const std::vector<Entry>& entries) {
    std::string filename = get_bucket_filename(bucket_id);

    // Open in binary write mode (truncate existing file)
    std::ofstream file(filename, std::ios::binary | std::ios::trunc);
    if (!file) {
        return;
    }

    // Write all entries
    for (const auto& entry : entries) {
        uint8_t idx_length = static_cast<uint8_t>(entry.index.length());
        uint8_t flags = entry.active ? 0x01 : 0x00;
        int32_t value = entry.value;

        file.write(reinterpret_cast<const char*>(&idx_length), 1);
        file.write(entry.index.c_str(), idx_length);
        file.write(reinterpret_cast<const char*>(&value), sizeof(int32_t));
        file.write(reinterpret_cast<const char*>(&flags), 1);
    }

    file.close();
}

void BucketManager::delete_entry(const std::string& index, int value) {
    int bucket_id = hash_bucket(index);
    std::string filename = get_bucket_filename(bucket_id);

    // Read entire bucket into memory
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        // File doesn't exist, nothing to delete
        return;
    }

    // Structure to hold an entry
    struct Entry {
        std::string index;
        int32_t value;
        uint8_t flags;
    };

    std::vector<Entry> entries;
    bool found = false;

    // Read all entries from the file
    uint8_t idx_length;
    while (input.read(reinterpret_cast<char*>(&idx_length), 1)) {
        std::string entry_index(idx_length, '\0');
        if (!input.read(&entry_index[0], idx_length)) {
            break;
        }

        int32_t entry_value;
        if (!input.read(reinterpret_cast<char*>(&entry_value), sizeof(int32_t))) {
            break;
        }

        uint8_t flags;
        if (!input.read(reinterpret_cast<char*>(&flags), 1)) {
            break;
        }

        bool active = (flags == 0x01);

        // Check if this is the entry to delete
        if (!found && active && entry_index == index && entry_value == value) {
            // Skip this entry (don't add it to the vector)
            found = true;
            continue;
        }

        // Add entry to vector
        entries.push_back({entry_index, entry_value, flags});
    }

    input.close();

    // If an entry was deleted, rewrite the original file in-place and update cache
    if (found) {
        std::ofstream output(filename, std::ios::binary | std::ios::trunc);
        if (!output) {
            return;
        }

        // Write all remaining entries back to the file
        for (const auto& entry : entries) {
            uint8_t idx_len = static_cast<uint8_t>(entry.index.length());
            output.write(reinterpret_cast<const char*>(&idx_len), 1);
            output.write(entry.index.c_str(), idx_len);
            output.write(reinterpret_cast<const char*>(&entry.value), sizeof(int32_t));
            output.write(reinterpret_cast<const char*>(&entry.flags), 1);
        }

        output.close();

        // Update cache: remove the deleted entry
        // Only update cache if it's already loaded for this bucket
        if (bucket_cache_.find(bucket_id) != bucket_cache_.end()) {
            bucket_cache_[bucket_id].erase({index, value});
        }
    }
}
