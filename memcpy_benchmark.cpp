#include <chrono>
#include <cstring>
#include <iostream>

namespace {
    const size_t GB = 1073741824ull;
    const size_t arr_size = GB * 12ull;
}

void in_array_copy(char * arr) {

    auto start = std::chrono::steady_clock::now();
    //std::memcpy(&arr[arr_size/2], &arr[0], arr_size/2);
    auto stop = std::chrono::steady_clock::now();

    auto time_taken = stop - start;

    std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken).count() << " milliseconds" << std::endl;
    std::cout << "Bandwidth: " << (arr_size/2) / (std::chrono::duration_cast<std::chrono::nanoseconds>(time_taken).count() / (double)1'000'000'000) / (1'048'576) << " MB/s" << std::endl;
}

int main() {
    char * arr1 = new char[arr_size];

    in_array_copy(arr1);

    delete[] arr1;
}
