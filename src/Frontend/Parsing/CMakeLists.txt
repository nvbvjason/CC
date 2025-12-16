add_library(Parsing STATIC
        Parser.cpp
        Operators.hpp
        Declarator.hpp
)

target_include_directories(Parsing PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
        ${CMAKE_SOURCE_DIR}/src/Frontend/Lexing
)

target_link_libraries(Parsing PUBLIC
        AST
        Lexing
)