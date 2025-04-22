find_package(GTest REQUIRED)

add_executable(CC_test
        main_test.cpp

        Codegen.cpp
        Codegen.hpp
        Lexer.cpp
        Lexer.hpp
        Parser.cpp
        Parser.hpp
)

target_link_libraries(CC_test PRIVATE
        CC_lib
        GTest::GTest
        GTest::Main
)

target_include_directories(CC_test PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME CC_test COMMAND CC_test)