add_library(Parsing STATIC
        Parser.cpp
        Operators.cpp
        Declarator.cpp
        Declarator.hpp
)

# Specify include directories for the Parsing library
target_include_directories(Parsing PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
        ${CMAKE_SOURCE_DIR}/src/Frontend/Lexing
)

# Link against dependent libraries
target_link_libraries(Parsing PUBLIC
        AST
        Lexing
)