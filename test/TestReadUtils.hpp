#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>

std::string getSourceCode(const std::filesystem::path& inputFile);
std::string removeLinesStartingWithHashOrComment(const std::string& input);
std::string removeLinesStartingWithHash(const std::string& input);
bool lexerValid(const std::filesystem::directory_entry& filePath);
bool ParseFileAndGiveResult(const std::filesystem::directory_entry& filePath);
bool CheckSemantics(const std::filesystem::directory_entry& filePath);
bool CheckSemanticsWithInclude(const std::string& sourceCode);
bool isCFile(const std::filesystem::directory_entry& entry);
bool isHFile(const std::filesystem::directory_entry& entry);
bool isCorHFile(const std::filesystem::directory_entry& entry);
void handlePreprocessDumb(const std::unordered_map<std::string, std::string>& hFiles,
                          const std::string& line, std::string& result);
std::string buildFileWithIncludes(
    const std::filesystem::path& path,
    const std::unordered_map<std::string, std::string>& hFiles);