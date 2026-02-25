#include "bucket_manager.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <cstring>

BucketManager::BucketManager() {
    // Bucket files are created on-demand when first written
    // No initialization needed here
}

int BucketManager::hash_bucket(const std::string& index) const {
    return std::hash<std::string>{}(index) % NUM_BUCKETS;
}

std::string BucketManager::get_bucket_filename(int bucket_id) const {
    char filename[32];
    snprintf(filename, sizeof(filename), "data_%02d.bin", bucket_id);
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

std::vector<Entry> BucketManager::load_bucket(int bucket_id) {
    std::vector<Entry> entries;
    std::string filename = get_bucket_filename(bucket_id);

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        // File doesn't exist yet, return empty vector
        return entries;
    }

    // Read all entries from the bucket file
    while (file.peek() != EOF) {
        uint8_t idx_length;
        if (!file.read(reinterpret_cast<char*>(&idx_length), 1)) {
            break;
        }

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
    Entry entry(index, value, true);
    append_to_bucket(bucket_id, entry);
}

std::vector<int> BucketManager::find_values(const std::string& index) {
    int bucket_id = hash_bucket(index);
    std::vector<Entry> entries = load_bucket(bucket_id);

    // Collect all active values matching the index
    std::vector<int> values;
    for (const auto& entry : entries) {
        if (entry.active && entry.index == index) {
            values.push_back(entry.value);
        }
    }

    // Sort values in ascending order
    std::sort(values.begin(), values.end());

    return values;
}
