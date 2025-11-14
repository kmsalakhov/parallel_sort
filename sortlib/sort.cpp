#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

#include "sort.h"
#include "parlay/primitives.h"
#include "parlay/parallel.h"
#include "parlay/sequence.h"
#include "parlay/utilities.h"

int partition(std::vector<int>& v, int l, int r) {
    const int m = l + r >> 1;
    const int value = v[m];

    int left = l, right = r;
    while (left <= right) {
        while (v[left] < value) {
            ++left;
        }
        while (v[right] > value) {
            --right;
        }
        if (right <= left) {
            break;
        }
        std::swap(v[left++], v[right--]);
    }

    return right;
}

void quick_sort(std::vector<int>& v, int l, int r) {
    if (l < r) {
        const int q = partition(v, l, r);
        quick_sort(v, l, q);
        quick_sort(v, q + 1, r);
    }
}

void sequence_sort(std::vector<int>& v) {
    if (!v.empty()) {
        quick_sort(v, 0, v.size() - 1);
    }
}

void parallel_quick_sort_hybrid(parlay::sequence<int>& seq, size_t l, size_t r, int depth = 0) {
    if (l >= r || l >= seq.size() || r >= seq.size() || l > r) {
        return;
    }
    
    const size_t global_threshold = 10000;
    
    const int max_depth = 2 * static_cast<int>(std::log2(parlay::num_workers() + 1));
    
    if (r - l + 1 < global_threshold || depth >= max_depth) {
        std::sort(seq.begin() + l, seq.begin() + r + 1);
        return;
    }
    
    size_t mid = l + (r - l) / 2;
    if (seq[l] > seq[mid]) std::swap(seq[l], seq[mid]);
    if (seq[mid] > seq[r]) std::swap(seq[mid], seq[r]);
    if (seq[l] > seq[mid]) std::swap(seq[l], seq[mid]);
    const int pivot = seq[mid];
    
    size_t i = l, j = r;
    while (i <= j) {
        while (i <= j && seq[i] < pivot) i++;
        while (i <= j && seq[j] > pivot) j--;
        if (i <= j) {
            std::swap(seq[i++], seq[j--]);
        }
    }

    const size_t left_size = i - l;
    const size_t right_size = r - (i - 1);
    

    if (left_size < (r - l + 1) / 10 || right_size < (r - l + 1) / 10) {
        std::sort(seq.begin() + l, seq.begin() + r + 1);
        return;
    }
    
    if (left_size > right_size) {
        parlay::par_do(
            [&]() { parallel_quick_sort_hybrid(seq, i, r, depth + 1); },
            []() {}
        );
        parallel_quick_sort_hybrid(seq, l, i - 1, depth + 1);
    } else {
        parlay::par_do(
            [&]() { parallel_quick_sort_hybrid(seq, l, i - 1, depth + 1); },
            []() {}
        );
        parallel_quick_sort_hybrid(seq, i, r, depth + 1);
    }
}

void parallel_sort(std::vector<int>& v) {
    if (v.empty()) return;
    
    std::cout << "Workers: " << parlay::num_workers() 
              << ", Data size: " << v.size() << std::endl;
    
    parlay::sequence<int> seq(v.begin(), v.end());
    
    parallel_quick_sort_hybrid(seq, 0, seq.size() - 1);
    
    if (v.size() == seq.size()) {
        parlay::parallel_for(0, v.size(), [&v, &seq](size_t i) {
            v[i] = seq[i];
        });
    } else {
        std::copy(seq.begin(), seq.end(), v.begin());
    }
}





// void parallel_quick_sort(parlay::sequence<int>& seq, int l, int r) {
//     if (l >= r) return;
    
//     const int m = (l + r) >> 1;
//     const int pivot = seq[m];
    
//     auto less = parlay::filter(seq.subseq(l, r+1), [pivot](int x) { return x < pivot; });
//     auto equal = parlay::filter(seq.subseq(l, r+1), [pivot](int x) { return x == pivot; });
//     auto greater = parlay::filter(seq.subseq(l, r+1), [pivot](int x) { return x > pivot; });
    
//     size_t idx = l;
//     parlay::parallel_for(0, less.size(), [&](size_t i) { seq[idx + i] = less[i]; }, 1e5);
//     idx += less.size();
//     parlay::parallel_for(0, equal.size(), [&](size_t i) { seq[idx + i] = equal[i]; }, 1e5);
//     idx += equal.size();
//     parlay::parallel_for(0, greater.size(), [&](size_t i) { seq[idx + i] = greater[i]; }, 1e5);
    
//     parallel_quick_sort(seq, l, l + less.size() - 1);
//     parallel_quick_sort(seq, l + less.size() + equal.size(), r);
// }

// void parallel_quick_sort_safe(parlay::sequence<int>& seq, size_t l, size_t r) {
//     if (l >= r || l >= seq.size() || r >= seq.size() || l > r) {
//         return;
//     }
    
//     const size_t threshold = 1000;
//     if (r - l + 1 < threshold) {
//         std::sort(seq.begin() + l, seq.begin() + r + 1);
//         return;
//     }
    
//     const size_t pivot_idx = l + (r - l) / 2;
//     if (pivot_idx >= seq.size()) return;
    
//     const int pivot = seq[pivot_idx];
    
//     const size_t subarray_size = r - l + 1;
//     if (subarray_size == 0) return;
    
//     auto range = parlay::iota(subarray_size);
    
//     auto left_indices = parlay::filter(range, [&](size_t idx) {
//         size_t pos = l + idx;
//         return pos < seq.size() && seq[pos] <= pivot;
//     });
    
//     auto right_indices = parlay::filter(range, [&](size_t idx) {
//         size_t pos = l + idx;
//         return pos < seq.size() && seq[pos] > pivot;
//     });
    
//     if (left_indices.empty() || right_indices.empty()) {
//         std::sort(seq.begin() + l, seq.begin() + r + 1);
//         return;
//     }
    
//     auto left_part = parlay::map(left_indices, [&](size_t idx) {
//         return seq[l + idx];
//     });
    
//     auto right_part = parlay::map(right_indices, [&](size_t idx) {
//         return seq[l + idx];
//     });
    
//     if (left_part.size() + right_part.size() != subarray_size) {
//         std::sort(seq.begin() + l, seq.begin() + r + 1);
//         return;
//     }
    
//     parlay::parallel_for(0, left_part.size(), [&](size_t i) {
//         if (l + i < seq.size()) seq[l + i] = left_part[i];
//     });
    
//     parlay::parallel_for(0, right_part.size(), [&](size_t i) {
//         size_t pos = l + left_part.size() + i;
//         if (pos < seq.size()) seq[pos] = right_part[i];
//     });
    
//     size_t left_size = left_part.size();
//     size_t split_point = l + left_size - 1;
    
//     if (split_point < l || split_point >= seq.size() || split_point + 1 > r) {
//         // Если разделение некорректное, используем однопоточную сортировку
//         std::sort(seq.begin() + l, seq.begin() + r + 1);
//         return;
//     }
    
//     if (left_size > threshold && (subarray_size - left_size) > threshold) {
//         auto seq_copy1 = seq; // Копия для левой части
//         auto seq_copy2 = seq; // Копия для правой части
        
//         parlay::par_do(
//             [&]() { 
//                 parallel_quick_sort_safe(seq_copy1, l, split_point); 
//                 parlay::parallel_for(l, split_point + 1, [&](size_t i) {
//                     if (i < seq.size() && i < seq_copy1.size()) 
//                         seq[i] = seq_copy1[i];
//                 });
//             },
//             [&]() {  
//                 parallel_quick_sort_safe(seq_copy2, split_point + 1, r);
//                 parlay::parallel_for(split_point + 1, r + 1, [&](size_t i) {
//                     if (i < seq.size() && i < seq_copy2.size()) 
//                         seq[i] = seq_copy2[i];
//                 });
//             }
//         );
//     } else {
//         parallel_quick_sort_safe(seq, l, split_point);
//         parallel_quick_sort_safe(seq, split_point + 1, r);
//     }
// }

// void parallel_sort(std::vector<int>& v) {
//     if (v.empty()) return;
    
//     std::cout << "Workers: " << parlay::num_workers() << std::endl;
    
//     parlay::sequence<int> seq(v.begin(), v.end());
    
//     if (!seq.empty()) {
//         parallel_quick_sort_safe(seq, 0, seq.size() - 1);
//     }
    
//     const size_t n = std::min(v.size(), seq.size());
//     parlay::parallel_for(0, n, [&v, &seq](size_t i){
//         v[i] = seq[i];
//     });
// }
