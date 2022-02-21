
CC = tcc

CWARNFLAGS = -Wall \
			 -Wunsupported \
			 -Wwrite-strings \
			 -Wimplicit-function-declaration \
			 -Wdiscarded-qualifiers

CFLAGS = -b -D DEBUG -bt8 -Werror $(CWARNFLAGS)

SOURCE = krstring.c krclib.c
HEADERS = $(SOURCE:.c=.h)

TESTCASES_C = $(wildcard test_*.c)
TESTCASES_O = $(TESTCASES_C:.c=.o)

run: run_test maze
	./maze $(ARGS)

run_test: test
	./test

test: test.c $(SOURCE) $(HEADERS) $(TESTCASES_C)

maze: maze.c krclib.c

testcases.h: discover_tests.awk $(TESTCASES_C)
	awk -f discover_tests.awk $(TESTCASES_C)

depfile.mk: $(SOURCE) $(HEADERS) $(TESTCASES_C) testcases.h
	$(CC) -MM -MF depfile.mk $(SOURCE) $(TESTCASES_C)

include depfile.mk

#doc: doc.awk *.c
#	awk -f doc.awk *.h > klib.md

clean:
	rm -f test *.o depfile.mk testcases.*

.PHONY: run clean 

