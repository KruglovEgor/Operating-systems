#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>

int main (int arraySize) {
    std::vector<int> numbers(arraySize);
    std::generate(numbers.begin(), numbers.end(), std::rand);

    auto start = std::chrono::high_resolution_clock::now();
    std::sort(numbers.begin(), numbers.end());
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Sort completed in " << elapsed.count() << " seconds.\n";

    return 0;
}

