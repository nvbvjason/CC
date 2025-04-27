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
        CodeGen/ConcreteTree.hpp
        CodeGen/PseudoRegisterReplacer.cpp
        CodeGen/PseudoRegisterReplacer.hpp)

set(IR_FILES
        IR/AbstractTree.cpp
        IR/AbstractTree.hpp
        IR/ConcreteTree.cpp
        IR/ConcreteTree.hpp)

add_library(CC_lib STATIC
        main.cpp
        CompilerDriver.cpp
        CompilerDriver.hpp
        ShortTypes.hpp
        ${LEXING_FILES}
        ${PARSING_FILES}
        ${CODEGEN_FILES}
        ${IR_FILES}
)


target_include_directories(CC_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(CC main.cpp)

target_compile_options(CC PRIVATE -Wswitch -Werror=switch)

target_link_libraries(CC PRIVATE CC_lib)

set_target_properties(CC_lib PROPERTIES PUBLIC_HEADER "${LEXING_FILES};${PARSING_FILES};${CODEGEN_FILES};${IR_FILES}")