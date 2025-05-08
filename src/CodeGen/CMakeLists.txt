add_library(CodeGen
        Assembly.cpp
        Assembly.hpp
        PseudoRegisterReplacer.cpp
        PseudoRegisterReplacer.hpp
        FixUpInstructions.cpp
        FixUpInstructions.hpp
        AsmAST.cpp
        AsmAST.hpp
        GenerateAsmTree.cpp
        GenerateAsmTree.hpp
        AsmPrinter.cpp
        AsmPrinter.hpp
)

target_include_directories(CodeGen PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/IR
)

target_link_libraries(CodeGen PUBLIC IR)