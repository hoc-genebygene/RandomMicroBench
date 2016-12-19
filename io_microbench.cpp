#include <sys/mman.h>

#include <chrono>
#include <fstream>
#include <iostream>

#include "boost/iostreams/device/mapped_file.hpp"


void bench_mmap_stream(const std::string & filename) {
    boost::iostreams::mapped_file_source file;
    try {
        file.open(filename);
    } catch (std::ios::failure & e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    if(0 != madvise((void *) file.data(), file.size(), MADV_SEQUENTIAL)) {
        std::cerr << "Failed to madvise" << std::endl;
        return;
    }

    size_t file_size = file.size();
    const char * data = file.data();

    char * buffer = new char[file_size];

    auto start = std::chrono::steady_clock::now();
    std::memmove(buffer, data, file_size);
    auto stop = std::chrono::steady_clock::now();

    volatile int result = buffer[file_size - 1]; // make sure file actually read

    auto time_taken = stop - start;
    std::cout << "Reading file via mmap took: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken).count() << " milliseconds" << std::endl;
    std::cout << "Bandwidth: " <<  ((double)file_size / 1048576) / (std::chrono::duration_cast<std::chrono::nanoseconds>(time_taken).count() / (double)1'000'000'000) << " MB/s\n" << std::endl;

    file.close();

    delete[] buffer;
}

void bench_ifstream(const std::string & filename) {
    std::ifstream file;
    file.open(filename, std::ios::binary | std::ios::ate);
    size_t file_size = file.tellg();
    file.seekg(0);

    char * buffer = new char[file_size];

    auto start = std::chrono::steady_clock::now();
    file.read(&buffer[0], file_size);
    auto stop = std::chrono::steady_clock::now();

    volatile int result = buffer[file_size - 1]; // make sure file actually read

    auto time_taken = stop - start;
    std::cout << "Reading file via ifstream took: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken).count() << " milliseconds" << std::endl;
    std::cout << "Bandwidth: " <<  ((double)file_size / 1048576) / (std::chrono::duration_cast<std::chrono::nanoseconds>(time_taken).count() / (double)1'000'000'000) << " MB/s\n" << std::endl;

    file.close();

    delete[] buffer;
}

void bench_memcpy(const std::string & filename) {
    boost::iostreams::mapped_file_source file;
    try {
        file.open(filename);
    } catch (std::ios::failure & e) {
        std::cerr << e.what() << std::endl;
        return;
    }

    if(0 != madvise((void *) file.data(), file.size(), MADV_SEQUENTIAL)) {
        std::cerr << "Failed to madvise" << std::endl;
        return;
    }

    size_t file_size = file.size();
    const char * data = file.data();

    char * buffer1 = new char[file_size];
    std::memcpy(buffer1, data, file_size);

    char * buffer2 = new char[file_size];
    auto start = std::chrono::steady_clock::now();
    std::memcpy(buffer2, buffer1, file_size);
    auto stop = std::chrono::steady_clock::now();

    for (size_t k = 0; k < file_size; ++k)
        buffer2[k] *= buffer2[k];


    auto time_taken = stop - start;
    std::cout << "Memcpy took: " << std::chrono::duration_cast<std::chrono::milliseconds>(time_taken).count() << " milliseconds" << std::endl;
    std::cout << "Bandwidth: " <<  ((double)file_size / 1048576) / (std::chrono::duration_cast<std::chrono::nanoseconds>(time_taken).count() / (double)1'000'000'000) << " MB/s\n" << std::endl;

    file.close();

    std::cout << "File size: " << file_size << std::endl;

    delete[] buffer1;
    delete[] buffer2;
}

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return -1;
    }

    std::string filename(argv[1]);

    bench_mmap_stream(filename);
    bench_ifstream(filename);
    bench_memcpy(filename);
    //bench_memcpy();

    return 0;
}
