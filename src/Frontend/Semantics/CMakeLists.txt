add_library(Semantics
        ValidateReturn.cpp
        VariableResolution.cpp
        LvalueVerification.cpp
        GotoLabelsUnique.cpp
        GotoLabelsUnique.hpp
        LoopLabeling.cpp
        LoopLabeling.hpp
        TypeResolution.cpp
        TypeResolution.hpp
)

target_include_directories(Semantics PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
        ${CMAKE_SOURCE_DIR}/src/Frontend/Lexing
        ${CMAKE_SOURCE_DIR}/src/Frontend/Traverser
        ${CMAKE_SOURCE_DIR}/src/Types
)

# Semantics depends on AST
target_link_libraries(Semantics PUBLIC AST Frontend TYPES)