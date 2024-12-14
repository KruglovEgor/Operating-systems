#define WIN32_LEAN_AND_MEAN // Исключает редко используемые функции из Windows.h
#include "CommandExecutor.h"
#include <windows.h>
#include <winternl.h>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <iostream>


// Типы и структуры для NtCreateUserProcess
typedef struct _PS_CREATE_INFO {
    SIZE_T Size;
    ULONG State;
    union {
        struct {
            HANDLE FileHandle;
            HANDLE SectionHandle;
            ULONGLONG UserProcessParametersNative;
            ULONG Flags;
        } InitState;
        struct {
            HANDLE FileHandle;
            HANDLE SectionHandle;
            ULONGLONG UserProcessParametersNative;
            ULONG Flags;
        } FailState;
    };
} PS_CREATE_INFO, *PPS_CREATE_INFO;

typedef struct _PS_ATTRIBUTE {
    ULONG Attribute;
    SIZE_T Size;
    union {
        ULONG_PTR Value;
        PVOID ValuePtr;
    };
    PSIZE_T ReturnLength;
} PS_ATTRIBUTE, *PPS_ATTRIBUTE;

typedef struct _PS_ATTRIBUTE_LIST {
    SIZE_T TotalLength;
    PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST, *PPS_ATTRIBUTE_LIST;

// Константы для процесс-параметров
#define RTL_USER_PROC_PARAMS_NORMALIZED 0x20

// Объявляем функции из NTDLL
extern "C" {
NTSYSAPI NTSTATUS NTAPI RtlCreateProcessParametersEx(
        PVOID *pProcessParameters,
        PUNICODE_STRING ImagePathName,
        PUNICODE_STRING DllPath,
        PUNICODE_STRING CurrentDirectory,
        PUNICODE_STRING CommandLine,
        PVOID Environment,
        PUNICODE_STRING WindowTitle,
        PUNICODE_STRING DesktopInfo,
        PUNICODE_STRING ShellInfo,
        PUNICODE_STRING RuntimeData,
        ULONG Flags
);

}

extern "C" {
NTSYSAPI BOOLEAN NTAPI RtlFreeHeap(
        HANDLE HeapHandle,
        ULONG Flags,
        PVOID BaseAddress
);
}

extern "C" NTSTATUS NtCreateUserProcess(
        PHANDLE ProcessHandle,
        PHANDLE ThreadHandle,
        ACCESS_MASK ProcessDesiredAccess,
        ACCESS_MASK ThreadDesiredAccess,
        POBJECT_ATTRIBUTES ProcessObjectAttributes,
        POBJECT_ATTRIBUTES ThreadObjectAttributes,
        ULONG ProcessFlags,
        ULONG ThreadFlags,
        PVOID ProcessParameters,
        PVOID CreateInfo,
        PVOID AttributeList
);

void RtlInitUnicodeString(PUNICODE_STRING DestinationString, PCWSTR SourceString) {
    if (DestinationString != nullptr) {
        size_t length = (SourceString != nullptr) ? wcslen(SourceString) * sizeof(WCHAR) : 0;
        DestinationString->Length = (USHORT)length;
        DestinationString->MaximumLength = (USHORT)(length + sizeof(WCHAR));
        DestinationString->Buffer = (PWCHAR)SourceString;
    }
}


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

void CommandExecutor::launchProcess(const std::wstring& fullPath, const std::wstring& commandLine) {
    std::wcout << L"Launching process:\n";
    std::wcout << L"Executable Path: " << fullPath << L"\n";
    std::wcout << L"Command Line: " << commandLine << L"\n";

    HANDLE processHandle = nullptr;
    HANDLE threadHandle = nullptr;

    UNICODE_STRING imagePath, commandLineUS;
    RtlInitUnicodeString(&imagePath, fullPath.c_str());
    RtlInitUnicodeString(&commandLineUS, commandLine.c_str());

    PVOID processParameters = nullptr;
    PS_CREATE_INFO createInfo = { sizeof(PS_CREATE_INFO) };
    PS_ATTRIBUTE_LIST attributeList = { sizeof(PS_ATTRIBUTE_LIST) };

    NTSTATUS status = RtlCreateProcessParametersEx(
            &processParameters,
            &imagePath,
            nullptr,
            nullptr,
            &commandLineUS,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            RTL_USER_PROC_PARAMS_NORMALIZED
    );

    if (!NT_SUCCESS(status)) {
        throw std::runtime_error("Failed to create process parameters, NTSTATUS: " + std::to_string(status));
    }

    status = NtCreateUserProcess(
            &processHandle,
            &threadHandle,
            MAXIMUM_ALLOWED,
            MAXIMUM_ALLOWED,
            nullptr,
            nullptr,
            0,
            0,
            processParameters,
            &createInfo,
            &attributeList
    );

    if (!NT_SUCCESS(status)) {
        throw std::runtime_error("Failed to create process, NTSTATUS: " + std::to_string(status));
    }

    std::cout << "Process launched successfully: " << std::string(fullPath.begin(), fullPath.end()) << std::endl;

    if (processHandle) CloseHandle(processHandle);
    if (threadHandle) CloseHandle(threadHandle);
    if (processParameters) RtlFreeHeap(GetProcessHeap(), 0, processParameters);
}
