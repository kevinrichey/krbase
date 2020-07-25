
CC = clang
CWARNFLAGS = -Wall -Wextra -Werror \
			 -Wno-missing-field-initializers \
			 -Wno-missing-braces
CFLAGS = -g -D DEBUG $(CWARNFLAGS)
#CFLAGS = -std=c11 -g -D DEBUG -I/mingw64/include/SDL2  -MMD $(CWARNFLAGS)

run: test
	./test

test: test.o klib.o test_*.o

testcases.inc testcases.h:
	awk -f discover_tests.awk test_*.c

depfile.mk: testcases.inc
	clang -MM *.c > depfile.mk

include depfile.mk

clean:
	rm -f test *.o depfile.mk testcases.*

.PHONY: run clean

