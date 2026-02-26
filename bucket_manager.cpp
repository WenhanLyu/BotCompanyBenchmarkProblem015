#include "bucket_manager.h"
#include <algorithm>
#include <fstream>
#include <cstring>

BucketManager::BucketManager() {
    // Initialize all buckets as not loaded
    bucket_loaded_.fill(false);
}

uint32_t BucketManager::compute_hash(const std::string& index) const {
    // Polynomial rolling hash with prime 31 (portable and deterministic)
    uint32_t hash = 0;
    for (unsigned char c : index) {
        hash = hash * 31 + c;
    }
    return hash;
}

int BucketManager::get_bucket_number(const std::string& index) const {
    return compute_hash(index) % NUM_BUCKETS;
}

std::string BucketManager::get_bucket_filename(int bucket_num) const {
    return "data_" + std::to_string(bucket_num) + ".bin";
}

void BucketManager::load_bucket(int bucket_num) {
    if (bucket_loaded_[bucket_num]) {
        return;  // Already loaded
    }

    std::string filename = get_bucket_filename(bucket_num);
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // File doesn't exist yet, mark as loaded (empty bucket)
        bucket_loaded_[bucket_num] = true;
        return;
    }

    // Configure larger buffer for better I/O performance
    char buffer[65536];
    file.rdbuf()->pubsetbuf(buffer, sizeof(buffer));

    int64_t offset = 0;
    uint8_t idx_length;

    while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        int64_t entry_start_offset = offset;

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

        // Add to cache if active
        if (active) {
            uint32_t key_hash = compute_hash(index);
            bucket_cache_[bucket_num][key_hash].emplace_back(key_hash, value, entry_start_offset);
        }

        // Update offset for next entry
        offset += 1 + idx_length + sizeof(int32_t) + 1;
    }

    file.close();
    bucket_loaded_[bucket_num] = true;
}

bool BucketManager::verify_entry_at_offset(int bucket_num, int64_t offset, const std::string& index) const {
    std::string filename = get_bucket_filename(bucket_num);
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        return false;
    }

    file.seekg(offset);

    uint8_t idx_length;
    if (!file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        file.close();
        return false;
    }

    std::string stored_index(idx_length, '\0');
    if (!file.read(&stored_index[0], idx_length)) {
        file.close();
        return false;
    }

    file.close();
    return stored_index == index;
}

void BucketManager::insert_entry(const std::string& index, int value) {
    int bucket_num = get_bucket_number(index);

    // Load bucket if not already loaded
    load_bucket(bucket_num);

    uint32_t key_hash = compute_hash(index);

    // Check if entry already exists in cache
    auto& cache_map = bucket_cache_[bucket_num];
    auto it = cache_map.find(key_hash);

    if (it != cache_map.end()) {
        // Hash collision possible - verify each entry
        for (const auto& entry : it->second) {
            if (entry.value == value && verify_entry_at_offset(bucket_num, entry.file_offset, index)) {
                // Duplicate found
                return;
            }
        }
    }

    // Get current file size to record offset
    std::string filename = get_bucket_filename(bucket_num);
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
    uint8_t flags = 0x01;  // Active entry
    int32_t val = value;

    file.write(reinterpret_cast<const char*>(&idx_length), 1);
    file.write(index.c_str(), idx_length);
    file.write(reinterpret_cast<const char*>(&val), sizeof(int32_t));
    file.write(reinterpret_cast<const char*>(&flags), 1);

    file.close();

    // Add to cache
    cache_map[key_hash].emplace_back(key_hash, value, offset);
}

std::vector<int> BucketManager::find_values(const std::string& index) {
    int bucket_num = get_bucket_number(index);

    // Load bucket if not already loaded
    load_bucket(bucket_num);

    uint32_t key_hash = compute_hash(index);
    std::vector<int> values;

    // Look up in cache
    auto& cache_map = bucket_cache_[bucket_num];
    auto it = cache_map.find(key_hash);

    if (it != cache_map.end()) {
        // Hash collision possible - verify each entry
        for (const auto& entry : it->second) {
            if (verify_entry_at_offset(bucket_num, entry.file_offset, index)) {
                values.push_back(entry.value);
            }
        }
    }

    // Sort values in ascending order
    std::sort(values.begin(), values.end());

    return values;
}

void BucketManager::delete_entry(const std::string& index, int value) {
    int bucket_num = get_bucket_number(index);

    // Load bucket if not already loaded
    load_bucket(bucket_num);

    uint32_t key_hash = compute_hash(index);

    // Find entry in cache
    auto& cache_map = bucket_cache_[bucket_num];
    auto it = cache_map.find(key_hash);

    if (it == cache_map.end()) {
        // Not in cache, doesn't exist
        return;
    }

    // Find the specific entry with matching value and index
    auto& entries = it->second;
    int64_t target_offset = -1;
    size_t entry_idx = 0;

    for (size_t i = 0; i < entries.size(); ++i) {
        if (entries[i].value == value && verify_entry_at_offset(bucket_num, entries[i].file_offset, index)) {
            target_offset = entries[i].file_offset;
            entry_idx = i;
            break;
        }
    }

    if (target_offset == -1) {
        // Entry not found
        return;
    }

    // Mark as deleted in file (in-place tombstone)
    std::string filename = get_bucket_filename(bucket_num);
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) {
        return;
    }

    // Calculate position of flags byte
    uint8_t idx_length = static_cast<uint8_t>(index.length());
    int64_t flags_offset = target_offset + 1 + idx_length + sizeof(int32_t);

    // Seek to flags position and write tombstone
    file.seekp(flags_offset);
    uint8_t tombstone = 0x00;
    file.write(reinterpret_cast<const char*>(&tombstone), 1);
    file.flush();
    file.close();

    // Remove from cache
    entries.erase(entries.begin() + entry_idx);

    // If no more entries for this hash, remove the hash key
    if (entries.empty()) {
        cache_map.erase(it);
    }
}
