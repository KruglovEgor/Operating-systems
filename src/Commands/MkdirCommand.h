#ifndef OSI_1_MKDIRCOMMAND_H
#define OSI_1_MKDIRCOMMAND_H

#include "Command.h"
#include <filesystem>
#include <iostream>

class MkdirCommand : public Command {
public:
    void execute(const std::string& args) override {
        if (args.empty()) {
            std::cerr << "Error: No directory name provided for mkdir.\n";
            return;
        }

        std::string dirPath = trimQuotes(args);

        if (std::filesystem::exists(dirPath)) {
            std::cerr << "Error: Directory already exists: " << dirPath << "\n";
            return;
        }

        if (!std::filesystem::create_directory(dirPath)) {
            std::cerr << "Error: Could not create directory: " << dirPath << "\n";
        } else {
            std::cout << "Directory created: " << dirPath << "\n";
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

#endif // OSI_1_MKDIRCOMMAND_H
