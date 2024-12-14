#ifndef OSI_1_SHELL_H
#define OSI_1_SHELL_H


#include <string>
#include <map>
#include <memory>
#include "Commands/Command.h"
#include "Commands/CdCommand.h"
#include "Commands/LsCommand.h"
#include "CommandExecutor.h"

class Shell {
public:
    Shell() {
        // Регистрация команд
        commands["cd"] = std::make_unique<CdCommand>();
        commands["ls"] = std::make_unique<LsCommand>();
    }

    void execute(const std::string& input) {
        auto [command, args] = parseCommand(input);

        if (commands.find(command) != commands.end()) {
            commands[command]->execute(args);
        } else {
            // Если команда не зарегистрирована, запускаем программу
            CommandExecutor executor;
            executor.execute(command, args);
        }
    }

private:
    std::map<std::string, std::unique_ptr<Command>> commands;

    static std::pair<std::string, std::string> parseCommand(const std::string& input) {
        size_t spacePos = input.find(' ');
        std::string command = input.substr(0, spacePos);
        std::string arguments = spacePos != std::string::npos ? input.substr(spacePos + 1) : "";

        return {command, arguments};
    }
};

#endif //OSI_1_SHELL_H
