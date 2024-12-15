#ifndef OSI_1_HELPCOMMAND_H
#define OSI_1_HELPCOMMAND_H

#include "Command.h"
#include <iostream>

class HelpCommand : public Command {
public:
    void execute(const std::string& args) override {
        std::cout << "Available commands:\n";
        std::cout << "cd <path>         - Change directory\n";
        std::cout << "cat <file>        - Display file content\n";
        std::cout << "history           - Show command history\n";
        std::cout << "mkdir <dir>       - Create a directory\n";
        std::cout << "help              - Display this help message\n";
        std::cout << "rm <file|dir>     - Remove a file or directory\n";
        std::cout << "touch <file>      - Create an empty file\n";
    }
};

#endif // OSI_1_HELPCOMMAND_H
