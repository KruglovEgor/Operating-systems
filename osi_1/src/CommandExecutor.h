#ifndef OSI_1_COMMANDEXECUTOR_H
#define OSI_1_COMMANDEXECUTOR_H

#include <string>
#include <vector>

class CommandExecutor {
public:
    void execute(const std::string& command, const std::vector<std::string>& args = {});

    std::string resolveFullPath(const std::string& command);

    void waitForAllProcesses();
private:
    void launchProcess(const std::wstring& fullPath, const std::wstring& commandLine);

};

#endif // OSI_1_COMMANDEXECUTOR_H
