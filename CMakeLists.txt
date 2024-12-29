cmake_minimum_required(VERSION 3.30)

project(CC)

set(CMAKE_CXX_STANDARD 20)

add_executable(CC
        main.cpp

        Program.cpp
        Program.h

        Lexing/Token.cpp
        Lexing/Token.h

        Lexing/Lexer.cpp
        Lexing/Lexer.h
        test/ManuelTesting.cpp
        test/ManuelTesting.h)