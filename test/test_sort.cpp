#include <gtest/gtest.h>
#include "sort.h"
#include <vector>
#include <random>
#include <algorithm>

TEST(SortTest, EmptyVector) {
    std::vector<int> vec = {};
    sequence_sort(vec);
    std::vector<int> expected = {};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, SingleElement) {
    std::vector<int> vec = {42};
    sequence_sort(vec);
    std::vector<int> expected = {42};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, AlreadySorted) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    sequence_sort(vec);
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, ReverseSorted) {
    std::vector<int> vec = {5, 4, 3, 2, 1};
    sequence_sort(vec);
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, RandomOrder) {
    std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};
    sequence_sort(vec);
    std::vector<int> expected = {1, 1, 2, 3, 4, 5, 6, 9};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, BigReverseSorted) {
    const int ELEMENTS = 1000, MAX_VALUE = 1e9;
    std::vector<int> vec(ELEMENTS), expected(ELEMENTS);

    for (int i = 0; i < ELEMENTS; ++i) {
        vec[i] = expected[i] = ELEMENTS - i;
    }

    std::sort(expected.begin(), expected.end());
    sequence_sort(vec);
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, BigRandomOrder) {
    const int ELEMENTS = 1000, MAX_VALUE = 1e9;
    std::vector<int> vec(ELEMENTS), expected(ELEMENTS);

    std::mt19937 gen(13);
    std::uniform_int_distribution<> dis(-MAX_VALUE, MAX_VALUE);
    for (int i = 0; i < ELEMENTS; ++i) {
        vec[i] = expected[i] = dis(gen);
    }

    std::sort(expected.begin(), expected.end());
    sequence_sort(vec);
    EXPECT_EQ(vec, expected);
}






TEST(SortTest, ParallelEmptyVector) {
    std::vector<int> vec = {};
    parallel_sort(vec);
    std::vector<int> expected = {};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, ParallelSingleElement) {
    std::vector<int> vec = {42};
    parallel_sort(vec);
    std::vector<int> expected = {42};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, ParallelAlreadySorted) {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    parallel_sort(vec);
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, ParallelReverseSorted) {
    std::vector<int> vec = {5, 4, 3, 2, 1};
    parallel_sort(vec);
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, ParallelRandomOrder) {
    std::vector<int> vec = {3, 1, 4, 1, 5, 9, 2, 6};
    parallel_sort(vec);
    std::vector<int> expected = {1, 1, 2, 3, 4, 5, 6, 9};
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, ParallelBigReverseSorted) {
    const int ELEMENTS = 1000, MAX_VALUE = 1e9;
    std::vector<int> vec(ELEMENTS), expected(ELEMENTS);

    for (int i = 0; i < ELEMENTS; ++i) {
        vec[i] = expected[i] = ELEMENTS - i;
    }

    std::sort(expected.begin(), expected.end());
    parallel_sort(vec);
    EXPECT_EQ(vec, expected);
}

TEST(SortTest, ParallelBigRandomOrder) {
    const int ELEMENTS = 1000, MAX_VALUE = 10;
    std::vector<int> vec(ELEMENTS), expected(ELEMENTS);

    std::mt19937 gen(13);
    std::uniform_int_distribution<> dis(-MAX_VALUE, MAX_VALUE);
    for (int i = 0; i < ELEMENTS; ++i) {
        vec[i] = expected[i] = dis(gen);
    }

    std::sort(expected.begin(), expected.end());
    parallel_sort(vec);
    EXPECT_EQ(vec, expected);
}