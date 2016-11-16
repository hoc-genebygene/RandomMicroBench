#include <future>
#include <chrono>
#include <vector>
#include <string>
#include <iostream>

int main(int argc, char * argv[]) {
	auto begin = std::chrono::high_resolution_clock::now();
	std::vector<std::future<void>> futures;
	for (int k = 0; k < std::stoi(argv[1]); ++k) {
		futures.emplace_back(std::async(std::launch::async, [](){}));
	}

	for (auto & future: futures) {
		future.wait();
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << "Time elapsed: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " milliseconds" << std::endl;
}