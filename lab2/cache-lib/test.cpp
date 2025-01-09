#include <api.h>
#include <iostream>
#include <cassert>
#include <cstring>

void fill_buffer(char* buffer, size_t size, char value) {
    memset(buffer, value, size);
}

int main() {
    system("chcp 65001"); // Установить кодировку консоли на UTF-8

    lab2_set_cache_enabled(true);

    const char* filename = "test_file.txt";
    lab2_fd fd = lab2_open(filename);

    // 1. Запись 1024 байт
    char write_data1[1024];
    fill_buffer(write_data1, sizeof(write_data1), 'A');
    ssize_t written = lab2_write(fd, write_data1, sizeof(write_data1));
    std::cout << "Записано 1024 байта: A" << std::endl;

    // 2. Чтение 1024 байт
    off_t new_pos = lab2_lseek(fd, 0, SEEK_SET);

    char read_data1[1024] = {0};
    ssize_t read = lab2_read(fd, read_data1, sizeof(read_data1));
    std::cout << "Прочитанные данные: " << std::string(read_data1, 16) << "..." << std::endl;


    // 3. Запись 512 байт
    char write_data2[512];
    fill_buffer(write_data2, sizeof(write_data2), 'B');
    written = lab2_write(fd, write_data2, sizeof(write_data2));
    std::cout << "Записано 512 байт: B" << std::endl;

    // 4. Чтение всех данных (1024 + 512)
    char read_data2[1024 + 512] = {0};
    off_t new_position = lab2_lseek(fd, 0, SEEK_SET);

    read = lab2_read(fd, read_data2, sizeof(read_data2));
    std::cout << "Прочитанные данные: "
              << std::string(read_data2, 16) << "..."
              << std::string(read_data2 + 1024, 16) << "..." << std::endl;

    // 5. Перемещение курсора в начало
    new_position = lab2_lseek(fd, 0, SEEK_SET);

    // 6. Запись 512 других байт
    char write_data3[512];
    fill_buffer(write_data3, sizeof(write_data3), 'C');
    written = lab2_write(fd, write_data3, sizeof(write_data3));
    std::cout << "Записано 512 байт: C" << std::endl;

    // 7. Чтение всех данных после перезаписи
    char read_data3[1024 + 512] = {0};
    new_position = lab2_lseek(fd, 0, SEEK_SET);

    read = lab2_read(fd, read_data3, sizeof(read_data3));
    std::cout << "Прочитанные данные после перезаписи: "
              << std::string(read_data3, 16) << "..."
              << std::string(read_data3 + 1024, 16) << "..." << std::endl;

    // Закрытие файла
    int close_result = lab2_close(fd);

    std::cout << "Тест завершен успешно!" << std::endl;
    return 0;
}
