#include "bucket_manager.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <cstring>
#include <cstdint>

BucketManager::BucketManager() {
    // Data file is created on-demand when first written
    // Index and LRU structures are initially empty
    // Bloom filter is initially empty

    // If data file exists, populate bloom filter for existing entries
    std::string filename = get_data_filename();
    std::ifstream file(filename, std::ios::binary);
    if (file) {
        uint8_t idx_length;
        while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
            std::string index(idx_length, '\0');
            if (!file.read(&index[0], idx_length)) {
                break;
            }

            int32_t value;
            if (!file.read(reinterpret_cast<char*>(&value), sizeof(int32_t))) {
                break;
            }

            uint8_t flags;
            if (!file.read(reinterpret_cast<char*>(&flags), 1)) {
                break;
            }

            bool active = (flags == 0x01);
            if (active) {
                // Add to bloom filter
                bloom_add(index, value);
            }
        }
        file.close();
    }
}

std::string BucketManager::get_data_filename() const {
    return "data.bin";
}

void BucketManager::update_lru(const std::pair<std::string, int>& key) {
    // Remove from current position if exists
    auto it = lru_pos_.find(key);
    if (it != lru_pos_.end()) {
        lru_list_.erase(it->second);
    }

    // Add to front (most recent)
    lru_list_.push_front(key);
    lru_pos_[key] = lru_list_.begin();
}

void BucketManager::evict_lru_if_needed() {
    while (index_.size() > MAX_INDEX_ENTRIES && !lru_list_.empty()) {
        // Evict least-recently-used entry (back of list)
        auto victim_key = lru_list_.back();
        lru_list_.pop_back();
        lru_pos_.erase(victim_key);
        index_.erase(victim_key);
    }
}

// Bloom filter hash function 1: FNV-1a variant
size_t BucketManager::bloom_hash1(const std::string& index, int value) const {
    size_t hash = 2166136261u;  // FNV offset basis

    // Hash the string
    for (unsigned char c : index) {
        hash ^= c;
        hash *= 16777619u;  // FNV prime
    }

    // Mix in the value
    hash ^= static_cast<uint32_t>(value);
    hash *= 16777619u;

    return hash % BLOOM_FILTER_SIZE;
}

// Bloom filter hash function 2: Polynomial rolling hash with prime 37
size_t BucketManager::bloom_hash2(const std::string& index, int value) const {
    uint32_t hash = 0;
    uint32_t prime = 37;

    for (unsigned char c : index) {
        hash = hash * prime + c;
    }

    // Mix in the value
    hash = hash * prime + static_cast<uint32_t>(value);

    return hash % BLOOM_FILTER_SIZE;
}

// Bloom filter hash function 3: Combination hash
size_t BucketManager::bloom_hash3(const std::string& index, int value) const {
    uint32_t hash = 5381;  // djb2 initial value

    for (unsigned char c : index) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }

    // Mix in the value with XOR and rotation
    uint32_t val_hash = static_cast<uint32_t>(value);
    hash ^= (val_hash + 0x9e3779b9 + (hash << 6) + (hash >> 2));

    return hash % BLOOM_FILTER_SIZE;
}

// Add entry to bloom filter
void BucketManager::bloom_add(const std::string& index, int value) {
    bloom_filter_.set(bloom_hash1(index, value));
    bloom_filter_.set(bloom_hash2(index, value));
    bloom_filter_.set(bloom_hash3(index, value));
}

// Check if entry might exist in bloom filter
bool BucketManager::bloom_contains(const std::string& index, int value) const {
    return bloom_filter_.test(bloom_hash1(index, value)) &&
           bloom_filter_.test(bloom_hash2(index, value)) &&
           bloom_filter_.test(bloom_hash3(index, value));
}

bool BucketManager::check_file_for_duplicate(const std::string& index, int value) {
    std::string filename = get_data_filename();
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // File doesn't exist yet, no duplicates
        return false;
    }

    // Configure larger buffer for better I/O performance
    char buffer[65536];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));

    // Scan file for duplicate
    uint8_t idx_length;
    while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        std::string entry_index(idx_length, '\0');
        if (!file.read(&entry_index[0], idx_length)) {
            break;
        }

        int32_t entry_value;
        if (!file.read(reinterpret_cast<char*>(&entry_value), sizeof(int32_t))) {
            break;
        }

        uint8_t flags;
        if (!file.read(reinterpret_cast<char*>(&flags), 1)) {
            break;
        }

        bool active = (flags == 0x01);

        // Check for duplicate
        if (active && entry_index == index && entry_value == value) {
            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

std::vector<Entry> BucketManager::load_all_entries() {
    std::vector<Entry> entries;
    std::string filename = get_data_filename();

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // File doesn't exist yet, return empty vector
        return entries;
    }

    // Configure larger buffer for better I/O performance
    char buffer[65536];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));

    // Read all entries from the file
    uint8_t idx_length;
    while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        std::string index(idx_length, '\0');
        if (!file.read(&index[0], idx_length)) {
            break;
        }

        int32_t value;
        if (!file.read(reinterpret_cast<char*>(&value), sizeof(int32_t))) {
            break;
        }

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
    std::pair<std::string, int> key = {index, value};

    // Check in-memory index first (O(1))
    if (index_.find(key) != index_.end()) {
        // Entry is in index, update LRU and return (duplicate)
        update_lru(key);
        return;
    }

    // Check bloom filter (O(1))
    if (!bloom_contains(index, value)) {
        // Bloom filter says "definitely not present" - safe to insert
        // This is the fast path for 99% of inserts after cache fills
    } else {
        // Bloom filter says "might be present" (~1% false positive rate)
        // Need to check file to confirm
        if (check_file_for_duplicate(index, value)) {
            // Duplicate found on disk
            return;
        }
    }

    // Get current file size to record offset
    std::string filename = get_data_filename();
    std::ifstream check_file(filename, std::ios::binary | std::ios::ate);
    int64_t offset = 0;
    if (check_file.is_open()) {
        offset = static_cast<int64_t>(check_file.tellg());
        check_file.close();
    }

    // Append to file
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

    // Add to bloom filter
    bloom_add(index, value);

    // Evict LRU entry if needed before adding new entry
    if (index_.size() >= MAX_INDEX_ENTRIES) {
        evict_lru_if_needed();
    }

    // Add to index and update LRU
    index_[key] = offset;
    update_lru(key);
}

std::vector<int> BucketManager::find_values(const std::string& index) {
    std::string filename = get_data_filename();
    std::vector<int> values;

    // Stream through the file to find matching entries without loading entire file
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

void BucketManager::save_all_entries(const std::vector<Entry>& entries) {
    std::string filename = get_data_filename();

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
    std::string filename = get_data_filename();

    // Read file, collect non-deleted entries
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        // File doesn't exist, nothing to delete
        return;
    }

    // Collect entries to keep (excluding the one to delete)
    std::string buffer;  // Binary buffer for rewriting
    bool found = false;

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
            // Skip this entry (don't add to buffer)
            found = true;
            continue;
        }

        // Add entry to buffer
        buffer.push_back(idx_length);
        buffer.append(entry_index);
        buffer.append(reinterpret_cast<const char*>(&entry_value), sizeof(int32_t));
        buffer.push_back(flags);
    }

    input.close();

    // If entry was found, rewrite file
    if (found) {
        std::ofstream output(filename, std::ios::binary | std::ios::trunc);
        if (!output) {
            return;
        }

        output.write(buffer.data(), buffer.size());
        output.close();

        // Update index and LRU structures
        std::pair<std::string, int> key = {index, value};
        auto idx_it = index_.find(key);
        if (idx_it != index_.end()) {
            index_.erase(idx_it);

            // Also remove from LRU tracking
            auto lru_it = lru_pos_.find(key);
            if (lru_it != lru_pos_.end()) {
                lru_list_.erase(lru_it->second);
                lru_pos_.erase(lru_it);
            }
        }
    }
}
