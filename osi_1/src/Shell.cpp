#include <sstream>
#include "Shell.h"


// Конструктор, регистрирующий команды
Shell::Shell() {
    commands["cd"] = std::make_unique<CdCommand>();
    commands["ls"] = std::make_unique<LsCommand>();
    commands["cat"] = std::make_unique<CatCommand>();
    commands["mkdir"] = std::make_unique<MkdirCommand>();
    commands["help"] = std::make_unique<HelpCommand>();
    commands["rm"] = std::make_unique<RmCommand>();
    commands["touch"] = std::make_unique<TouchCommand>();

    // Регистрация команды history с передачей ссылки на вектор истории
    commands["history"] = std::make_unique<HistoryCommand>();

}

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
            addToHistory(input); // Сохранение в историю
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

std::vector<DWORD> Shell::executeExternalCommand(const std::string &command, const std::string &args) {
    try {
        std::vector<std::string> argsVector;
        if (!args.empty()) {
            std::istringstream iss(args);
            std::string arg;
            while (iss >> arg) {
                argsVector.push_back(arg);
            }
        }

        // Поиск аргумента --n=X
        int repetitions = 1; // По умолчанию запускаем 1 раз
        for (auto it = argsVector.begin(); it != argsVector.end(); ++it) {
            if (it->rfind("--n=", 0) == 0) { // Если аргумент начинается с --n=
                try {
                    repetitions = std::stoi(it->substr(4)); // Извлечение числа X
                    argsVector.erase(it); // Удаляем параметр --n=X из списка
                } catch (const std::invalid_argument&) {
                    throw std::runtime_error("Invalid value for --n=X");
                }
                break;
            }
        }

        // Ищем полный путь команды
        std::string resolvedPath = executor.resolveFullPath(command);
        if (resolvedPath.empty()) {
            throw std::runtime_error("Command not found: " + command);
        }

        auto start = std::chrono::high_resolution_clock::now();
        std::vector<DWORD> pids; // Для сохранения PID всех процессов


        // Многократный запуск команды
        for (int i = 0; i < repetitions; ++i) {

            // Создаём модифицированный список аргументов для каждого запуска
            std::vector<std::string> modifiedArgs = argsVector;
            for (auto &arg : modifiedArgs) {
                if (arg.size() > 4 && arg.substr(arg.size() - 4) == ".dat") {
                    // Заменяем имя файла на уникальное
                    arg = arg.substr(0, arg.size() - 4) + "_" + std::to_string(i + 1) + ".dat";
                }
            }
            std::cout << "Starting instance " << (i + 1) << " of " << repetitions << "\n";
            if (i == 0){
                start = std::chrono::high_resolution_clock::now();
            }
            DWORD pid = executor.execute(resolvedPath, modifiedArgs);
            pids.push_back(pid); // Сохраняем PID

        }
        executor.waitForAllProcesses();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;
        std::cout << "All processes completed in " << elapsed.count() << " seconds." << std::endl;

        return pids; // Возвращаем список PID
    } catch (const std::exception &e) {
        throw std::runtime_error(std::string("Failed to execute external command: ") + e.what());
    }
}

void Shell::addToHistory(const std::string& command) {
    // Находим команду history
    auto it = commands.find("history");
    if (it != commands.end()) {
        auto historyCommand = dynamic_cast<HistoryCommand*>(it->second.get());
        if (historyCommand) {
            historyCommand->addToHistory(command); // Добавляем в историю
        }
    }
}