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
        Parsing/ConcreteTree.cpp
        Parsing/ConcreteTree.hpp
        Parsing/AbstractTree.cpp
        Parsing/AbstractTree.hpp
        Parsing/AstVisualizer.cpp
        Parsing/AstVisualizer.hpp)

set(CODEGEN_FILES
        CodeGen/Assembly.cpp
        CodeGen/Assembly.hpp
        CodeGen/AbstractTree.cpp
        CodeGen/AbstractTree.hpp
        CodeGen/ConcreteTree.cpp
        CodeGen/ConcreteTree.hpp)

set(TACKY_FILES
        Tacky/AbstractTree.cpp
        Tacky/AbstractTree.hpp
        Tacky/ConcreteTree.cpp
        Tacky/ConcreteTree.hpp)

add_executable(CC_run
        main.cpp

        CompilerDriver.cpp
        CompilerDriver.hpp

        ShortTypes.hpp
        ${LEXING_FILES}
        ${PARSING_FILES}
        ${CODEGEN_FILES}
        ${TACKY_FILES})

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
        ${CODEGEN_FILES}
        ${TACKY_FILES})

target_link_libraries(CC_test GTest::gtest GTest::gtest_main)

add_test(CC CC_test)