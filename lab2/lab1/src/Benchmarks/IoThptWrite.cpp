#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <api.h>


int main(int argc, char* argv[]) {
    system("chcp 65001"); // Установить кодировку консоли на UTF-8

    lab2_set_cache_enabled(true);

    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <file> <block_size> <num_blocks> [repeat_count]\n";
        return 1;
    }

    const char* filename = argv[1];
    size_t blockSize = std::atoi(argv[2]);
    size_t numBlocks = std::atoi(argv[3]);
    int repeatCount = (argc > 4) ? std::atoi(argv[4]) : 1;  // Повторения по умолчанию = 1

    if (blockSize <= 0) {
        std::cerr << "Error: block_size must be a positive integer.\n";
        return 1;
    }

    if (numBlocks <= 0) {
        std::cerr << "Error: num_blocks must be a positive integer.\n";
        return 1;
    }

    if (repeatCount <= 0) {
        std::cerr << "Error: Invalid repeat count.\n";
        return 1;
    }

    // Открываем файл с использованием lab2_open
    lab2_fd fd = lab2_open(filename);
    if (fd < 0) {
        std::cerr << "Failed to open file using lab2_open.\n";
        return 1;
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
                return 1;
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
            return 1;
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
    
    // Пауза для предотвращения закрытия консоли
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}