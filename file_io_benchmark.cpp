#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <fstream>

int main(int argc, char * argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <num GB to write> <output_file>" << std::endl;
        return -1;
    }

    uint64_t num_bytes = std::stoi(argv[1]) * 1024ull * 1024ull * 1024ull;
    std::string output_file_path = argv[2];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0,255);

    std::vector<char> input_vec;
    input_vec.resize(num_bytes);

    std::generate(std::begin(input_vec), std::end(input_vec), [&dist, &gen](){return dist(gen);});

    std::ofstream output_stream(output_file_path, std::ios_base::binary);

    auto start = std::chrono::steady_clock::now();
    output_stream.write(input_vec.data(), input_vec.size());
    output_stream.flush();
    auto stop = std::chrono::steady_clock::now();

    std::cout << "Writing " << input_vec.size() << " bytes took: " << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;
    std::cout << (num_bytes / 1024.0 * 1024 * 1024) / (std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count() * 1000000.0) << " GiB/s" << std::endl;


    return 0;
}
