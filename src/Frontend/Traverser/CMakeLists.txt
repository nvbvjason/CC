add_library(ParsingTraversers STATIC
        ASTPrinter.cpp
        ASTTraverser.cpp
        ConstASTTraverser.cpp)

target_include_directories(ParsingTraversers PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
)

target_link_libraries(ParsingTraversers PUBLIC AST)