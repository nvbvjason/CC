add_library(Semantics
        ValidateReturn.cpp
        VariableResolution.cpp
        LvalueVerification.cpp
)

target_include_directories(Semantics PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
        ${CMAKE_SOURCE_DIR}/src/Frontend/Traverser
)

# Semantics depends on AST
target_link_libraries(Semantics PUBLIC AST)