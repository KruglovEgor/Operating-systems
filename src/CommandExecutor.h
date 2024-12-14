#ifndef OSI_1_COMMANDEXECUTOR_H
#define OSI_1_COMMANDEXECUTOR_H


#include <string>
#include <windows.h>
#include <iostream>
#include <winternl.h>
#include <chrono>

extern "C" {
VOID NTAPI RtlInitUnicodeString(
        PUNICODE_STRING DestinationString,
        PCWSTR SourceString
);
}
extern "C" {
NTSYSAPI NTSTATUS NTAPI NtCreateUserProcess(
        PHANDLE ProcessHandle,
        PHANDLE ThreadHandle,
        ACCESS_MASK ProcessDesiredAccess,
        ACCESS_MASK ThreadDesiredAccess,
        PVOID ProcessObjectAttributes,
        PVOID ThreadObjectAttributes,
        ULONG ProcessFlags,
        ULONG ThreadFlags,
        PVOID ProcessParameters,
        PVOID CreateInfo,
        PVOID AttributeList
);
}


// Определение типа для функции NtCreateUserProcess
typedef NTSTATUS(NTAPI* NtCreateUserProcess_t)(
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

// Определение структуры PS_ATTRIBUTE
typedef struct _PS_ATTRIBUTE {
    ULONG_PTR Attribute; // Тип атрибута
    SIZE_T Size;         // Размер значения
    union {
        ULONG_PTR Value;
        PVOID ValuePtr;
    };
    PSIZE_T ReturnLength;
} PS_ATTRIBUTE;


// Определение структуры PS_CREATE_INFO
typedef struct _PS_CREATE_INFO {
    SIZE_T Size;
    ULONG State;
    ULONG_PTR Reserved[2];
    ULONG_PTR SuccessState[3];
} PS_CREATE_INFO;

// Определение структуры PS_ATTRIBUTE_LIST
typedef struct _PS_ATTRIBUTE_LIST {
    SIZE_T TotalLength;
    PS_ATTRIBUTE Attributes[1];
} PS_ATTRIBUTE_LIST;

class CommandExecutor {
public:
    bool execute(const std::string& command, const std::string& arguments) {
//        // Загружаем ntdll.dll
//        HMODULE ntdll = GetModuleHandleA("ntdll.dll");
//        if (!ntdll) {
//            std::cerr << "Failed to load ntdll.dll.\n";
//            return false;
//        }
//
//        // Получаем указатель на NtCreateUserProcess
//        auto NtCreateUserProcess = (NtCreateUserProcess_t)GetProcAddress(ntdll, "NtCreateUserProcess");
//        if (!NtCreateUserProcess) {
//            std::cerr << "Failed to get NtCreateUserProcess function.\n";
//            return false;
//        }
//
//        // Конвертация строки в UNICODE_STRING
//        UNICODE_STRING uApplicationName;
//        RtlInitUnicodeString(&uApplicationName, std::wstring(applicationPath.begin(), applicationPath.end()).c_str());
//
//        UNICODE_STRING uCommandLine;
//        RtlInitUnicodeString(&uCommandLine, std::wstring(arguments.begin(), arguments.end()).c_str());
//
//        // Инициализация PS_CREATE_INFO
//        PS_CREATE_INFO createInfo = {0};
//        createInfo.Size = sizeof(PS_CREATE_INFO);
//
//        // Инициализация PS_ATTRIBUTE_LIST
//        PS_ATTRIBUTE_LIST attributeList = {0};
//        attributeList.TotalLength = sizeof(PS_ATTRIBUTE_LIST);
//
//        // Переменные для процессов
//        HANDLE processHandle = nullptr;
//        HANDLE threadHandle = nullptr;
//
//        // Вызов NtCreateUserProcess
//        NTSTATUS status = NtCreateUserProcess(
//                &processHandle, &threadHandle,
//                PROCESS_ALL_ACCESS, THREAD_ALL_ACCESS,
//                nullptr, nullptr, 0, 0,
//                nullptr, &createInfo, &attributeList
//        );
//
//        if (status != 0) {
//            std::cerr << "Failed to create process, NTSTATUS: " << std::hex << status << "\n";
//            return false;
//        }
//
//        // Ожидание завершения процесса
//        WaitForSingleObject(processHandle, INFINITE);
//        CloseHandle(processHandle);
//        CloseHandle(threadHandle);
//
//        return true;
        std::wstring fullPath = resolveFullPath(std::wstring(command.begin(), command.end()));
        std::wstring commandLine = fullPath + L" " + std::wstring(arguments.begin(), arguments.end());

        std::wcout << L"Full Path: " << fullPath << std::endl;
        std::wcout << L"Command Line: " << commandLine << std::endl;

        UNICODE_STRING imagePath, commandLineUS;
        RtlInitUnicodeString(&imagePath, fullPath.c_str());
        RtlInitUnicodeString(&commandLineUS, commandLine.c_str());

        // Инициализация PS_CREATE_INFO
        PS_CREATE_INFO createInfo;
        RtlSecureZeroMemory(&createInfo, sizeof(PS_CREATE_INFO));
        createInfo.Size = sizeof(PS_CREATE_INFO);


        // Инициализация PS_ATTRIBUTE_LIST
        PS_ATTRIBUTE_LIST attributeList;
        RtlSecureZeroMemory(&attributeList, sizeof(PS_ATTRIBUTE_LIST));
        attributeList.TotalLength = sizeof(PS_ATTRIBUTE_LIST);

        HANDLE processHandle, threadHandle;
        NTSTATUS status = NtCreateUserProcess(
                &processHandle,              // ProcessHandle
                &threadHandle,               // ThreadHandle
                MAXIMUM_ALLOWED,             // ProcessDesiredAccess
                MAXIMUM_ALLOWED,             // ThreadDesiredAccess
                nullptr,                     // ProcessObjectAttributes
                nullptr,                     // ThreadObjectAttributes
                0,                           // ProcessFlags
                0,                           // ThreadFlags
                nullptr,                     // ProcessParameters
                &createInfo,                 // CreateInfo
                &attributeList               // AttributeList
        );

        if (!NT_SUCCESS(status)) {
            std::cerr << "Failed to create process, NTSTATUS: " << std::hex << status << std::endl;
            return false;
        }

        std::cout << "Process created successfully!" << std::endl;

        // Wait for the process to complete
        WaitForSingleObject(processHandle, INFINITE);

        // Close handles
        CloseHandle(processHandle);
        CloseHandle(threadHandle);
    }

private:
    static std::wstring resolveFullPath(const std::wstring& command) {
//        char cwd[1024];
//        if (_getcwd(cwd, sizeof(cwd)) == nullptr) {
//            std::cerr << "Failed to get current directory." << std::endl;
//            return L"";
//        }
//
//        std::wstring currentPath = std::wstring(cwd, cwd + strlen(cwd));
//        std::wstring fullPath = currentPath + L"\\" + command + L".exe";
        std::wstring fullPath = L".\\" + command;
        return fullPath;
    }
};




#endif //OSI_1_COMMANDEXECUTOR_H
