#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define STRAND_DATA_TYPE  int
#define STRAND_DATA_SIZE  1
#define STRAND_CHAR_SIZE  (sizeof(STRAND_DATA_TYPE)*STRAND_DATA_SIZE)

union strand {
	STRAND_DATA_TYPE data[STRAND_DATA_SIZE];
	char characters[STRAND_CHAR_SIZE];
};

void exp_strand()
{
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
}

// thou shalt not pass NULL
int kr_reduce_i(int *start, int *stop, int (*f)(int,int))
{
	int result = 0;

	if (stop > start) {
		for (result = *start++; start < stop; ++start)
			result = f(result, *start);
	}

	return result;
}

int kr_reduce_in(int *start, int n, int (*f)(int,int))
{
	return (n) ?  kr_reduce_i(start, start+n, f) : 0;
}

int maxi(int a, int b) { return a > b ? a : b; }
int mini(int a, int b) { return a < b ? a : b; }
int sumi(int a, int b) { return a + b; }

int main(int argc, char *argv[])
{
	//exp_strand();

	int arr[] = { 1, 2, -10, 50, 3 };
	int *end = arr + 5;
	printf("Greatest: %d\n", kr_reduce_i(arr, end, maxi));
	printf("Least:    %d\n", kr_reduce_i(arr, end, mini));
	printf("Sum:      %d\n", kr_reduce_i(arr, end, sumi));

	int a = 1, b = 2, c = -3, d = 0, e = 100, f = 5;
	printf("Params max: %d\n", kr_reduce_in( (int[]){ a, b, c, d, e, f }, 6, maxi));

	return 0;
}

