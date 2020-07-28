
CC = clang
CWARNFLAGS = -Wall -Wextra -Werror \
			 -Wno-missing-field-initializers \
			 -Wno-missing-braces
CFLAGS = -g -D DEBUG $(CWARNFLAGS)
#CFLAGS = -std=c11 -g -D DEBUG -I/mingw64/include/SDL2  -MMD $(CWARNFLAGS)
TESTCASES_C = $(wildcard test_*.c)
TESTCASES_O = $(TESTCASES_C:.c=.o)

run: test
	./test

test: test.o klib.o $(TESTCASES_O)

testcases.inc testcases.h:
	awk -f discover_tests.awk $(TESTCASES_C)

depfile.mk: testcases.h
	clang -MM *.c > depfile.mk

include depfile.mk

doc: doc.awk *.c
	awk -f doc.awk *.h > klib.md

clean:
	rm -f test *.o depfile.mk testcases.*

.PHONY: run clean

