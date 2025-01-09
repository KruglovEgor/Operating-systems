#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <numeric>
#include <fstream>

void runSort(int arraySize, int repeatCount) {
    std::vector<double> times;

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
}

void runIoThptWrite(const char* filename, size_t blockSize, size_t numBlocks, int repeatCount) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return;
    }

    std::vector<char> buffer(blockSize, 'A');
    std::vector<double> times;

    for (int i = 0; i < repeatCount; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t j = 0; j < numBlocks; ++j) {
            file.write(buffer.data(), buffer.size());
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        times.push_back(elapsed.count());

        std::cout << "Write throughput (iteration " << (i + 1) << "): "
                  << (blockSize * numBlocks / (1024.0 * 1024.0)) / elapsed.count()
                  << " MB/s, Time: " << elapsed.count() << " seconds\n";

        // Перемещаемся в начало файла для следующей итерации
        file.seekp(0, std::ios::beg);
    }

    file.close();

    if (repeatCount > 1) {
        double minTime = *std::min_element(times.begin(), times.end());
        double maxTime = *std::max_element(times.begin(), times.end());
        double avgTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

        std::cout << "\nStatistics for " << repeatCount << " repeats:\n";
        std::cout << "Minimum time: " << minTime << " seconds\n";
        std::cout << "Maximum time: " << maxTime << " seconds\n";
        std::cout << "Average time: " << avgTime << " seconds\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Usage: " << argv[0] << " <sort_array_size> <sort_repeat_count> | <file> <block_size> <num_blocks> <write_repeat_count>\n";
        return 1;
    }

    // Поиск разделителя |
    int separatorIndex = -1;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "|") {
            separatorIndex = i;
            break;
        }
    }

    if (separatorIndex == -1) {
        std::cerr << "Error: Missing '|' separator.\n";
        return 1;
    }

    // Считываем аргументы для sort
    int arraySize = std::atoi(argv[1]);
    int sortRepeatCount = (separatorIndex > 2) ? std::atoi(argv[2]) : 1;

    // Проверка на валидность входных данных для sort
    if (arraySize <= 0 || sortRepeatCount <= 0) {
        std::cerr << "Error: Invalid parameters for sort.\n";
        return 1;
    }

    // Считываем аргументы для io-thpt-write
    const char* filename = argv[separatorIndex + 1];
    size_t blockSize = std::atoi(argv[separatorIndex + 2]);
    size_t numBlocks = std::atoi(argv[separatorIndex + 3]);
    int writeRepeatCount = (argc > separatorIndex + 4) ? std::atoi(argv[separatorIndex + 4]) : 1;

    // Проверка на валидность входных данных для io-thpt-write
    if (blockSize <= 0 || numBlocks <= 0 || writeRepeatCount <= 0) {
        std::cerr << "Error: Invalid parameters for io-thpt-write.\n";
        return 1;
    }

    // Выполнение сортировки
    runSort(arraySize, sortRepeatCount);

    // Выполнение записи с измерением пропускной способности
    runIoThptWrite(filename, blockSize, numBlocks, writeRepeatCount);

    // Пауза для предотвращения закрытия консоли
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}