#pragma once

#include <vector>

#include "parlay/sequence.h"

void sequence_sort(std::vector<int>& v);

void parallel_sort(std::vector<int>& v);

class ParallelSorter {
private:
    parlay::sequence<int> flags;
    parlay::sequence<int> tmp_seq;
    parlay::sequence<int> seq;

public:
    explicit ParallelSorter(size_t max_size) : flags(max_size), tmp_seq(max_size), seq(max_size) {}
    explicit ParallelSorter(const std::vector<int>& v) : flags(v.size()), tmp_seq(v.size()), seq(v.begin(), v.end()) {}

    void parallel_sort(std::vector<int>& v);
};