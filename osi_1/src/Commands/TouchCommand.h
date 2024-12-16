#ifndef OSI_1_TOUCHCOMMAND_H
#define OSI_1_TOUCHCOMMAND_H

#include "Command.h"
#include <filesystem>
#include <iostream>
#include <fstream>

class TouchCommand : public Command {
public:
    void execute(const std::string& args) override {
        if (args.empty()) {
            std::cerr << "Error: No file provided for touch.\n";
            return;
        }

        std::string filePath = trimQuotes(args);

        if (std::filesystem::exists(filePath)) {
            std::cerr << "Error: File already exists: " << filePath << "\n";
            return;
        }

        std::ofstream file(filePath);
        if (file) {
            std::cout << "File created: " << filePath << "\n";
        } else {
            std::cerr << "Error: Could not create file: " << filePath << "\n";
        }
    }

private:
    std::string trimQuotes(const std::string& str) {
        if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
            return str.substr(1, str.size() - 2);
        }
        return str;
    }
};

#endif // OSI_1_TOUCHCOMMAND_H
