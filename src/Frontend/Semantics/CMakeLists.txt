add_library(Semantics
        ValidateReturn.cpp
        VariableResolution.cpp
        LvalueVerification.cpp
        LabelsUnique.cpp
        LabelsUnique.hpp
        LoopLabeling.cpp
        LoopLabeling.hpp
        ScopeStack.cpp
        ScopeStack.hpp
        TypeResolution.cpp
        TypeResolution.hpp
        RemoveRedundantDecls.cpp
        RemoveRedundantDecls.hpp
        VariableResolutionNextTry.cpp
        VariableResolutionNextTry.hpp
)

target_include_directories(Semantics PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
        ${CMAKE_SOURCE_DIR}/src/Frontend/Lexing
        ${CMAKE_SOURCE_DIR}/src/Frontend/Traverser
)

# Semantics depends on AST
target_link_libraries(Semantics PUBLIC AST)