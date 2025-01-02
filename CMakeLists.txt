cmake_minimum_required(VERSION 3.30)

project(CC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)


set(LEXING_FILES
        Lexing/Token.cpp
        Lexing/Token.hpp

        Lexing/Lexer.cpp
        Lexing/Lexer.hpp)

set(PARSING_FILES
        Parser/Parser.cpp
        Parser/Parser.hpp)

set(CODEGEN_FILES
        Codegen/Assembly.cpp
        Codegen/Assembly.hpp)

add_executable(CC_run
        main.cpp

        Program.cpp
        Program.hpp

        ${LEXING_FILES}
        ${PARSING_FILES}
        ${CODEGEN_FILES})

add_executable(CC_test
        ${LEXING_FILES}
        ${PARSING_FILES}
        ${CODEGEN_FILES}

        Program.cpp
        Program.hpp

        test/main_test.cpp
        test/Lexer.cpp
        test/Lexer.hpp
        test/Parser.cpp
        test/Parser.hpp)

target_link_libraries(CC_test GTest::gtest GTest::gtest_main)

add_test(CC CC_test)