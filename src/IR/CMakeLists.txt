add_library(IR STATIC
        IrPrinter.cpp
        ASTIr.cpp
)

target_include_directories(IR PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)