add_library(Lexing STATIC
        Lexer.cpp
        Token.cpp
        TokenStore.cpp
        TokenStore.hpp
)

target_include_directories(Lexing PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(Lexing PUBLIC
        Frontend
)