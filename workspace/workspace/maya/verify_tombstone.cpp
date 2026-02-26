#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

int main() {
    std::ifstream file("data.bin", std::ios::binary);
    if (!file) {
        std::cerr << "Cannot open data.bin" << std::endl;
        return 1;
    }

    int active_count = 0;
    int tombstone_count = 0;

    uint8_t idx_length;
    while (file.read(reinterpret_cast<char*>(&idx_length), 1)) {
        std::string index(idx_length, '\0');
        if (!file.read(&index[0], idx_length)) break;

        int32_t value;
        if (!file.read(reinterpret_cast<char*>(&value), sizeof(int32_t))) break;

        uint8_t flags;
        if (!file.read(reinterpret_cast<char*>(&flags), 1)) break;

        if (flags == 0x01) {
            active_count++;
        } else if (flags == 0x00) {
            tombstone_count++;
        }
    }

    std::cout << "Active entries: " << active_count << std::endl;
    std::cout << "Tombstone entries: " << tombstone_count << std::endl;
    std::cout << "Total entries: " << (active_count + tombstone_count) << std::endl;

    return 0;
}
