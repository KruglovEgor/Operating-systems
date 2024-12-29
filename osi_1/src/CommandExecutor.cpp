#define WIN32_LEAN_AND_MEAN // Исключает редко используемые функции из Windows.h
#include "CommandExecutor.h"
#include <Windows.h>
#include <sstream>
#include <stdexcept>
#include <filesystem>

#include "ntdll.h"
#pragma comment(lib, "ntdll")

// Добавляем хранение запущенных процессов (глобально или как член класса)
std::vector<HANDLE> runningProcesses;

void CommandExecutor::execute(const std::string& command, const std::vector<std::string>& args) {
    // Разрешаем полный путь команды
    std::string fullPath = resolveFullPath(command);
    if (fullPath.empty()) {
        throw std::runtime_error("Command not found: " + command);
    }

    // Конвертируем путь в формат NT
    std::wstring ntPath = toNtPath(fullPath);

    // Формируем строку аргументов
    std::wstring commandLine = std::wstring(command.begin(), command.end());
    for (const auto& arg : args) {
        commandLine += L" " + std::wstring(arg.begin(), arg.end());
    }

    // Запускаем процесс
    launchProcess(ntPath, commandLine);
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


std::wstring CommandExecutor::toNtPath(const std::string& fullPath) {
    // Преобразуем путь в формат NT (\\??\\...)
    return L"\\??\\" + std::wstring(fullPath.begin(), fullPath.end());
}

void CommandExecutor::launchProcess(const std::wstring& ntPath, const std::wstring& commandLine) {
    // Path to the image file from which the process will be created
    UNICODE_STRING NtImagePath;
    RtlInitUnicodeString(&NtImagePath, (PWSTR)ntPath.c_str());

    // Command-line parameters for cmd.exe
    UNICODE_STRING CommandLine;
    RtlInitUnicodeString(&CommandLine, (PWSTR)commandLine.c_str());


    // Create the process parameters
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters = NULL;
    NTSTATUS status = RtlCreateProcessParametersEx(&ProcessParameters, &NtImagePath, NULL, NULL, &CommandLine, NULL, NULL, NULL, NULL, NULL, RTL_USER_PROCESS_PARAMETERS_NORMALIZED);

    if (!NT_SUCCESS(status)) {
        throw std::runtime_error("Failed to create process parameters.");
    }

    // Initialize the PS_CREATE_INFO structure
    PS_CREATE_INFO CreateInfo = { 0 };
    CreateInfo.Size = sizeof(CreateInfo);
    CreateInfo.State = PsCreateInitialState;

    // Initialize the PS_ATTRIBUTE_LIST structure
    PPS_ATTRIBUTE_LIST AttributeList = (PS_ATTRIBUTE_LIST*)RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PS_ATTRIBUTE));
    if (!AttributeList) {
        RtlDestroyProcessParameters(ProcessParameters);
        throw std::runtime_error("Failed to allocate memory for PS_ATTRIBUTE_LIST.");
    }
    AttributeList->TotalLength = sizeof(PS_ATTRIBUTE_LIST) - sizeof(PS_ATTRIBUTE);
    AttributeList->Attributes[0].Attribute = PS_ATTRIBUTE_IMAGE_NAME;
    AttributeList->Attributes[0].Size = NtImagePath.Length;
    AttributeList->Attributes[0].Value = (ULONG_PTR)NtImagePath.Buffer;

    // Create the process
    HANDLE hProcess, hThread = NULL;
    status = NtCreateUserProcess(&hProcess, &hThread, PROCESS_ALL_ACCESS, THREAD_ALL_ACCESS, NULL, NULL, NULL, NULL, ProcessParameters, &CreateInfo, AttributeList);


    if (!NT_SUCCESS(status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, AttributeList);
        RtlDestroyProcessParameters(ProcessParameters);
        throw std::runtime_error("Failed to create process.");
    }

    // Добавляем процесс в список запущенных
    runningProcesses.push_back(hProcess);

    // Закрываем поток, поскольку он уже запущен
    CloseHandle(hThread);

    // Очистка ресурсов
    RtlFreeHeap(RtlProcessHeap(), 0, AttributeList);
    RtlDestroyProcessParameters(ProcessParameters);
}

// Метод для ожидания всех запущенных процессов (по необходимости)
void CommandExecutor::waitForAllProcesses() {
    for (HANDLE process : runningProcesses) {
        WaitForSingleObject(process, INFINITE);
        CloseHandle(process);
    }
    runningProcesses.clear();
}