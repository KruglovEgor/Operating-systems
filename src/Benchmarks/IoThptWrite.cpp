#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <file> <block_size> <num_blocks>\n";
        return 1;
    }

    const char* filename = argv[1];
    size_t blockSize = std::atoi(argv[2]);
    size_t repetitions = std::atoi(argv[3]);

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file.\n";
        return 1;
    }

    std::vector<char> buffer(blockSize, 'A');
    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < repetitions; ++i) {
        file.write(buffer.data(), buffer.size());
    }

    file.close();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Write throughput: " << (blockSize * repetitions / (1024.0 * 1024.0)) / elapsed.count()
              << " MB/s\n";
}
