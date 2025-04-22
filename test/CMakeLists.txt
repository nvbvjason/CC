find_package(GTest REQUIRED)

add_executable(CC_test
        main_test.cpp
        Lexer.cpp
        Parser.cpp
)

target_link_libraries(cc_test PRIVATE
        CC_lib           # Link to core library
        GTest::gtest
        GTest::gtest_main
)

# Register test
add_test(NAME CC_test COMMAND CC_test)


add_executable(CC_test
        ../test/main_test.cpp
        ../test/Lexer.cpp
        ../test/Lexer.hpp
        ../test/Parser.cpp
        ../test/Parser.hpp

        CC_lib)

target_link_libraries(CC_test GTest::gtest GTest::gtest_main)

add_test(CC CC_test)