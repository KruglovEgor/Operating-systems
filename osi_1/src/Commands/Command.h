#ifndef OSI_1_COMMAND_H
#define OSI_1_COMMAND_H


#include <string>

// Базовый класс для команд
class Command {
public:
    virtual ~Command() = default;
    virtual void execute(const std::string& args) = 0;
};


#endif //OSI_1_COMMAND_H
