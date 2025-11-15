#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include <assert.h>

#include "sort.h"

#include "parlay/primitives.h"
#include "parlay/parallel.h"
#include "parlay/sequence.h"
#include "parlay/utilities.h"

#ifdef DEBUG
    #define debug(var) \
        do { \
            std::cout << #var << " = " << (var) << std::endl; \
        } while(0)

    template<typename T>
    void print_container(const T& container, const std::string& name) {
        std::cout << name << " = [";
        bool first = true;
        for (const auto& elem : container) {
            if (!first) std::cout << ", ";
            std::cout << elem;
            first = false;
        }
        std::cout << "]" << std::endl;
    }

    #define cdebug(var) \
        print_container(var, #var)

    #define print(var) \
        do { \
            std::cout << (var) << std::endl; \
        } while(0)

#else

    #define debug(var) do {} while(0)
    #define cdebug(var) do {} while(0)
    #define print(var) do {} while(0)

#endif

const size_t MAX_VECTOR_SIZE = 1e8;
const size_t THREADS_COUNT = 4;

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

void copy_sequence(const parlay::sequence<int>& source, std::vector<int>& dest) {
  // This *might* be more efficient than a parallel-for copying each element individually
//   parlay::blocked_for(0, source.size(), 512, [&](size_t i, size_t start, size_t end) {
//     std::copy(source.begin() + start, source.begin() + end, dest.begin() + start);
//   });
    parlay::parallel_for(0, source.size(), [&source, &dest](const size_t i){
        dest[i] = source[i];
    });
}

void copy_sequence(const std::vector<int>& source, parlay::sequence<int>& dest) {
  // This *might* be more efficient than a parallel-for copying each element individually
    parlay::parallel_for(0, source.size(), [&source, &dest](const size_t i){
        dest[i] = source[i];
    });
}

const size_t THRESHOLD_TO_SEQUENCE = 1000000;

void parallel_quick_sort(parlay::sequence<int>& seq, const size_t l, const size_t r, parlay::sequence<int>& flags, parlay::sequence<int>& tmp_seq) {
    if (r - l <= 1) {
        return;
    }

#ifndef DEBUG
    if (r - l < THRESHOLD_TO_SEQUENCE) {
        std::sort(seq.begin() + l, seq.begin() + r);
        return;
    }
#endif

    debug(l);
    debug(r);
    cdebug(seq);
    const int size = r - l;
    const int m = l + r >> 1;
    const int pivot = seq[m];

    parlay::parallel_for(l, r, [&flags, &seq, &tmp_seq, pivot](const size_t i) {
        flags[i] = seq[i] < pivot;
        tmp_seq[i] = seq[i];
    });
    print("filled flags and tmp_seq");
    cdebug(flags);
    cdebug(tmp_seq);

    const int left_size = parlay::scan_inclusive_inplace(flags.cut(l, r));
    print("scanned flags");
    debug(left_size);
    cdebug(flags);

    parlay::parallel_for(l, r, [&flags, &seq, &tmp_seq, l](const size_t i) {
        if ((i == l && flags[i] != 0) || (i != l && flags[i] != flags[i - 1])) {
            seq[l + flags[i] - 1] = tmp_seq[i];
        }
    }, THRESHOLD_TO_SEQUENCE);
    print("filled left part");
    cdebug(seq);

    parlay::parallel_for(l, r, [&flags, &tmp_seq, pivot](const size_t i) {
        flags[i] = tmp_seq[i] >= pivot;
    });
    print("filled flags for right part");
    cdebug(flags);

    const int right_size = parlay::scan_inclusive_inplace(flags.cut(l, r));
    assert(left_size + right_size == size);
    print("scanned flags for right part");
    cdebug(flags);

    parlay::parallel_for(l, r, [&seq, &tmp_seq, &flags, left_size, l](const size_t i) {
        if ((i == l && flags[i] != 0) || (i != l && flags[i] != flags[i - 1])) {
            seq[l + left_size + flags[i] - 1] = tmp_seq[i];
        }
    });
    print("filled left part, now seq should be correct");
    cdebug(seq);

    auto slice = seq.cut(l, r);
    auto pivot_it = parlay::find(slice, pivot);
    assert(pivot_it != slice.end());
    const size_t pivot_id = l + (pivot_it - slice.begin());
    const size_t new_pivot_id = l + left_size;
    std::swap(seq[pivot_id], seq[new_pivot_id]);
    print("getted pivot_id and swapped it");
    debug(pivot_id);
    debug(new_pivot_id);
    debug(l + left_size);
    cdebug(seq);

    parlay::parallel_do(
        [&seq, l, new_pivot_id, &flags, &tmp_seq]() {
            parallel_quick_sort(seq, l, new_pivot_id, flags, tmp_seq);
        },
        [&seq, r, new_pivot_id, &flags, &tmp_seq]() {
            parallel_quick_sort(seq, new_pivot_id + 1, r, flags, tmp_seq);
        }
    );
    print("finished");
}

void parallel_sort(std::vector<int>& v) {
    if (v.empty()) {
        return;
    }

    parlay::sequence<int> flags(v.size());
    parlay::sequence<int> tmp_seq(v.size());
    parlay::sequence<int> seq(v.begin(), v.end());
    parallel_quick_sort(seq, 0, v.size(), flags, tmp_seq);

    copy_sequence(seq, v);
}

void ParallelSorter::parallel_sort(std::vector<int> &v) {
    copy_sequence(v, seq);
    parallel_quick_sort(this->seq, 0, v.size(), this->flags, this->tmp_seq);
    copy_sequence(seq, v);
}
