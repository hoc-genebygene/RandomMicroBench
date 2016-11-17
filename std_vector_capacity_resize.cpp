#include <chrono>
#include <iostream>
#include <random>

int unconditional_clear(int resize_iterations, std::uniform_int_distribution<> & dist, std::mt19937 & gen) {
    std::vector<int> vec;
    
    auto begin = std::chrono::high_resolution_clock::now();
    for (int k = 0; k < resize_iterations; ++k) {
        int size = dist(gen);
        
        vec.clear();
        
        vec.resize(size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

int conditional_clear(int resize_iterations, std::uniform_int_distribution<> & dist, std::mt19937 & gen) {
    std::vector<int> vec;
    
    auto begin = std::chrono::high_resolution_clock::now();
    for (int k = 0; k < resize_iterations; ++k) {
        int size = dist(gen);
        
        if (size > vec.capacity())
            vec.clear();
        
        vec.resize(size);
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
}

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1,500);
    
    int test_iterations = 10;
    int resize_iterations = 100*1000*1000;
    
    double unconditional_sum = 0.0;
    for (int k = 0; k < test_iterations; ++k)
        unconditional_sum += unconditional_clear(resize_iterations, dist, gen);
    
    double conditional_sum = 0.0;
    for (int k = 0; k < test_iterations; ++k)
        conditional_sum += conditional_clear(resize_iterations, dist, gen);
    
    std::cout << "Unconditional avg execution time: " << unconditional_sum / test_iterations << " milliseconds" << std::endl;
    std::cout << "Conditional avg execution time: " << conditional_sum / test_iterations << " milliseconds" << std::endl;
}
