#include "Program.h"

#include <iostream>

int main(const int argc, char *argv[])
{
    const Program program(std::vector<std::string>(argv, argv + argc));
    return program.run();
}