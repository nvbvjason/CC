cmake_minimum_required(VERSION 3.30)

project(CC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

set(LEXING_FILES
        Lexing/Lexeme.cpp
        Lexing/Lexeme.hpp

        Lexing/Lexer.cpp
        Lexing/Lexer.hpp)

set(PARSING_FILES
        Parsing/Parser.cpp
        Parsing/Parser.hpp
        Parsing/ConcreteTree.cpp
        Parsing/ConcreteTree.hpp
        Parsing/AbstractTree.cpp
        Parsing/AbstractTree.hpp)

set(CODEGEN_FILES
        Codegen/Assembly.cpp
        Codegen/Assembly.hpp)

add_executable(CC_run
        main.cpp

        CompilerDriver.cpp
        CompilerDriver.hpp

        ShortTypes.hpp
        ${LEXING_FILES}
        ${PARSING_FILES}
        ${CODEGEN_FILES})

add_executable(CC_test
        test/main_test.cpp
        test/Lexer.cpp
        test/Lexer.hpp
        test/Parser.cpp
        test/Parser.hpp

        CompilerDriver.cpp
        CompilerDriver.hpp

        ShortTypes.hpp
        ${LEXING_FILES}
        ${PARSING_FILES}
        ${CODEGEN_FILES})

target_link_libraries(CC_test GTest::gtest GTest::gtest_main)

add_test(CC CC_test)