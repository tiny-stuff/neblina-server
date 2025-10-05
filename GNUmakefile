PROJECT=neblina

all: ${PROJECT} ${PROJECT}-test

#
# objects
#

ifeq ($(OS),Windows_NT)
  OS=win32
  OS_GENERIC=win32
else
  OS_GENERIC=posix
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
    OS=apple
  else
    OS=unix
  endif
endif

include objects.mk

#
# warnings
# 

WARNINGS=-Wall -Wextra -Wpedantic -Wbad-function-cast -Wcast-align -Wcast-qual -Wconversion -Wfloat-equal -Wnull-dereference -Wshadow -Wstack-protector -Wswitch-enum -Wundef -Wno-vla -Wno-sign-conversion -Wmissing-field-initializers
WARNINGS_GCC=-Wduplicated-branches -Wduplicated-cond -Wformat-signedness -Wjump-misses-init -Wlogical-op -Wnested-externs -Wnormalized -Wshift-negative-value -Wshift-overflow=2 -Wstrict-overflow=3 -Wsuggest-attribute=malloc -Wtrampolines -Wwrite-strings -Wsuggest-attribute=pure -Wsuggest-attribute=const -Wsuggest-attribute=format -Wsuggest-attribute=cold -Wsuggest-attribute=returns_nonnull
WARNINGS_CLANG=-Warray-bounds-pointer-arithmetic -Wassign-enum -Wcast-function-type -Wcomma -Wcovered-switch-default -Wduplicate-enum -Widiomatic-parentheses -Wloop-analysis -Wformat-non-iso -Wformat-pedantic -Wformat-type-confusion -Wfour-char-constants  -Wimplicit-fallthrough -Wpointer-arith -Wreserved-identifier -Wshift-sign-overflow -Wsigned-enum-bitfield -Wstatic-in-inline  -Wtautological-constant-in-range-compare  -Wthread-safety -Wunreachable-code -Wunreachable-code-aggressive -Wunused-macros -Wused-but-marked-unused -Wvariadic-macros -Wzero-as-null-pointer-constant -Wno-strict-prototypes -Wno-newline-eof

COMPILER_ID := $(shell $(CC) -v 2>&1 | grep clang)
ifeq ($(findstring clang,$(COMPILER_ID)),clang)
  WARNINGS += $(WARNINGS_CLANG)
else
  WARNINGS += $(WARNINGS_GCC)
endif

#
# flags
#

INCLUDES=-I. -Isrc -Isrc/contrib

CFLAGS=-std=c17 -D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE -MMD $(INCLUDES)

ifdef DEV
  CFLAGS += -O0 -ggdb -fno-inline-functions -fstack-protector-strong -fno-common $(WARNINGS)
  ifeq ($(CC),g++)
    CFLAGS += -fanalyzer
  endif
else
  CFLAGS += -Os -ffast-math -march=native -flto -D_FORTIFY_SOURCE=2 -fstack-protector-strong -fno-common
  LDFLAGS += -flto=auto
endif

CFLAGS_CONTRIB = -I. -O3 -ffast-math -march=native -flto -Wno-switch

#
# main executable targets
#

$(PROJECT): src/main.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)
ifndef DEV
	strip ./$@
endif
ifeq ($(UNAME_S),Linux)
	sudo setcap cap_net_bind_service=ep ./$@
endif

#
# development targets
#

leaks: dev
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=valgrind.supp ./$(PROJECT)

thread: dev
	valgrind --tool=drd ./$(PROJECT)

dev:
	$(MAKE) all DEV=1

bear:
	make clean
	bear -- make

clean:
	find . -type f -name '*.[od]' -exec rm {} +
	rm -f $(PROJECT) $(PROJECT)-test tests/error tests/nonrecoverable tests/infloop tests/parrot-test

install: $(PROJECT)
	install $(PROJECT) /usr/local/bin/$(PROJECT)

uninstall:
	rm /usr/local/bin/$(PROJECT)

#
# test targets
#

tests/parrot-test: CFLAGS=-D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE $(INCLUDES) -I../src -O0 -ggdb -fno-inline-functions -fstack-protector-strong -fno-common $(WARNINGS)
tests/parrot-test: tests/parrot.o $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

parrot-test-leaks: tests/parrot-test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=valgrind.supp ./tests/parrot-test

$(PROJECT)-test: CFLAGS=-D_XOPEN_SOURCE=700 -D_POSIX_C_SOURCE=200112L -D_DEFAULT_SOURCE $(INCLUDES) -I../src -O0 -ggdb -fno-inline-functions -fstack-protector-strong -fno-common $(WARNINGS)
$(PROJECT)-test: tests/tests.o $(OBJ) tests/error tests/nonrecoverable tests/infloop tests/parrot-test
	$(CC) -o $@ tests/tests.o $(OBJ) $(LDFLAGS)

tests/infloop: tests/watchdog/infloop.o
	$(CC) -o $@ $^

tests/error: tests/watchdog/error.o
	$(CC) -o $@ $^

tests/nonrecoverable: tests/watchdog/nonrecoverable.o
	$(CC) -o $@ $^

check: $(PROJECT)-test
	./$(PROJECT)-test

leaks-check: $(PROJECT)-test
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --suppressions=valgrind.supp ./$(PROJECT)-test

thread-check: $(PROJECT)-test
	valgrind --tool=drd ./$(PROJECT)-test


# include dependencies
-include $(OBJ:.o=.d)

# vim:sts=8:ts=8:sw=8:noexpandtab
