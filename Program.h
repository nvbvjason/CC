#pragma once

#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include <vector>
#include <fstream>

class Program {
    std::vector<std::string> args;
public:
    Program() = delete;
    Program(const Program& other) = delete;
    Program(int argc, char* argv[]);
    [[nodiscard]] int run() const;
};


static  bool fileExists(const std::string &name)
{
    std::ifstream f(name.c_str());
    return f.good();
}

static bool isCommandLineArgumentValid(const std::string &argument)
{
    const std::vector<std::string> validArguements = {"--help", "-h", "--version", "--lex", "--parse", "--codegen"};
    for (const std::string &validArguement : validArguements)
        if (argument == validArguement)
            return true;
    return false;
}


#endif //PROGRAM_H
