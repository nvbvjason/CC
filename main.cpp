#include "Program.h"

int main(const int argc, char *argv[])
{
    const Program program(argc, argv);
    return program.run();
}