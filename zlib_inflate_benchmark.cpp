#include <fstream>
#include <iostream>
#include <exception>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "boost/iostreams/device/mapped_file.hpp"


#include "zlib.h"

namespace {
    const unsigned int MAX_BLOCK_SIZE = 65536;
}

class BamReader {
public:
    BamReader(const std::string & filename, const std::string & mode): mode_(mode) {
        if (mode_ == "use_ifstream") {
            read_file_ifstream(filename);
        } else if (mode_ == "use_boost_mmap") {
            read_file_boost_mmap(filename);
        }
        else {
            read_file_mmap(filename.c_str());
        }
    }

    ~BamReader() {
        if (mode_ == "use_ifstream") {
            delete(buffer_);
        }
        else {
            munmap(buffer_, file_size_);
        }
    }

    void read_file_boost_mmap(const std::string & filename) {
        std::cout << "Using boost::mapped_file_source" << std::endl;

        try {
            file.open(filename);
        } catch (std::ios::failure & e) {
            throw;
        }

        file_size_ = file.size();
        buffer_ = (char *)file.data();

        block_begin_index_ = 0;
    }

    void read_file_mmap(const char * filename) {
        std::cout << "Using POSIX mmap" << std::endl;

        int fd = open(filename, O_RDONLY);
        struct stat sb;
        fstat(fd, &sb);

        file_size_ = sb.st_size;
        std::cout << "File size: " << file_size_ << std::endl;

        buffer_ = (char *)mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        if (buffer_ == MAP_FAILED) {
            throw std::runtime_error("mmap failed!");
        }

        block_begin_index_ = 0;
    }

    void read_file_ifstream(const std::string & filename) {
        std::cout << "Using ifstream" << std::endl;
        std::ifstream read_stream(filename, std::ios::binary | std::ios::ate);

        file_size_ = read_stream.tellg();
        std::cout << "File size: " << file_size_ << std::endl;
        read_stream.seekg(0, std::ios::beg);

        buffer_ = new char[file_size_];

        auto start = std::chrono::steady_clock::now();
        if (!read_stream.read(buffer_, file_size_)) {
            throw std::runtime_error("Error reading file into buffer!");
        }
        read_stream.close();
        auto stop = std::chrono::steady_clock::now();
        double file_size_compressed_MB = file_size_ / (1024 * 1024.0);
        double time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / 1000000000.0;
        std::cout << "ifstream read file at " << file_size_compressed_MB / time << " compressed MB/s" << std::endl;

        block_begin_index_ = 0;
    }

    std::pair<uint64_t, uint32_t> getNextBlock() {
        std::lock_guard<std::mutex> lock(next_block_mutex_);
        if (block_begin_index_ == file_size_)
            return { file_size_, 0 };

        uint16_t length = 0;
        std::memcpy(&length, buffer_+block_begin_index_+16, sizeof(length));
        length+=1;
        auto ret = std::pair<uint64_t, uint32_t>(block_begin_index_, length);
        block_begin_index_ += length;
        return ret;
    }

    char * getBuffer() {
        return buffer_;
    }

    uint64_t getFileSize() {
        return file_size_;
    }

private:
    char * buffer_;

    boost::iostreams::mapped_file_source file;

    std::mutex next_block_mutex_;
    uint64_t block_begin_index_;

    uint64_t file_size_;

    std::string mode_;
};

int main(int argc, char * argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " /path/to/bam/file num_threads [use_ifstream]" << std::endl;
        return -1;
    }

    BamReader reader(argv[1], argc == 3 ? "" : argv[3]);
    const int NUM_THREADS = std::stoi(argv[2]);

    auto deflateProc = [&reader]() {
        z_stream zs;
        zs.zalloc = nullptr;
        zs.zfree = nullptr;
        int status = inflateInit2(&zs, -15);
        if (status != Z_OK) {
            throw std::runtime_error("Zlib initialization failed!");
        }

        while(true) {
            auto block = reader.getNextBlock();

            if (block.second == 0)
                return;

            const uint64_t block_begin_index = block.first;
            const uint32_t length = block.second;
            const char * buffer = reader.getBuffer();

            if (buffer[block_begin_index] != (char)31 || buffer[block_begin_index+1] != (char)139) {
                throw std::runtime_error("BgzfBlock header magic bytes don't match! Corrupt file?");
            }

            uint32_t uncompressed_size = 0;
            std::memcpy(&uncompressed_size, buffer+block_begin_index+length-4, sizeof(uncompressed_size));


            zs.next_in = (Bytef *)buffer+block_begin_index+18;
            zs.avail_in = length - 16;

            std::vector<char> uncompressed_data(MAX_BLOCK_SIZE);

            zs.next_out = (Bytef *)uncompressed_data.data();
            zs.avail_out = uncompressed_data.size();

            auto inflate_status = inflate(&zs, Z_FINISH);
            if (inflate_status != Z_STREAM_END) {
                inflateEnd(&zs);
                throw std::runtime_error("Zlib failed to decompress entire block!");
            }

            if (zs.total_out != uncompressed_size) {
                std::cerr << "total_out: " << zs.total_out << " uncompressed_size: " << uncompressed_size << std::endl;
                throw std::runtime_error("Uncompressed block size does not match expected!");
            }

            auto reset_status = inflateReset(&zs);
            if (reset_status != Z_OK) {
                throw std::runtime_error("Failed to reset zlib!");
            }
        }

    };

    std::vector<std::thread> thread_vec;
    thread_vec.reserve(NUM_THREADS);

    auto start = std::chrono::steady_clock::now();
    for (int k = 0; k < NUM_THREADS; ++k) {
        thread_vec.emplace_back(deflateProc);
    }

    for (auto & thread : thread_vec) {
        thread.join();
    }
    auto stop = std::chrono::steady_clock::now();

    double file_size_compressed_MB = reader.getFileSize() / (1024 * 1024.0);
    double time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / 1000000000.0;

    std::cout << "Deflating took: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;
    std::cout << "Deflated file at " << file_size_compressed_MB / time << " compressed MB/s" << std::endl;
}
