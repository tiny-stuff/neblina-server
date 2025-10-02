# standards
Programming standards that I follow

# C

- Reinvent the wheel or include contributes source files. Avoid using external libraries. Aim to generate a single executable - the smallest the better.
- Start with a complete set of warnings. Remove them if they create much trouble.
- Embed data files into the executable. Gzip them.
- Unit tests with TDD
- Isolate OS specific code in source files dedicated only for that. Use posix as default, and emulate posix calls on other OSes
- Subsystems should be as isolated as possible
- Use OO style when it make sense, avoid global unless is for fixed data.
- When user scripting is required, use Lua
- prefer using OO when it makes sense (multiple objects)
- hide structs internally
- _XOPEN_SOURCE = 700
- how to ensure compatibility with all OSes/compilers
  - use GNU make
  - distribute GNU make + required executables/DLLs in a separate folder for Windows
  - have a .sln file as well to help debugging in windows
- Error checking
  - Return >= 0 on success, -1 on failure
    - TODO - or maybe the opposite?
  - Use errno, create custom errnos per subsystem (>1000)
  - If a return value is not a positive value, return by pointer
  - Use macro to decide what to do with the errors
  - TODO - what about propagation

Recommended contributed code:
- JSON parsing: microJSON
- Gzipping: miniz

Make should have the following rules:
- all: generate the smallest, fastest executable possible
- dev: build fast (low warnings) with all debugging information
- warnings: run checks for all warnings in gcc and clang
- leaks: check for leaks (valgrind)
- threads: check for error in threads (drd or helgrind)
- check: run unit tests
- check-leaks, check-threads: run these tools on the unit tests
- bear: create a compile-commands.json

Warnings:
 - `-Wall -Wextra -Wpendantic -Wbad-function-cast -Wcast-align -Wcast-qual -Wconversion -Wfloat-equal -Wnull-dereference -Wshadow -Wstack-protected -Wstrict-prototypes -Wswtich-enum -Wundef -Wvla`
 - for gcc: `-Wduplicated-branches -Wduplicated-cond -Wformat-signedness -Wjump-misses-init -Wlogical-op -Wnested-externs -Wnormalized -Wold-style-definition -Wshift-negative-value -Wshift-overflow=2 -Wstrict-overflow=-3 -Wsuggest-attribute=printf -Wsuggest-attribute=malloc -Wtraditional-conversion -Wtrampolines -Wwrite-strings`
 - for clang: `-Warray-bounds-pointer-arithmetic -Wassign-enum -Wcast-function-type -Wcomma -Wcovered-switch-default -Wduplicate-enum -Widiomatic-parentheses -Wloop-analysis -Wformat-non-iso -Wformat-pedantic -Wformat-type-confusion -Wfour-char-constants  -Wimplicit-fallthrough -Wpointer-arith -Wpragmas-Wreserved-identifier -Wshift-sign-overflow -Wsigned-enum-bitfield -Wstatic-in-inline  -Wtautological-constant-in-range-compare  -Wthread-safety -Wunreachable-code -Wunreachable-code-aggressive -Wunused-macros -Wused-but-marked-unused -Wvariadic-macros -Wzero-as-null-pointer-constant`

Function naming:
 - Module_object_verb
 - Standard verbs:
   - create / destroy: allocate a new object

File naming:
 - module.h
 - module_priv.h
 - module_action.c
