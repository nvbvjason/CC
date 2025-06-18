add_subdirectory(Frontend)
add_subdirectory(IR)
add_subdirectory(CodeGen)
add_subdirectory(Types)

# Add CompilerDriver library
add_library(CompilerDriver
        CompilerDriver.cpp
        ShortTypes.hpp
)

# Specify include directories for CompilerDriver
target_include_directories(CompilerDriver PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/IR
        ${CMAKE_SOURCE_DIR}/src/CodeGen
        ${CMAKE_SOURCE_DIR}/src/Frontend
        ${CMAKE_SOURCE_DIR}/src/Types
)

# Link CompilerDriver against required libraries
target_link_libraries(CompilerDriver PUBLIC
        CodeGen
        IR
        Frontend
        TYPES
)

# Add executable for the compiler
add_executable(CC
        main.cpp
        StateCode.hpp
)

# Specify include directories for the CC executable
target_include_directories(CC PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link CC against all required libraries
target_link_libraries(CC PRIVATE
        CompilerDriver
        Frontend
        Parsing
        Lexing
        Semantics
        FrontendIR
        AST
        IR
        CodeGen
        TYPES
)