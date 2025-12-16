add_library(AST STATIC
        ASTFwd.hpp
        ASTVisitor.hpp
        ASTExpr.hpp
        ASTBase.hpp
        ASTUtils.cpp
        ASTUtils.hpp
        FuncEntry.hpp
)

target_include_directories(AST PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)