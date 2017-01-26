#include <fstream>
#include <iostream>
#include <exception>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>

#include "zlib.h"

namespace {
    const std::string BAM_FILE_PATH = "/Users/hocheung20/genome_data/aengine/aengine.150x.bam";
    const unsigned int NUM_THREADS = 8;
    const unsigned int MAX_BLOCK_SIZE = 65536;
}

class BamReader {
public:
    BamReader(const std::string & filename) {
        std::ifstream read_stream(filename, std::ios::binary | std::ios::ate);

        file_size_ = read_stream.tellg();
        std::cout << "File size: " << file_size_ << std::endl;
        read_stream.seekg(0, std::ios::beg);

        buffer_.resize(file_size_);
        auto start = std::chrono::steady_clock::now();
        if (!read_stream.read(buffer_.data(), file_size_)) {
            throw std::runtime_error("Error reading file into buffer!");
        }
        auto stop = std::chrono::steady_clock::now();
        double file_size_compressed_MB = file_size_ / (1024 * 1024.0);
        double time = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count() / 1000000000.0;
        std::cout << "ifstream read file at " << file_size_compressed_MB / time << " compressed MB/s" << std::endl;

        block_begin_it_ = buffer_.cbegin();
    }

    std::pair<std::vector<char>::const_iterator, uint32_t> getNextBlock() {
        std::lock_guard<std::mutex> lock(next_block_mutex_);
        if (block_begin_it_ == buffer_.cend())
            return { buffer_.cend(), 0 };

        uint16_t length = 0;
        std::memcpy(&length, &*(block_begin_it_+16), sizeof(length));
        length+=1;
        auto ret = std::pair<std::vector<char>::const_iterator, uint32_t>(block_begin_it_, length);
        block_begin_it_ += length;
        return ret;
    }

    uint64_t getFileSize() {
        return file_size_;
    }

private:
    std::vector<char> buffer_;

    std::mutex next_block_mutex_;
    std::vector<char>::const_iterator block_begin_it_;

    uint64_t file_size_;
};

int main() {
    BamReader reader(BAM_FILE_PATH);

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

            std::vector<char>::const_iterator block_begin_it = block.first;
            const uint32_t length = block.second;

            if (*block_begin_it != (char)31 || *(block_begin_it+1) != (char)139) {
                throw std::runtime_error("BgzfBlock header magic bytes don't match! Corrupt file?");
            }

            uint32_t uncompressed_size = 0;
            std::memcpy(&uncompressed_size, &*(block_begin_it+length-4), sizeof(uncompressed_size));


            zs.next_in = (Bytef *)&*(block_begin_it+18);
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
