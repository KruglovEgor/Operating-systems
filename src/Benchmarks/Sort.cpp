#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib> // Для std::atoi

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: Please provide the size of the array.\n";
        return 1;
    }

    int arraySize = std::atoi(argv[1]);
    if (arraySize <= 0) {
        std::cerr << "Error: Invalid array size.\n";
        return 1;
    }

    std::vector<int> numbers(arraySize);
    std::generate(numbers.begin(), numbers.end(), std::rand);

    auto start = std::chrono::high_resolution_clock::now();
    std::sort(numbers.begin(), numbers.end());
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Sort completed in " << elapsed.count() << " seconds.\n";

    return 0;
}
