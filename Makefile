
CC = clang
CWARNFLAGS = -Wall -Wextra \
			 -Werror=return-type \
			 -Wno-missing-field-initializers \
			 -Wno-missing-braces
CFLAGS = -g -D DEBUG $(CWARNFLAGS)
#CFLAGS = -std=c11 -g -D DEBUG -I/mingw64/include/SDL2  -MMD $(CWARNFLAGS)

run: all_tests.inc test
	./test

all_tests.inc:
	awk -f all_tests_awk test_*.c

test: test.c klib.c test_example.c

klib.c: klib.h

clean:
	rm -f test *.o all_tests.h all_tests.inc


#.PHONY: all clean run run-test
.PHONY: run 

