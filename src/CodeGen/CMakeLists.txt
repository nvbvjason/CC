add_library(CodeGen STATIC
        AsmAST.hpp
        AsmPrinter.cpp
        Assembly.cpp
        GenerateAsmTree.cpp
        PseudoRegisterReplacer.cpp
        Operators.hpp
        FixUpInstructions.cpp
)

target_include_directories(CodeGen PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/IR
)

target_link_libraries(CodeGen PUBLIC IR)