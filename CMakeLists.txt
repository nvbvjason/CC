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
        Parser/Program.cpp
        Parser/Program.hpp

        Parser/FunctionDefinition.cpp
        Parser/FunctionDefinition.hpp

        Parser/Statement.cpp
        Parser/Statement.hpp

        Parser/Expression.cpp
        Parser/Expression.hpp

        Parser/Parser.cpp
        Parser/Parser.hpp)

add_executable(CC_run
        main.cpp

        Program.cpp
        Program.h

        ${LEXING_FILES}
        ${PARSING_FILES})

set(TEST_SOURCES
        test/main_test.cpp

        Program.cpp
        Program.h

        ${LEXING_FILES}
        ${PARSING_FILES}
        test/Lexer.cpp
        test/Lexer.hpp)

add_executable(CC_test ${LEXING_FILES} ${TEST_SOURCES})

target_link_libraries(CC_test GTest::gtest GTest::gtest_main)

add_test(CC CC_test)