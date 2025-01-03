#include "CompilerDriver.hpp"

int main(const int argc, char *argv[])
{
    const CompilerDriver program(std::vector<std::string>(argv, argv + argc));
    return program.run();
}