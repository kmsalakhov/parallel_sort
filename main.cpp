#include <iostream>
#include <vector>

#include "sort.h"

int main() {
    std::vector<int> vec{7, 5, 3, 2, 5, 5, 1, 6};
    sequence_sort(vec);
    for (int i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << ' ';
    }
    return 0;
}