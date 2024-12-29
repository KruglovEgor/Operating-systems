#ifndef OSI_1_COMMANDEXECUTOR_H
#define OSI_1_COMMANDEXECUTOR_H

#include <string>
#include <vector>
#include <windows.h>

class CommandExecutor {
public:
    DWORD execute(const std::string& command, const std::vector<std::string>& args = {});
    std::string resolveFullPath(const std::string& command);
    void waitForAllProcesses();
private:
    DWORD launchProcess(const std::wstring& fullPath, const std::wstring& commandLine);
    std::wstring toNtPath(const std::string& fullPath);

};

#endif // OSI_1_COMMANDEXECUTOR_H
