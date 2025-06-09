cmake_minimum_required(VERSION 3.30)

project(CC)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# Get the root directory first
get_filename_component(PROJECT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

# Then add the definition
add_compile_definitions(PROJECT_ROOT_DIR="${PROJECT_ROOT_DIR}")

add_subdirectory(src)
add_subdirectory(test)