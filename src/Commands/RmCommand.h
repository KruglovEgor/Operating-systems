#ifndef OSI_1_RMCOMMAND_H
#define OSI_1_RMCOMMAND_H

#include "Command.h"
#include <filesystem>
#include <iostream>

class RmCommand : public Command {
public:
    void execute(const std::string& args) override {
        if (args.empty()) {
            std::cerr << "Error: No file or directory provided for rm.\n";
            return;
        }

        std::string path = trimQuotes(args);

        if (!std::filesystem::exists(path)) {
            std::cerr << "Error: File or directory does not exist: " << path << "\n";
            return;
        }

        try {
            std::filesystem::remove_all(path);
            std::cout << "Removed: " << path << "\n";
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error: Could not remove " << path << " - " << e.what() << "\n";
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

#endif // OSI_1_RMCOMMAND_H
