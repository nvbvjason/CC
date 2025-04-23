#include "CompilerDriver.hpp"

#include <filesystem>

void cleanUp();

int main(const int argc, char *argv[])
{
    cleanUp();
    const CompilerDriver program(std::vector<std::string>(argv, argv + argc));
    const i32 returnCode = program.run();
    return returnCode;
}

void cleanUp()
{
    const std::filesystem::path generatedDir = std::filesystem::path(PROJECT_ROOT_DIR) / "generated_files";
    for (const auto& entry : std::filesystem::directory_iterator(generatedDir))
        remove_all(entry.path());
}