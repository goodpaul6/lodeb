cmake_minimum_required(VERSION 3.12.0)
set(CMAKE_CXX_STANDARD 20)

project(lodeb VERSION 0.1.0 LANGUAGES C CXX)

include(ExternalProject)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# globbing up sources
# - scaffold & non-cmake vendor sources
file(GLOB scaffold_sources CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/scaffold/source/*.cpp
    ${CMAKE_SOURCE_DIR}/scaffold/builtin/*.cpp
    ${CMAKE_SOURCE_DIR}/vendor/imgui/*.cpp
    ${CMAKE_SOURCE_DIR}/vendor/glad/src/gl.c
    ${CMAKE_SOURCE_DIR}/vendor/tinygltf/tinygltf_impl.cpp
    ${CMAKE_SOURCE_DIR}/vendor/tinyfiledialogs/*.cpp
)

# - project sources
file(GLOB project_sources CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/lodeb/*.cpp
)

# register sources
add_executable(${CMAKE_PROJECT_NAME} 
    main.cpp 
    ${scaffold_sources}
    ${project_sources}
)

find_path(LLVM_INCLUDE_DIR
    NAMES libunwind.h
    PATHS
        /usr/local/opt/llvm/include
        /opt/homebrew/opt/llvm/include
        NO_DEFAULT_PATH
        # TODO(Apaar): Add windows search paths
)

if(NOT LLVM_INCLUDE_DIR)
    message(FATAL_ERROR "LLVM include directory not found.")
endif()

# Find LLDB library
find_library(LLDB_LIBRARY
    NAMES lldb
    PATHS
        /usr/local/opt/llvm/lib
        /opt/homebrew/opt/llvm/lib
        # TODO(Apaar): Add windows paths
)

if(NOT LLDB_LIBRARY)
    message(FATAL_ERROR "LLDB library not found.")
endif()

message(STATUS "LLVM Include Directory: ${LLVM_INCLUDE_DIR}")
message(STATUS "LLDB Library: ${LLDB_LIBRARY}")

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

# exposing includes
# - scaffold & vendor includes
target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/scaffold/include
    ${CMAKE_SOURCE_DIR}/vendor/glfw-3.3.8/include/GLFW
    ${CMAKE_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/glm
    ${CMAKE_SOURCE_DIR}/vendor/tinyfiledialogs
    ${LLVM_INCLUDE_DIR}
)

# - project includes
#target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
#    ${CMAKE_SOURCE_DIR}path/to/includes
#)

# vendor specific
# - glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(${CMAKE_SOURCE_DIR}/vendor/glfw-3.3.8)

target_link_libraries(${CMAKE_PROJECT_NAME} 
    glfw
    ${LLDB_LIBRARY}
)
