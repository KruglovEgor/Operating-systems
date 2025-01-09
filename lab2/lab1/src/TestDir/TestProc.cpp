#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <windows.h>
#include "../ntdll.h"
#pragma comment(lib, "ntdll")
#include <thread> // Для sleep_for
#include <sstream>


#pragma comment(lib, "pdh.lib")

std::vector<HANDLE> runningProcesses;

void logMetrics(DWORD pid, const std::string& logFile) {
    // Открываем файл для записи
    std::ofstream log(logFile, std::ios::app);
    if (!log.is_open()) {
        std::cerr << "Failed to open log file.\n";
        return;
    }

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!hProcess) {
        log << "Failed to open process " << pid << "\n";
        return;
    }

    FILETIME creationTime, exitTime, kernelTime, userTime;
    while (true) {
        if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
            // Конвертируем FILETIME в секунды
            ULARGE_INTEGER kernelTime64, userTime64;
            kernelTime64.LowPart = kernelTime.dwLowDateTime;
            kernelTime64.HighPart = kernelTime.dwHighDateTime;
            userTime64.LowPart = userTime.dwLowDateTime;
            userTime64.HighPart = userTime.dwHighDateTime;

            double userSeconds = userTime64.QuadPart / 1e7;
            double kernelSeconds = kernelTime64.QuadPart / 1e7;

            log << "PID: " << pid
                << " | User Time: " << userSeconds << "s"
                << " | Kernel Time: " << kernelSeconds << "s\n";
        } else {
            log << "Failed to query times for PID " << pid << "\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Период обновления
    }
}


DWORD launchProcess(const std::wstring &fullPath, const std::wstring &commandLine, const std::string& logFile) {
    // Путь до image файла из которого будет создан процесс
    UNICODE_STRING NtImagePath;
    RtlInitUnicodeString(&NtImagePath, (PWSTR)fullPath.c_str());

    // Параметры CommandLine
    UNICODE_STRING CommandLine;
    RtlInitUnicodeString(&CommandLine, (PWSTR)commandLine.c_str());


    // Задаем параметры процесса
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters = NULL;
    NTSTATUS status = RtlCreateProcessParametersEx(&ProcessParameters, &NtImagePath, NULL, NULL, &CommandLine, NULL, NULL, NULL, NULL, NULL, RTL_USER_PROCESS_PARAMETERS_NORMALIZED);

    if (!NT_SUCCESS(status)) {
        throw std::runtime_error("Failed to create process parameters.");
    }

    // Инициализируем структуру PS_CREATE_INFO
    PS_CREATE_INFO CreateInfo = { 0 };
    CreateInfo.Size = sizeof(CreateInfo);
    CreateInfo.State = PsCreateInitialState;

    // Инициализируем структуру PS_ATTRIBUTE_LIST
    PPS_ATTRIBUTE_LIST AttributeList = (PS_ATTRIBUTE_LIST*)RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PS_ATTRIBUTE));
    if (!AttributeList) {
        RtlDestroyProcessParameters(ProcessParameters);
        throw std::runtime_error("Failed to allocate memory for PS_ATTRIBUTE_LIST.");
    }
    AttributeList->TotalLength = sizeof(PS_ATTRIBUTE_LIST) - sizeof(PS_ATTRIBUTE);
    AttributeList->Attributes[0].Attribute = PS_ATTRIBUTE_IMAGE_NAME;
    AttributeList->Attributes[0].Size = NtImagePath.Length;
    AttributeList->Attributes[0].Value = (ULONG_PTR)NtImagePath.Buffer;

    // Создаем процесс
    HANDLE hProcess, hThread = NULL;
    status = NtCreateUserProcess(&hProcess, &hThread, PROCESS_ALL_ACCESS, THREAD_ALL_ACCESS, NULL, NULL, NULL, NULL, ProcessParameters, &CreateInfo, AttributeList);


    if (!NT_SUCCESS(status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, AttributeList);
        RtlDestroyProcessParameters(ProcessParameters);
        throw std::runtime_error("Failed to create process.");
    }

    // Получаем PID через GetProcessId
    DWORD pid = GetProcessId(hProcess);
    if (pid == 0) {
        // Ошибка получения PID
        RtlFreeHeap(RtlProcessHeap(), 0, AttributeList);
        RtlDestroyProcessParameters(ProcessParameters);
        throw std::runtime_error("Failed to retrieve PID.");
    }

    // Добавляем процесс в список запущенных
    runningProcesses.push_back(hProcess);
    std::thread logThread(logMetrics, pid, logFile);
    logThread.detach();

    // Закрываем поток, поскольку он уже запущен
    CloseHandle(hThread);

    // Очистка ресурсов
    RtlFreeHeap(RtlProcessHeap(), 0, AttributeList);
    RtlDestroyProcessParameters(ProcessParameters);

    return pid;
}

// Метод для ожидания всех запущенных процессов (по необходимости)
void waitForAllProcesses() {
    for (HANDLE process : runningProcesses) {
        WaitForSingleObject(process, INFINITE);
        CloseHandle(process);
    }
    runningProcesses.clear();
}

std::wstring makeOutPath(int index){
    std::wostringstream woss;
    woss << L"out_" << index << L".dat";
    return woss.str();
}

// Тестовая функция
void runTests() {

    std::wstring sortPath = L"\\??\\D:\\Git\\Operating-systems\\osi_1\\cmake-build-release\\sort.exe";
    std::wstring ioPath = L"\\??\\D:\\Git\\Operating-systems\\osi_1\\cmake-build-release\\io-thpt-write.exe";
    std::wstring combinedPath = L"\\??\\D:\\Git\\Operating-systems\\osi_1\\cmake-build-release\\combined.exe";

    std::wstring sortLine = L"sort.exe 100000000 2";
    std::wstring ioLineOne = L"io-thpt-write.exe";
    std::wstring ioLineTwo = L"2048 800000 2";
    std::wstring combinedLineOne = L"combined.exe 100000000 2 |";
    std::wstring combinedLineTwo = L"2048 400000 2";
    std::string logPath = "..\\src\\TestDir\\metrics.log";

    int n = 2;
    auto start = std::chrono::high_resolution_clock::now();
    for(int i = 0; i < n; ++i){
        std::wstring ioLine = ioLineOne + L" " + makeOutPath(i) + L" " + ioLineTwo;
        std::wstring combinedLine = combinedLineOne + L" " + makeOutPath(i+n) + L" " + combinedLineTwo;

        launchProcess(sortPath, sortLine, logPath);
        launchProcess(ioPath, ioLine, logPath);
        launchProcess(combinedPath, combinedLine, logPath);
    }
    std::cout << "All workloads launched. Metrics are being logged to: " << logPath << "\n";
    waitForAllProcesses();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "All processes completed in " << elapsed.count() << " seconds.\n";

}

int main(){
    runTests();
    return 0;
}