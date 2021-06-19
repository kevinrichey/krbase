#include <stdlib.h>
#include "krclib.h"
#include "krstring.h"

int main(int argc, char *argv[])
{
	int lineno = 1, longest_no = 0;
	string *longest = NULL;

	string *s = NULL;
	while ((s = string_fgetline(stdin, s))) {
		if (string_length(s) > string_length(longest)) {
			string_swap(&longest, &s);
			longest_no = lineno;
		}
		++lineno;
	}

	if (longest)
		printf("%d: %s\n", longest_no, string_cstr(longest));

    return 0;
}


