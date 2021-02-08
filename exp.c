#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#define STRAND_DATA_TYPE  int
#define STRAND_DATA_SIZE  1
#define STRAND_CHAR_SIZE  (sizeof(STRAND_DATA_TYPE)*STRAND_DATA_SIZE)

union strand {
	STRAND_DATA_TYPE data[STRAND_DATA_SIZE];
	char characters[STRAND_CHAR_SIZE];
};

int main(int argc, char *argv[])
{
	union strand sd = {0};

	printf("size of strand: %d, stand.data: %d, strand.characters: %d\n", 
			sizeof(sd),     sizeof(sd.data), sizeof(sd.characters));
	char *s = sd.characters;
	strcpy(s, "Hello, world.");
	printf("strand is %s\n", s);

	int num = 11;
	int mod = num % sizeof(union strand);
	int size = mod ? num + sizeof(union strand) - mod : num;
	int num_strands = size / sizeof(union strand);

	union strand *line = malloc(size);
	memset(line, '*', size-1);
	line[3].characters[STRAND_CHAR_SIZE-1] = '\0';
	printf("num: %d, size: %d, num strands: %d\n", num, size, num_strands);

	char *front = line->characters;
	char *back  = front;
	char *stop  = front + size;

	int c;
	while ((c = getchar()) != EOF  &&  c != '\n'  &&  back < stop)
		*back++ = c;

	int length = back - front;
	printf("Line (%d): \"%.*s\"\n", length, length, line->characters);
	free(line);

	return 0;
}

