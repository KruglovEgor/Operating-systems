#ifndef OSI_1_CATCOMMAND_H
#define OSI_1_CATCOMMAND_H

#include "Command.h"
#include <fstream>
#include <iostream>

class CatCommand : public Command {
public:
    void execute(const std::string& args) override {
        if (args.empty()) {
            std::cerr << "Error: No file provided for cat.\n";
            return;
        }

        std::string filePath = trimQuotes(args);

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filePath << "\n";
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << "\n";
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

#endif // OSI_1_CATCOMMAND_H
