find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(CC_test
        main_test.cpp
        Codegen.cpp
        Lexer.cpp
        Parser.cpp
        Codegen.hpp
        Lexer.hpp
        Parser.hpp
)

target_include_directories(CC_test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/Frontend
        ${CMAKE_SOURCE_DIR}/src/Frontend/Lexing
        ${CMAKE_SOURCE_DIR}/src/Frontend/Parsing
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
        ${CMAKE_SOURCE_DIR}/src/Frontend/Semantics
        ${CMAKE_SOURCE_DIR}/src/Frontend/IR
        ${CMAKE_SOURCE_DIR}/src/IR
        ${CMAKE_SOURCE_DIR}/src/CodeGen
)

target_link_libraries(CC_test PRIVATE
        CompilerDriver
        Frontend
        Parsing
        Lexing
        Semantics
        FrontendIR
        AST
        IR
        CodeGen
        GTest::gtest
        GTest::gtest_main
)

enable_testing()
add_test(NAME CC_test COMMAND CC_test)