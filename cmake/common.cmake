set(WARNINGS "-Wall -Wextra -Wpedantic -Wbad-function-cast -Wcast-align -Wcast-qual \
    -Wconversion -Wfloat-equal -Wnull-dereference -Wshadow -Wstack-protector -Wswitch-enum \
    -Wundef -Wno-vla -Wno-sign-conversion -Wmissing-field-initializers ${SPECIFIC_WARNINGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNINGS}")

add_definitions(-D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb -fno-inline-functions -fstack-protector-strong -fno-common")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -ffast-math -march=native -flto -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fno-common")
    set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -flto=auto")
endif()