#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>
#include <pdh.h>


std::vector<HANDLE> runningProcesses;

void logMetrics(const std::string& processName, DWORD pid, const std::string& logFile) {
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

            log << "Process: " << processName
                << " | PID: " << pid
                << " | User Time: " << userSeconds << "s"
                << " | Kernel Time: " << kernelSeconds << "s\n";
        } else {
            log << "Failed to query times for PID " << pid << "\n";
        }

        std::this_thread::sleep_for(std::chrono::seconds(1)); // Период обновления
    }
}

void launchProcess(const std::string command, const std::wstring &fullPath, const std::wstring &commandLine, const std::string& logFile) {
    // Инициализация структуры для CreateProcess
    STARTUPINFOW startupInfo = { sizeof(startupInfo) };
    PROCESS_INFORMATION processInfo;

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

    std::cout << "Process " << command << " launched successfully! PID: " << processInfo.dwProcessId << std::endl;

    runningProcesses.push_back(processInfo.hProcess);
    std::thread logThread(logMetrics, command, processInfo.dwProcessId, logFile);
    logThread.detach();
    // Дескриптор потока можно закрыть сразу, он больше не нужен
    CloseHandle(processInfo.hThread);
}


int main() {
    // Настройки теста
    std::string logFile = "metrics.log";
    std::wstring workloadSort = L"E:\\Program Files\\CLionProjects\\osi_1\\cmake-build-release\\sort.exe";
    std::wstring workloadIO = L"E:\\Program Files\\CLionProjects\\osi_1\\cmake-build-release\\io-thpt-write.exe";
    std::string commandSort = "sort";
    std::string commandIO = "IO";
    int instancesPerWorkload = 2;

     //Запуск нагрузчиков
    for (int i = 0; i < instancesPerWorkload; ++i) {
        launchProcess(commandSort, workloadSort, L"\"" + workloadSort + L"\" 1000000000", logFile);
        launchProcess(commandIO, workloadIO, L"\"" + workloadIO + L"\" out_" + std::to_wstring(i+1) + L".dat 3072 100000", logFile);
    }

    std::cout << "All workloads launched. Metrics are being logged to: " << logFile << "\n";

    // Удерживаем процесс, чтобы дать время на сбор данных
    std::this_thread::sleep_for(std::chrono::minutes(1));

    return 0;
}
