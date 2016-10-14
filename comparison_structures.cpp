#include <chrono>
#include <iostream>
#include <random>

std::random_device rand_device;
std::default_random_engine engine(rand_device());
std::uniform_int_distribution<int> distribution(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
 
namespace {
    volatile int a = distribution(engine);
    volatile int b = distribution(engine);
    volatile int c = distribution(engine);
    volatile int d = distribution(engine);
    volatile int e = distribution(engine);
    volatile int f = distribution(engine);
    volatile int g = distribution(engine);
    volatile int h = distribution(engine);
}

int compareMethod2() {
    if (a != b)
        return a - b;
    else if (c != d)
        return c - d;
    else if (e != f)
        return e - f;
    else if (g != h)
        return g - h;
    else 
        return 0;
}

int compareMethod1() {
    int retval = 0;

    if (retval == 0) retval = a - b;
    if (retval == 0) retval = c - d;
    if (retval == 0) retval = e - f;
    if (retval == 0) retval = g - h;

    return retval;
}



int main(int argc, char * argv[]) {
    if (argc != 2)
        return -1;

    const std::uint64_t loop_executions = std::stoull(argv[1]);
    {
        auto start = std::chrono::high_resolution_clock::now();
        for (std::uint64_t k = 0; k < loop_executions; ++k)
            compareMethod1();
        auto end = std::chrono::high_resolution_clock::now();

        std::cout << std::chrono::duration<double, std::milli>(end - start).count() << std::endl;
    }

    {
        auto start = std::chrono::high_resolution_clock::now();
        for (std::uint64_t k = 0; k < loop_executions; ++k)
            compareMethod2();
        auto end = std::chrono::high_resolution_clock::now();

        std::cout << std::chrono::duration<double, std::milli>(end - start).count() << std::endl;
    }


    return 0;
}