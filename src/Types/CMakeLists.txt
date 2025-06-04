add_library(TYPES STATIC
        Type.hpp
        TypeConversion.cpp
)

target_include_directories(TYPES PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)