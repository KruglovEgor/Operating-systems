#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <numeric>
#include <api.h>
#include <string>


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

void runIoThptWrite(const char* filename, size_t blockSize, size_t numBlocks, int repeatCount, bool cacheEnabled=false) {

    lab2_set_cache_enabled(cacheEnabled);

    // Открываем файл с использованием lab2_open
    lab2_fd fd = lab2_open(filename);
    if (fd < 0) {
        std::cerr << "Failed to open file using lab2_open.\n";
        exit(1);
    }

    std::vector<char> buffer(blockSize, 'A');
    std::vector<double> times;



    for (int i = 0; i < repeatCount; ++i) {
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t j = 0; j < numBlocks; ++j) {
            ssize_t written = lab2_write(fd, buffer.data(), buffer.size());
            if (written < 0 || static_cast<size_t>(written) != buffer.size()) {
                std::cerr << "Failed to write to file using lab2_write.\n";
                lab2_close(fd);
                exit(1);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        times.push_back(elapsed.count());

        std::cout << "Write throughput (iteration " << (i + 1) << "): "
                  << (blockSize * numBlocks / (1024.0 * 1024.0)) / elapsed.count()
                  << " MB/s, Time: " << elapsed.count() << " seconds\n";

        // Перемещаемся в начало файла для следующей итерации
        if (lab2_lseek(fd, 0, SEEK_SET) < 0) {
            std::cerr << "Failed to reset file position using lab2_lseek.\n";
            lab2_close(fd);
            exit(1);
        }
    }

    lab2_close(fd);  // Закрываем файл

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
    system("chcp 65001"); // Установить кодировку консоли на UTF-8

    // Проверка наличия флага --cache
    bool cacheEnabled = false;
    std::vector<std::string> args(argv + 1, argv + argc);
    auto cacheIt = std::find(args.begin(), args.end(), "--cache");
    if (cacheIt != args.end()) {
        cacheEnabled = true;
        args.erase(cacheIt); // Удаляем флаг из аргументов
    }

    // Поиск разделителя |
    auto separatorIt = std::find(args.begin(), args.end(), "|");
    if (separatorIt == args.end()) {
        std::cerr << "Error: Missing '|' separator.\n";
        return 1;
    }

    // Аргументы для sort
    auto sortArgs = std::vector<std::string>(args.begin(), separatorIt);
    if (sortArgs.size() < 1) {
        std::cerr << "Error: Insufficient parameters for sort.\n";
        return 1;
    }
    int arraySize = std::stoi(sortArgs[0]);
    int sortRepeatCount = (sortArgs.size() == 1) ? 1 : std::stoi(sortArgs[1]);

    // Проверка на валидность входных данных для sort
    if (arraySize <= 0 || sortRepeatCount <= 0) {
        std::cerr << "Error: Invalid parameters for sort.\n";
        return 1;
    }

    // Аргументы для io-thpt-write
    auto ioArgs = std::vector<std::string>(separatorIt + 1, args.end());
    if (ioArgs.size() < 3) {
        std::cerr << "Error: Insufficient parameters for io-thpt-write.\n";
        return 1;
    }

    const char* filename = ioArgs[0].c_str();
    size_t blockSize = std::stoi(ioArgs[1]);
    size_t numBlocks = std::stoi(ioArgs[2]);
    int writeRepeatCount = (ioArgs.size() > 3) ? std::stoi(ioArgs[3]) : 1;

    // Проверка на валидность входных данных для io-thpt-write
    if (blockSize <= 0 || numBlocks <= 0 || writeRepeatCount <= 0) {
        std::cerr << "Error: Invalid parameters for io-thpt-write.\n";
        return 1;
    }

    // Выполнение сортировки
    runSort(arraySize, sortRepeatCount);

    // Выполнение записи с измерением пропускной способности
    runIoThptWrite(filename, blockSize, numBlocks, writeRepeatCount, cacheEnabled);

    // Пауза для предотвращения закрытия консоли
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}