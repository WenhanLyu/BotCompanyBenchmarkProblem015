#include <iostream>
#include <fstream>

int main() {
    std::ofstream out("rachel_large_test.txt");

    // Insert 1000 entries to test file count limit
    out << "1000\n";
    for (int i = 0; i < 1000; i++) {
        out << "insert key_" << i << " " << i << "\n";
    }
    out.close();

    std::cout << "Generated test with 1000 insertions\n";
    return 0;
}
