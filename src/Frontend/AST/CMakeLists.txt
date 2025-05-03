add_library(AST
        ASTFwd.cpp
        ASTParser.cpp
        ASTPrinter.cpp
        ASTVisitor.cpp
)

target_include_directories(AST PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)