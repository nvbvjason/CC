add_library(AST STATIC
        ASTFwd.cpp
        ASTParser.cpp
        ASTVisitor.cpp
        ASTExpr.cpp
        ASTBase.cpp
        ASTTypes.cpp
        ASTInitializer.cpp
        ASTInitializer.hpp
)

target_include_directories(AST PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)