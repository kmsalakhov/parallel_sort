#include <benchmark/benchmark.h>
#include <algorithm>
#include <random>
#include <vector>

#include "sort.h"
#include "parlay/primitives.h"
#include "parlay/parallel.h"

std::vector<int> generate_random_data(size_t size) {
    std::vector<int> data(size);
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 1000000);
    
    parlay::parallel_for(0, size, [&](size_t i) {
        data[i] = dist(rng);
    });
    
    return data;
}

void BM_SequentialSort(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    
    for (auto _ : state) {
        state.PauseTiming();
        auto test_data = data;
        state.ResumeTiming();
        
        sequence_sort(test_data);
    }
    
    state.SetBytesProcessed(state.iterations() * size * sizeof(int));
}

void BM_ParallelSort(benchmark::State& state) {
    const size_t size = state.range(0);
    auto data = generate_random_data(size);
    
    const int num_threads = parlay::num_workers();
    
    for (auto _ : state) {
        state.PauseTiming();
        auto test_data = data;
        state.ResumeTiming();
        
        parallel_sort(test_data);
    }
    
    state.SetBytesProcessed(state.iterations() * size * sizeof(int));
    state.SetLabel(std::string("Threads: ") + std::to_string(num_threads));
}

BENCHMARK(BM_SequentialSort)->Arg(100000000); // 100 млн элементов
BENCHMARK(BM_ParallelSort)->Arg(100000000);   // 4 потока

BENCHMARK_MAIN();