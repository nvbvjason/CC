add_library(FrontendIR STATIC
        GenerateIr.cpp
)

target_include_directories(FrontendIR PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/Frontend/AST
        ${CMAKE_SOURCE_DIR}/src/Frontend
        ${CMAKE_SOURCE_DIR}/src/IR
)

target_link_libraries(FrontendIR PUBLIC
        AST
        IR
)