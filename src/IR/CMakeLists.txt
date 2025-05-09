add_library(IR
        IrPrinter.cpp
        IrPrinter.hpp
        ASTIr.cpp
        ASTIr.hpp
)

target_include_directories(IR PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)