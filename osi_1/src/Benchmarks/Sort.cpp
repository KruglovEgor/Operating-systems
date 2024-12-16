#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib> // Для std::atoi
#include <numeric>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Error: Please provide the size of the array.\n";
        return 1;
    }

    int arraySize = std::atoi(argv[1]);
    int repeatCount = 1; // Значение по умолчанию для повторений
    std::vector<double> times;


    if (argc > 2) {
        repeatCount = std::atoi(argv[2]);
        if (repeatCount <= 0) {
            std::cerr << "Error: Invalid repeat count.\n";
            return 1;
        }
    }

    if (arraySize <= 0) {
        std::cerr << "Error: Invalid array size.\n";
        return 1;
    }

    for (int i = 0; i < repeatCount; ++i) {
        std::vector<int> numbers(arraySize);
        std::generate(numbers.begin(), numbers.end(), std::rand);

        auto start = std::chrono::high_resolution_clock::now();
        std::sort(numbers.begin(), numbers.end());
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> elapsed = end - start;
        times.push_back(elapsed.count());
        std::cout << "Sort iteration " << (i + 1) << " completed in " << elapsed.count() << " seconds.\n";
    }

    if (repeatCount > 1) {
        double minTime = *std::min_element(times.begin(), times.end());
        double maxTime = *std::max_element(times.begin(), times.end());
        double avgTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

        std::cout << "\nStatistics for " << repeatCount << " repeats:\n";
        std::cout << "Minimum time: " << minTime << " seconds\n";
        std::cout << "Maximum time: " << maxTime << " seconds\n";
        std::cout << "Average time: " << avgTime << " seconds\n";
    }

    return 0;
}
