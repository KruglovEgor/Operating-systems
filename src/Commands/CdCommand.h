#ifndef OSI_1_CDCOMMAND_H
#define OSI_1_CDCOMMAND_H


#include "Command.h"
#include <windows.h>
#include <iostream>

class CdCommand : public Command {
public:
    void execute(const std::string& args) override {
        if (args.empty()) {
            std::cerr << "Error: No path provided for cd.\n";
            return;
        }

        if (!SetCurrentDirectoryA(args.c_str())) {
            std::cerr << "Error: Could not change directory to " << args << "\n";
        }
    }
};


#endif //OSI_1_CDCOMMAND_H
