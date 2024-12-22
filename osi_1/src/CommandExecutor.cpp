#define WIN32_LEAN_AND_MEAN // Исключает редко используемые функции из Windows.h
#include "CommandExecutor.h"
#include <windows.h>
#include <winternl.h>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <iostream>


// Добавляем хранение запущенных процессов (глобально или как член класса)
std::vector<HANDLE> runningProcesses;

void CommandExecutor::execute(const std::string& command, const std::vector<std::string>& args) {
    // Разрешаем полный путь команды
    std::string fullPath = resolveFullPath(command);
    if (fullPath.empty()) {
        throw std::runtime_error("Command not found: " + command);
    }

    // Формируем строку аргументов
    std::wstring commandLine = L"\"" + std::wstring(fullPath.begin(), fullPath.end()) + L"\"";

    for (const auto& arg : args) {
        commandLine += L" " + std::wstring(arg.begin(), arg.end());
    }

    // Запускаем процесс
    launchProcess(std::wstring(fullPath.begin(), fullPath.end()), commandLine);
}

std::string CommandExecutor::resolveFullPath(const std::string &command) {
    // Проверяем, указана ли команда как относительный путь
    if (std::filesystem::exists(command)) {
        return std::filesystem::absolute(command).string();
    }

    // Проверяем в текущей директории
    std::string fullPath = (std::filesystem::current_path() / command).string();
    if (std::filesystem::exists(fullPath)) {
        return fullPath;
    }

    // Если не найдено, возвращаем пустую строку
    return "";
}

void CommandExecutor::launchProcess(const std::wstring &fullPath, const std::wstring &commandLine) {
    // Инициализация структуры для CreateProcess
    STARTUPINFOW startupInfo = { sizeof(startupInfo) };
    PROCESS_INFORMATION processInfo;

//    std::wcout << L"  Full path: " << fullPath << std::endl;
//    std::wcout << L"  Command line: " << commandLine << std::endl;

    // Запуск процесса
    BOOL result = CreateProcessW(
            fullPath.c_str(),         // Исполняемый файл
            const_cast<wchar_t*>(commandLine.c_str()), // Полная командная строка
            nullptr,                  // Атрибуты безопасности процесса
            nullptr,                  // Атрибуты безопасности потока
            FALSE,                    // Наследование дескрипторов
            0,                        // Флаги
            nullptr,                  // Переменные окружения
            nullptr,                  // Текущая директория
            &startupInfo,             // Стартовые параметры
            &processInfo              // Информация о процессе
    );

    if (!result) {
        DWORD error = GetLastError();
        throw std::runtime_error("Failed to create process, error code: " + std::to_string(error));
    }

    std::cout << "Process launched successfully! PID: " << processInfo.dwProcessId << std::endl;


    runningProcesses.push_back(processInfo.hProcess);
    // Дескриптор потока можно закрыть сразу, он больше не нужен
    CloseHandle(processInfo.hThread);
}

// Метод для ожидания всех запущенных процессов (по необходимости)
void CommandExecutor::waitForAllProcesses() {
    for (HANDLE process : runningProcesses) {
        WaitForSingleObject(process, INFINITE);
        CloseHandle(process);
    }
    runningProcesses.clear();
}