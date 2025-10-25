add_subdirectory(Lexing)
add_subdirectory(Parsing)
add_subdirectory(Semantics)
add_subdirectory(IR)
add_subdirectory(AST)
add_subdirectory(Traverser)

add_library(Frontend
        FrontendDriver.cpp
        SymbolTable.cpp
        Error.hpp
)

# Link subdirectory libraries
target_link_libraries(Frontend PUBLIC
        Lexing
        AST
        Parsing
        Semantics
        ParsingTraversers
        FrontendIR  # Avoid name clash with src/IR
)

# Include directories for Frontend
target_include_directories(Frontend PUBLIC
)