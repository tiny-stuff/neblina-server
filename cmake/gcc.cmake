set(SPECIFIC_WARNINGS "-Wduplicated-branches -Wduplicated-cond -Wformat-signedness \
    -Wjump-misses-init -Wlogical-op -Wnested-externs -Wnormalized -Wshift-negative-value \
    -Wshift-overflow=2 -Wstrict-overflow=3 -Wsuggest-attribute=malloc -Wtrampolines \
    -Wwrite-strings -Wsuggest-attribute=pure -Wsuggest-attribute=const \
    -Wsuggest-attribute=format -Wsuggest-attribute=cold -Wsuggest-attribute=returns_nonnull")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fanalyzer")
endif()
