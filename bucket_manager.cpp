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
    std::string filename = get_bucket_filename(bucket_id);

    // Stream through the file to check for duplicates without loading entire bucket
    std::ifstream file(filename, std::ios::binary);
    if (file) {
        while (file.peek() != EOF) {
            uint8_t idx_length;
            if (!file.read(reinterpret_cast<char*>(&idx_length), 1)) {
                break;
            }

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

            // Check for duplicate
            if (active && entry_index == index && entry_value == value) {
                // Duplicate found, do not insert
                file.close();
                return;
            }
        }
        file.close();
    }

    // No duplicate found, safe to append
    Entry entry(index, value, true);
    append_to_bucket(bucket_id, entry);
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

    while (file.peek() != EOF) {
        uint8_t idx_length;
        if (!file.read(reinterpret_cast<char*>(&idx_length), 1)) {
            break;
        }

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
    std::string temp_filename = filename + ".tmp";

    // Stream through the file without loading entire bucket into memory
    std::ifstream input(filename, std::ios::binary);
    if (!input) {
        // File doesn't exist, nothing to delete
        return;
    }

    std::ofstream output(temp_filename, std::ios::binary);
    if (!output) {
        input.close();
        return;
    }

    bool found = false;

    // Stream through the file and copy all entries except the one to delete
    while (input.peek() != EOF) {
        // Read entry
        uint8_t idx_length;
        if (!input.read(reinterpret_cast<char*>(&idx_length), 1)) {
            break;
        }

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
            // Skip this entry (don't write it to output)
            found = true;
            continue;
        }

        // Write this entry to the temp file
        output.write(reinterpret_cast<const char*>(&idx_length), 1);
        output.write(entry_index.c_str(), idx_length);
        output.write(reinterpret_cast<const char*>(&entry_value), sizeof(int32_t));
        output.write(reinterpret_cast<const char*>(&flags), 1);
    }

    input.close();
    output.close();

    // Replace the original file with the temp file if an entry was deleted
    if (found) {
        std::remove(filename.c_str());
        std::rename(temp_filename.c_str(), filename.c_str());
    } else {
        // No entry was deleted, remove the temp file
        std::remove(temp_filename.c_str());
    }
}
