#ifndef OSI_1_HISTORYCOMMAND_H
#define OSI_1_HISTORYCOMMAND_H

#include "Command.h"
#include <vector>
#include <iostream>

class HistoryCommand : public Command {
private:
    std::vector<std::string> commandHistory;

public:
    void execute(const std::string& args) override {
        if (commandHistory.empty()) {
            std::cout << "No commands in history.\n";
            return;
        }

        for (size_t i = 0; i < commandHistory.size(); ++i) {
            std::cout << i + 1 << ": " << commandHistory[i] << "\n";
        }
    }

    void addToHistory(const std::string& command) {
        commandHistory.push_back(command);
    }
};

#endif // OSI_1_HISTORYCOMMAND_H
