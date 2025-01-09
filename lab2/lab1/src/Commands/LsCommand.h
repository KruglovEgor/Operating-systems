#ifndef OSI_1_LSCOMMAND_H
#define OSI_1_LSCOMMAND_H


#include "Command.h"
#include <windows.h>
#include <iostream>

class LsCommand : public Command {
public:
    void execute(const std::string& args) override {
        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA("*", &findFileData);

        if (hFind == INVALID_HANDLE_VALUE) {
            std::cerr << "Error: Unable to list files in directory.\n";
            return;
        }

        do {
            std::cout << (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? "[DIR] " : "      ")
                      << findFileData.cFileName << "\n";
        } while (FindNextFileA(hFind, &findFileData) != 0);

        FindClose(hFind);
    }
};

#endif //OSI_1_LSCOMMAND_H
