#include "CodeGenDriver.hpp"
#include "AsmPrinter.hpp"
#include "Assembly.hpp"
#include "FixUpInstructions.hpp"
#include "GenerateAsmTree.hpp"
#include "PseudoRegisterReplacer.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

namespace CodeGen {

void run(const Ir::Program& irProgram,
         const std::string& argument,
         const std::string& inputFile)
{
    Program codegenProgram = codegen(irProgram);
    if (argument == "--codegen")
        return;
    if (argument == "--printAsm") {
        AsmPrinter printer;
        std::cout << printer.printProgram(codegenProgram);
        return;
    }
    fixAsm(codegenProgram);
    if (argument == "--printAsmAfter") {
        AsmPrinter printer;
        std::cout << printer.printProgram(codegenProgram);
        return;
    }
    std::string output = asmProgram(codegenProgram);
    std::string outputFileName = writeAsmFile(inputFile, output);
    if (argument == "--assemble")
        return;
    if (argument == "-c")
        makeLib(outputFileName, inputFile.substr(0, inputFile.length() - 2));
    else if (argument.starts_with("-l"))
        linkLib(outputFileName, inputFile.substr(0, inputFile.length() - 2), argument);
    else
        assemble(outputFileName, inputFile.substr(0, inputFile.length() - 2));
}

Program codegen(const Ir::Program& irProgram)
{
    Program codegenProgram;
    GenerateAsmTree generateAsmTree;
    generateAsmTree.genProgram(irProgram, codegenProgram);
    return codegenProgram;
}

i32 replacingPseudoRegisters(const Function& function)
{
    PseudoRegisterReplacer pseudoRegisterReplacer;
    for (const auto& inst : function.instructions)
        inst->accept(pseudoRegisterReplacer);
    return pseudoRegisterReplacer.stackPointer();
}

void fixUpInstructions(Function& function, const i32 stackAlloc)
{
    FixUpInstructions fixUpInstructions(function.instructions, stackAlloc);
    fixUpInstructions.fixUp();
}

void fixAsm(const Program& codegenProgram)
{
    for (auto& topLevel : codegenProgram.topLevels) {
        if (topLevel->kind != TopLevel::Kind::Function)
            continue;
        const auto function = dynamic_cast<Function*>(topLevel.get());
        const i32 stackAlloc = replacingPseudoRegisters(*function);
        fixUpInstructions(*function, stackAlloc);
    }
}

std::string writeAsmFile(const std::string& inputFile, const std::string& output)
{
    std::string stem = std::filesystem::path(inputFile).stem().string();
    const std::string inputFolder = std::filesystem::path(inputFile).parent_path().string();
    std::filesystem::path inputPath(inputFile);
    std::filesystem::path outputPath = inputPath.parent_path() / (inputPath.stem().string() + ".s");
    std::string outputFileName = outputPath.string();
    std::ofstream ofs(outputFileName);
    if (!ofs) {
        std::cerr << "Error: Could not open output file " << outputFileName << '\n';
        return outputFileName;
    }
    ofs << output;
    ofs.close();
    return outputFileName;
}

void assemble(const std::string& asmFile, const std::string& outputFile)
{
    const std::string command = "gcc " + asmFile + " -o " + outputFile;
    std::system(command.c_str());
}

void linkLib(const std::string& asmFile, const std::string& outputFile, const std::string& argument)
{
    const std::string command = "gcc " + asmFile + " -o " + outputFile + " " + argument;
    std::system(command.c_str());
}

void makeLib(const std::string& asmFile, const std::string& outputFile)
{
    const std::string command = "gcc -c " + asmFile + " -o " + outputFile + ".o";
    std::system(command.c_str());
}
} // CodeGen