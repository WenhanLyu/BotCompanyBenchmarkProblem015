#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>

int hash_bucket(const std::string& index) {
    const int NUM_BUCKETS = 20;
    uint32_t hash = 0;
    for (char c : index) {
        hash = hash * 31u + static_cast<uint32_t>(static_cast<unsigned char>(c));
    }
    return static_cast<int>(hash % NUM_BUCKETS);
}

int main() {
    std::ifstream input("stress_100k.txt");
    if (!input) {
        std::cerr << "Cannot open stress_100k.txt" << std::endl;
        return 1;
    }

    int n;
    input >> n;

    std::vector<int> bucket_counts(20, 0);
    std::vector<int> insert_counts(20, 0);
    int total_inserts = 0;

    for (int i = 0; i < n; i++) {
        std::string cmd, index;
        int value;
        input >> cmd >> index;

        if (cmd == "insert") {
            input >> value;
            int bucket = hash_bucket(index);
            bucket_counts[bucket]++;
            insert_counts[bucket]++;
            total_inserts++;
        } else if (cmd == "delete") {
            input >> value;
            int bucket = hash_bucket(index);
            bucket_counts[bucket]--;
        } else if (cmd == "find") {
            int bucket = hash_bucket(index);
        }
    }

    std::cout << "Bucket Distribution Analysis for stress_100k.txt" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "Total insert operations: " << total_inserts << std::endl << std::endl;

    int max_bucket = 0;
    int min_bucket = insert_counts[0];
    long long total_comparisons = 0;

    for (int i = 0; i < 20; i++) {
        std::cout << "Bucket " << i << ": " << insert_counts[i] << " inserts, "
                  << "final count: " << bucket_counts[i] << std::endl;
        if (insert_counts[i] > max_bucket) max_bucket = insert_counts[i];
        if (insert_counts[i] < min_bucket) min_bucket = insert_counts[i];

        // O(n^2) cost: for n inserts, we do 1+2+3+...+n = n*(n+1)/2 comparisons
        long long bucket_cost = (long long)insert_counts[i] * (insert_counts[i] + 1) / 2;
        total_comparisons += bucket_cost;
    }

    std::cout << std::endl;
    std::cout << "Statistics:" << std::endl;
    std::cout << "Max bucket inserts: " << max_bucket << std::endl;
    std::cout << "Min bucket inserts: " << min_bucket << std::endl;
    std::cout << "Avg bucket inserts: " << total_inserts / 20.0 << std::endl;
    std::cout << "Imbalance ratio: " << (double)max_bucket / min_bucket << std::endl;
    std::cout << std::endl;
    std::cout << "Total sequential comparisons (O(n^2)): " << total_comparisons << std::endl;

    return 0;
}
