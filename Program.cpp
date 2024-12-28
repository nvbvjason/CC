#include "Program.h"

#include <iostream>

Program::Program(int argc, char *argv[])
{
    args = std::vector<std::string>(argv, argv + argc);
}

int Program::run() const
{
    if (args.size() < 2 || 3 < args.size()) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        return 1;
    }
    if (!fileExists(args[1])) {
        std::cerr << "File " << args[1] << " not found" << '\n';
        return 2;
    }
    if (args.size() == 3) {
        std::string argument = args[2];
        if (!isCommandLineArgumentValid(argument)) {
            std::cerr << "Invalid argument: " << argument << '\n';
            return 3;
        }
    }
    const std::string file = args[1];

    return 0;
}
