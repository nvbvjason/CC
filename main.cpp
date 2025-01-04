#include "CompilerDriver.hpp"

#include <filesystem>

void cleanUp();

int main(const int argc, char *argv[])
{
    const CompilerDriver program(std::vector<std::string>(argv, argv + argc));
    const i32 returnCode = program.run();
    cleanUp();
    return returnCode;
}

void cleanUp()
{
    for (const auto& entry : std::filesystem::directory_iterator("/home/jason/src/CC/generated_files/"))
        remove_all(entry.path());
}