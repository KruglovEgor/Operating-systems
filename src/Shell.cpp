#include <sstream>
#include "Shell.h"


// Конструктор, регистрирующий команды
Shell::Shell() {
    commands["cd"] = std::make_unique<CdCommand>();
    commands["ls"] = std::make_unique<LsCommand>();
}

//// Основной цикл работы shell
//void Shell::run() {
//    std::string input;
//    while (true) {
//        // Получаем текущую директорию
//        std::string currentPath = std::filesystem::current_path().string();
//
//        // Выводим приглашение с текущей директорией
//        std::cout << currentPath << " > ";
//        std::getline(std::cin, input);
//
//        if (input == "exit") {
//            break;
//        }
//
//        try {
//            executeCommand(input);
//        } catch (const std::exception& ex) {
//            std::cerr << "Error: " << ex.what() << std::endl;
//        }
//    }
//}
//
//// Выполнение команды
//void Shell::executeCommand(const std::string& input) {
//    std::istringstream iss(input);
//    std::string command;
//    iss >> command;
//
//    std::string args;
//    std::getline(iss, args);
//    args.erase(0, args.find_first_not_of(' ')); // Удаление начальных пробелов
//
//    // Проверяем, зарегистрирована ли команда
//    auto it = commands.find(command);
//    if (it != commands.end()) {
//        it->second->execute(args); // Выполняем команду
//    } else {
//        // Если команда не найдена, передаем ее в CommandExecutor
//        commandExecutor.execute(command, {args});
//    }
//}

void Shell::run() {
    std::string input;
    while (true) {
        // Печать текущей директории перед вводом
        std::cout << std::filesystem::current_path().string() << " > ";
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        try {
            handleCommand(input);
        } catch (const std::exception &e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

void Shell::handleCommand(const std::string &input) {
    // Разделяем ввод на команду и аргументы
    size_t spacePos = input.find(' ');
    std::string command = spacePos == std::string::npos ? input : input.substr(0, spacePos);
    std::string args = spacePos == std::string::npos ? "" : input.substr(spacePos + 1);

    // Ищем команду в зарегистрированных
    auto it = commands.find(command);
    if (it != commands.end()) {
        it->second->execute(args); // Выполняем встроенную команду
    } else {
        // Если команда не встроенная, вызываем через CommandExecutor
        executeExternalCommand(command, args);
    }
}

void Shell::executeExternalCommand(const std::string &command, const std::string &args) {
    try {
        std::vector<std::string> argsVector;
        if (!args.empty()) {
            std::istringstream iss(args);
            std::string arg;
            while (iss >> arg) {
                argsVector.push_back(arg);
            }
        }

        // Ищем полный путь команды
        std::string resolvedPath = executor.resolveFullPath(command);
        if (resolvedPath.empty()) {
            throw std::runtime_error("Command not found: " + command);
        }

        executor.execute(resolvedPath, argsVector); // Запуск команды
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Failed to execute external command: ") + e.what());
    }
}