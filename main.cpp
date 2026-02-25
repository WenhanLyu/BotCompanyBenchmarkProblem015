#include <iostream>
#include <string>
#include "bucket_manager.h"

using namespace std;

int main() {
    // Initialize bucket manager
    BucketManager manager;

    // Read number of commands
    int n;
    cin >> n;

    // Process each command
    for (int i = 0; i < n; ++i) {
        string command;
        cin >> command;

        if (command == "insert") {
            string index;
            int value;
            cin >> index >> value;
            manager.insert_entry(index, value);
        }
        else if (command == "delete") {
            string index;
            int value;
            cin >> index >> value;
            // Delete operation deferred to M2
            // For now, just read and ignore the delete command
        }
        else if (command == "find") {
            string index;
            cin >> index;

            vector<int> values = manager.find_values(index);

            if (values.empty()) {
                cout << "null" << endl;
            } else {
                for (size_t j = 0; j < values.size(); ++j) {
                    if (j > 0) cout << " ";
                    cout << values[j];
                }
                cout << endl;
            }
        }
    }

    return 0;
}
