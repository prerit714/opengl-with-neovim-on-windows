cmake_minimum_required(VERSION 3.20)

set(FILENAME 2026-03-29)
set(EXECUTABLE_NAME "opengl_hello")

project(${EXECUTABLE_NAME}
  VERSION 0.1.0
  LANGUAGES C CXX
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# GLFW as vendored subproject
add_subdirectory(third_party/glfw)

# GLAD
add_library(glad STATIC
  third_party/glad/src/gl.c
)

target_include_directories(glad
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/include"
)


# Main executable
add_executable(${EXECUTABLE_NAME}
  "src/${FILENAME}.cpp"
)

add_custom_command(
  TARGET ${EXECUTABLE_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/shaders"
)

target_link_libraries(${EXECUTABLE_NAME}
  PRIVATE
    glad
    glfw
    opengl32
)

target_compile_options(${EXECUTABLE_NAME}
  PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_options(${EXECUTABLE_NAME} PRIVATE -O0 -g3)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(${EXECUTABLE_NAME} PRIVATE -O3 -DNDEBUG)
endif()
