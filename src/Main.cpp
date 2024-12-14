#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <direct.h>
#include "Shell.h"



int main() {
    Shell shell;

    std::cout << "Custom Shell (type 'exit' to quit)\n";

    while (true) {

        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            std::cout << "shell " << cwd << "> ";
        } else {
            std::cerr << "getcwd() error" << std::endl;
        }

        std::string input;
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        try {
            shell.execute(input);
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << "\n";
        }
    }

    return 0;
}