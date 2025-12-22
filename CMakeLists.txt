cmake_minimum_required(VERSION 3.14)
project(ColonySimulator)

if (UNIX)
    set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "Build GLFW Wayland support")
    set(GLFW_BUILD_X11 ON CACHE BOOL "Build GLFW X11 support")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Raylib
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/raylib ${CMAKE_BINARY_DIR}/_deps/raylib-build)

# Raygui
add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty/raygui/projects/CMake ${CMAKE_BINARY_DIR}/_deps/raygui-build)

file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE include/)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -static)
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib raygui)

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
endif()
