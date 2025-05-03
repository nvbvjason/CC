set(LEXING_FILES
        Lexing/Token.cpp
        Lexing/Token.hpp

        Lexing/Lexer.cpp
        Lexing/Lexer.hpp)

set(PARSING_FILES
        Parsing/Parser.cpp
        Parsing/Parser.hpp
        Parsing/ASTParser.cpp
        Parsing/ASTParser.hpp
        Parsing/ASTPrinter.cpp
        Parsing/ASTPrinter.hpp
        Parsing/ASTVisitor.cpp
        Parsing/ASTVisitor.hpp
        Parsing/AstTraverser.cpp
        Parsing/AstTraverser.hpp
        Parsing/ASTFwd.cpp
        Parsing/ASTFwd.hpp
        Parsing/ConstAstTraverser.cpp
        Parsing/ConstAstTraverser.hpp)

set(SEMANTICS_FILES
        Semantics/VariableResolution.cpp
        Semantics/VariableResolution.hpp
        Semantics/ValidateReturn.cpp
        Semantics/ValidateReturn.hpp
        Semantics/LvalueVerification.cpp
        Semantics/LvalueVerification.hpp
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