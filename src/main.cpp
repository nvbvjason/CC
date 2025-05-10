#include "CompilerDriver.hpp"

int main(const int argc, char *argv[])
{
    CompilerDriver program(argc, argv);
    return program.run();
}