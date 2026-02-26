#include "bucket_manager.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <cstring>
#include <cstdint>

BucketManager::BucketManager() {
    // Data file is created on-demand when first written
    // Index and LRU structures are initially empty
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

    // Not in index - check disk if index might be incomplete
    if (index_.size() >= MAX_INDEX_ENTRIES) {
        // Index is full, need to check disk for entries not in cache
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

    // Read entire file into memory
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        // File doesn't exist, nothing to delete
        return;
    }

    // Structure to hold an entry
    struct EntryData {
        std::string index;
        int32_t value;
        uint8_t flags;
    };

    std::vector<EntryData> entries;
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

    // If an entry was deleted, rewrite the file and update index
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

        // Update index: remove the deleted entry
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
