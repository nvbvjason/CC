add_library(CodeGen STATIC
        AsmAST.cpp
        AsmPrinter.cpp
        Assembly.cpp
        FixUpInstructions.cpp
        GenerateAsmTree.cpp
        PseudoRegisterReplacer.cpp
)

target_include_directories(CodeGen PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/IR
)

target_link_libraries(CodeGen PUBLIC IR)