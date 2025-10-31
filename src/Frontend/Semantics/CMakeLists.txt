add_library(Semantics STATIC
        ValidateReturn.cpp
        VariableResolution.cpp
        LvalueVerification.cpp
        GotoLabelsUnique.cpp
        LoopLabeling.cpp
        TypeResolution.cpp
        Utils.hpp
        Utils.cpp
        TypeCheckAndConvert.cpp
        TypeCheckAndConvert.hpp
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