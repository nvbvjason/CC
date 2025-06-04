add_library(Lexing STATIC
        Lexer.cpp
        Token.cpp
)

target_include_directories(Lexing PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)