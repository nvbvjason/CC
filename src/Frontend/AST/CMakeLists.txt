add_library(AST
        ASTFwd.cpp
        ASTParser.cpp
        ../Traverser/ASTPrinter.cpp
        ASTVisitor.cpp
        ASTExpr.cpp
        ASTExpr.hpp
        ASTBase.cpp
        ASTBase.hpp
        AstTypes.cpp
        AstTypes.hpp
)

target_include_directories(AST PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)