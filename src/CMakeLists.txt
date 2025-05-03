set(LEXING_FILES
        Frontend/Lexing/Token.cpp
        Frontend/Lexing/Token.hpp

        Frontend/Lexing/Lexer.cpp
        Frontend/Lexing/Lexer.hpp)

set(PARSING_FILES
        Frontend/Parsing/Parser.cpp
        Frontend/Parsing/Parser.hpp
        Frontend/Parsing/ASTParser.cpp
        Frontend/Parsing/ASTParser.hpp
        Frontend/Parsing/ASTPrinter.cpp
        Frontend/Parsing/ASTPrinter.hpp
        Frontend/Parsing/ASTVisitor.cpp
        Frontend/Parsing/ASTVisitor.hpp
        Frontend/Parsing/AstTraverser.cpp
        Frontend/Parsing/AstTraverser.hpp
        Frontend/Parsing/ASTFwd.cpp
        Frontend/Parsing/ASTFwd.hpp
        Frontend/Parsing/ConstAstTraverser.cpp
        Frontend/Parsing/ConstAstTraverser.hpp)

set(SEMANTICS_FILES
        Frontend/Semantics/VariableResolution.cpp
        Frontend/Semantics/VariableResolution.hpp
        Frontend/Semantics/ValidateReturn.cpp
        Frontend/Semantics/ValidateReturn.hpp
        Frontend/Semantics/LvalueVerification.cpp
        Frontend/Semantics/LvalueVerification.hpp
)

set(IR_FILES
        IR/IrAST.cpp
        IR/IrAST.hpp
        IR/GenerateIr.cpp
        IR/GenerateIr.hpp
        IR/Printer.cpp
        IR/Printer.hpp)

set(CODEGEN_FILES
        CodeGen/Assembly.cpp
        CodeGen/Assembly.hpp
        CodeGen/AsmAST.cpp
        CodeGen/AsmAST.hpp
        CodeGen/GenerateAsmTree.cpp
        CodeGen/GenerateAsmTree.hpp
        CodeGen/PseudoRegisterReplacer.cpp
        CodeGen/PseudoRegisterReplacer.hpp
        CodeGen/FixUpInstructions.cpp
        CodeGen/FixUpInstructions.hpp
)

add_library(CC_lib STATIC
        main.cpp
        CompilerDriver.cpp
        CompilerDriver.hpp
        ShortTypes.hpp
        ${LEXING_FILES}
        ${PARSING_FILES}
        ${SEMANTICS_FILES}
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