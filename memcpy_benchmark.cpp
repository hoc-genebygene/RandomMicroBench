#include <chrono>
#include <cstring>
#include <iostream>

namespace {
    const size_t GB = 1073741824ull;
    const size_t arr_size = GB * 8ull;
}

void large_memcpy_copy(char * arr) {
    auto start = std::chrono::steady_clock::now();
    std::memcpy(&arr[arr_size/2], &arr[0], arr_size/2);
    auto stop = std::chrono::steady_clock::now();
    auto time_taken = stop - start;

    uint64_t accum = 0;
    for (uint64_t k = 0; k < arr_size; ++k) {
        accum += arr[k];
    }
    std::cout << accum << std::endl;

    std::cout << "Time taken (large memcpy): " << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken).count() << " milliseconds" << std::endl;
    std::cout << "Bandwidth: " << (arr_size/2) / (std::chrono::duration_cast<std::chrono::nanoseconds>(time_taken).count() / (double)1000000000) / (1048576) << " MB/s" << std::endl;
}

void small_memcpy_copy(char * arr, uint64_t block_size) {
    auto start = std::chrono::steady_clock::now();
    for (uint64_t k = 0; k < arr_size/2; k+=block_size) {
        std::memcpy(&arr[k+arr_size/2], &arr[k], block_size);
    }
    auto stop = std::chrono::steady_clock::now();
    auto time_taken = stop - start;

    uint64_t accum = 0;
    for (uint64_t k = 0; k < arr_size; ++k) {
        accum += arr[k];
    }

    std::cout << block_size << " Bandwidth: " << (arr_size/2) / (std::chrono::duration_cast<std::chrono::nanoseconds>(time_taken).count() / (double)1000000000) / (1048576) << " MB/s " << accum << std::endl;
}

void naive_copy(char * arr) {
    auto start = std::chrono::steady_clock::now();
    for (uint64_t k = 0; k+arr_size/2+3 < arr_size; k+=4) {
        arr[k + arr_size/2] = arr[k];
        arr[k + arr_size/2 + 1] = arr[k+1];
        arr[k + arr_size/2 + 2] = arr[k+2];
        arr[k + arr_size/2 + 3] = arr[k+3];
    }

    auto stop = std::chrono::steady_clock::now();
    auto time_taken = stop - start;

    uint64_t accum = 0;
    for (uint64_t k = 0; k < arr_size; ++k) {
        accum += arr[k];
    }
    std::cout << accum << std::endl;

    std::cout << "Time taken (naive copy): " << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken).count() << " milliseconds" << std::endl;
    std::cout << "Bandwidth: " << (arr_size/2) / (std::chrono::duration_cast<std::chrono::nanoseconds>(time_taken).count() / (double)1000000000) / (1'048'576) << " MB/s" << std::endl;

}

int main() {
    std::cout << "Array size: " << arr_size << std::endl;
    char * arr1 = new char[arr_size];

    std::memset(arr1, 1, arr_size);

    large_memcpy_copy(arr1);
    std::cout << std::endl;
    for (uint64_t k = 1; k <= GB; k *= 2) {
        small_memcpy_copy(arr1, k);
    }
    std::cout << std::endl;
    large_memcpy_copy(arr1);
    //naive_copy(arr1);

    delete[] arr1;
}
