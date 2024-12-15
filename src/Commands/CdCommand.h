#ifndef OSI_1_CDCOMMAND_H
#define OSI_1_CDCOMMAND_H

#include "Command.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <filesystem>

class CdCommand : public Command {
public:
    void execute(const std::string& args) override {
        if (args.empty()) {
            std::cerr << "Error: No path provided for cd.\n";
            return;
        }

        // Убираем кавычки вокруг пути, если они есть
        std::string trimmedArgs = trimQuotes(args);

        // Проверяем существование директории
        if (!std::filesystem::exists(trimmedArgs)) {
            std::cerr << "Error: Directory does not exist: " << trimmedArgs << "\n";
            return;
        }

        // Меняем директорию
        if (!SetCurrentDirectoryA(trimmedArgs.c_str())) {
            std::cerr << "Error: Could not change directory to " << trimmedArgs << "\n";
        } else {
            std::cout << "Changed directory to: " << trimmedArgs << "\n";
        }
    }

private:
    // Утилита для удаления кавычек из строки
    std::string trimQuotes(const std::string& str) {
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            return str.substr(1, str.size() - 2);
        }
        return str;
    }
};

#endif // OSI_1_CDCOMMAND_H

