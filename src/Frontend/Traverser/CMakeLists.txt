add_library(ParsingTraversers
        ASTTraverser.cpp
        ConstASTTraverser.cpp)

target_include_directories(ParsingTraversers PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
)