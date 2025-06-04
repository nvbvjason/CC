add_library(CodeGen STATIC
        Assembly.cpp
        PseudoRegisterReplacer.cpp
        FixUpInstructions.cpp
        AsmAST.cpp
        GenerateAsmTree.cpp
        AsmPrinter.cpp
)

target_include_directories(CodeGen PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/IR
)

target_link_libraries(CodeGen PUBLIC IR)