
CC = tcc

CWARNFLAGS = -Wall \
			 -Wunsupported \
			 -Wwrite-strings \
			 -Wimplicit-function-declaration \
			 -Werror \
			 -Wdiscarded-qualifiers

CFLAGS = -std=c11 -g -b -bt8 -D DEBUG $(CWARNFLAGS)

CFILES = krbase.c 
HFILES = $(CFILES:.c=.h)
#UTESTS = $(wildcard test_*.c)
UTESTS = test_krbase.c

test: $(CFILES) $(HFILES) $(UTESTS) test.c testcases.h testcases.inc tags
	$(CC) $(CFLAGS) $(CFILES) $(UTESTS) test.c -run

tags: $(CFILES) $(HFILES) $(UTESTS) test.c
	ctags -R

maze: $(CFILES) $(HFILES) maze.c 
	$(CC) $(CFLAGS) $(CFILES) maze.c -o maze

testcases.inc testcases.h: discover_tests.awk $(UTESTS)
	awk -f discover_tests.awk $(UTESTS)

#doc: doc.awk *.c
#	awk -f doc.awk *.h > klib.md

clean:
	rm -f test maze testcases.*

.PHONY: run clean 

