#include <iostream>
#include <fstream>
#include <vector>

inline bool fileExists(const std::string &name);
inline bool isCommandLineArgumentValid(const std::string &argument);

int main(const int argc, char *argv[])
{

    if (argc < 2 || 3 < argc) {
        std::cerr << "Usage: <input_file> possible-argument" << '\n';
        return 1;
    }
    if (!fileExists(argv[1])) {
        std::cerr << "File " << argv[1] << " not found" << '\n';
        return 2;
    }
    if (argc == 3) {
        std::string argument = argv[2];
        if (!isCommandLineArgumentValid(argument)) {
            std::cerr << "Invalid argument: " << argument << '\n';
            return 3;
        }
    }
    const std::string file = argv[1];

    return 0;
}

inline bool fileExists(const std::string &name)
{
    std::ifstream f(name.c_str());
    return f.good();
}

inline bool isCommandLineArgumentValid(const std::string &argument)
{
    const std::vector<std::string> validArguements = {"--help", "-h", "--version", "--lex", "--parse", "--codegen"};
    for (const std::string &validArguement : validArguements)
        if (argument == validArguement)
            return true;
    return false;
}