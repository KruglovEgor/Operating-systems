#include <iostream>
#include <cassert>
#include <cstring>
#include "include/api.h"
#include "include/lru_policy.h"

int main() {
    system("chcp 65001");  // Установить кодировку консоли на UTF-8


    const char* test_file_path = "test_file.txt";
    std::cout << "Попытка открыть файл: " << test_file_path << std::endl;


    // Тестирование LRU Cache напрямую
    LRUCache cache(1024 * 1024); // 1MB кэш

    // Запись в кэш
    const char* write_data = "Hello, LRU Cache!";
    cache.write(1, 0, write_data, strlen(write_data));
    std::cout << "Записаны данные: " << write_data << std::endl;


    // Чтение из кэша
    char read_data[256] = {0};
    size_t bytes_read = cache.read(1, 0, read_data, sizeof(read_data) - 1);
    assert(bytes_read > 0 && "Ошибка чтения из кэша");
    std::cout << "Прочитанные данные из кэша: " << read_data << std::endl;

    // Проверка, что данные в кэше совпадают с записанными
    assert(strcmp(read_data, write_data) == 0 && "Данные не совпадают");

    // Тестирование API (функции из api.cpp)
    lab2_fd fd = lab2_open(test_file_path);
    assert(fd != -1 && "Ошибка открытия файла");

    // Запись в файл
    ssize_t written = lab2_write(fd, write_data, strlen(write_data));
    assert(written == strlen(write_data) && "Ошибка записи в файл");

    // Перемещение по файлу
    off_t new_position = lab2_lseek(fd, 0, SEEK_SET);
    assert(new_position == 0 && "Ошибка перемещения по файлу");

    // Чтение из файла
    char file_data[256] = {0};
    ssize_t read = lab2_read(fd, file_data, sizeof(file_data) - 1);
    assert(read > 0 && "Ошибка чтения из файла");
    std::cout << "Прочитанные данные из файла: " << file_data << std::endl;

    // Проверка, что данные совпадают
    assert(strcmp(file_data, write_data) == 0 && "Данные не совпадают");

    // Синхронизация данных на диске
    int sync_result = lab2_fsync(fd);
    assert(sync_result == 0 && "Ошибка синхронизации");

    // Закрытие файла
    int close_result = lab2_close(fd);
    assert(close_result == 0 && "Ошибка закрытия файла");

    std::cout << "Тест завершен успешно!" << std::endl;

    return 0;
}
