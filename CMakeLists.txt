#-------------------------
# General configuration
#------------------------

cmake_minimum_required(VERSION 3.16)
project(neblina
    VERSION 0.1
    DESCRIPTION "A tiny multi-protocol server"
    LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories("src", "src/contrib", "os")

#-------------------------
# Compiler specific code
#------------------------

if(MSVC)
    message("MSVC compiler detected")
    include(cmake/msvc.cmake)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    message("Clang compiler detected")
    include(cmake/common.cmake)
    include(cmake/clang.cmake)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    message("gcc compiler detected")
    include(cmake/common.cmake)
    include(cmake/gcc.cmake)
endif()

#-------------------------
# Determine compilation files
#------------------------

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        os/*.h
        src/*.[ch])

if(WIN32)
    message("Windows OS detected")
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS os/win32/*.c)
elseif(APPLE)
    message("Apple OS detected")
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS os/posix/*.c os/apple/*.c)
elseif(UNIX)
    message("Unix OS detected")
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS os/posix/*.c os/unix/*.c)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(SRC_FILES "${SRC_FILES};${ADDITIONAL_FILES}")
message("Source files: ${SRC_FILES}")

#-------------------------
# Main build
#------------------------

add_executable(${PROJECT_NAME} main/main.c ${SRC_FILES})
