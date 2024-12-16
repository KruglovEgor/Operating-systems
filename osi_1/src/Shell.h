#ifndef SHELL_H
#define SHELL_H

#include <string>
#include <unordered_map>
#include <memory>  // Для использования std::unique_ptr
#include "Commands/Command.h"
#include "Commands/CdCommand.h"
#include "Commands/LsCommand.h"
#include "Commands/CatCommand.h"
#include "Commands/HelpCommand.h"
#include "Commands/HistoryCommand.h"
#include "Commands/MkdirCommand.h"
#include "Commands/RmCommand.h"
#include "Commands/TouchCommand.h"

#include "CommandExecutor.h"
#include <filesystem>

class Shell {
public:
    Shell();                 // Конструктор для регистрации команд
    void run();              // Метод для запуска командной оболочки

private:
    void handleCommand(const std::string &input);  // Обработка введенной команды
    void executeExternalCommand(const std::string &command, const std::string &args); // Выполнение внешней команды
    void addToHistory(const std::string& command);


    CommandExecutor executor;  // Обработчик внешних команд
    std::unordered_map<std::string, std::unique_ptr<Command>> commands; // Словарь команд с unique_ptr

};

#endif // SHELL_H