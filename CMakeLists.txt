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

set(INCLUDES src src/contrib os)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

#-------------------------
# Compiler specific code
#------------------------

if(MSVC)
    include(cmake/msvc.cmake)
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    include(cmake/clang.cmake)
    include(cmake/common.cmake)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    include(cmake/gcc.cmake)
    include(cmake/common.cmake)
endif()

include_directories(${INCLUDES})

#-------------------------
# Determine compilation files
#------------------------

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
        os/*.h
        src/*.[ch])

if(WIN32)
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS os/win32/*.c)
elseif(CMAKE_SYSTEM_NAME MATCHES "BSD")
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS os/posix/*.c os/bsd/*.c)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB_RECURSE ADDITIONAL_FILES CONFIGURE_DEPENDS os/posix/*.c os/linux/*.c)
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(SRC_FILES "${SRC_FILES};${ADDITIONAL_FILES}")

#-------------------------
# Main build
#------------------------

# main executable
add_executable(${PROJECT_NAME} main/main.c ${SRC_FILES})

# protocol testing
add_executable(parrot-test EXCLUDE_FROM_ALL tests/parrot.c ${SRC_FILES})

# unit tests targets and their dependencies
add_executable(tests-infloop EXCLUDE_FROM_ALL tests/watchdog/infloop.c)
add_executable(tests-error EXCLUDE_FROM_ALL tests/watchdog/error.c)
add_executable(tests-nonrecoverable EXCLUDE_FROM_ALL tests/watchdog/nonrecoverable.c)
add_executable(${PROJECT_NAME}-tests EXCLUDE_FROM_ALL tests/tests.c ${SRC_FILES})
add_dependencies(${PROJECT_NAME}-tests tests-infloop tests-error tests-nonrecoverable)

# add special targets
set(SPECIAL_TARGETS ${PROJECT_NAME} ${PROJECT_NAME}-tests parrot-test)
include(cmake/special-targets.cmake)