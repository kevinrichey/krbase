#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

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


typedef struct { int a, b; } pair;

#define pack2(A_, B_)  (pair){ .a=(A_), .b=(B_) }

#define unpack2(A_, B_, P_)  do{ \
	pair pair_temp = P_; \
	A_ = pair_temp.a;    \
	B_ = pair_temp.b;    \
}while(0)

pair get_pair()
{
	return pack2(10, 11);
}

typedef struct { bool ok; int result; }  ok_result;

ok_result  maybe_get_number(int x)
{
	if (x > 0)
		return (ok_result){ .ok=true, .result=100 };
	else
		return (ok_result){ .ok=false };
}

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

    int x = 0, y = 0;
	unpack2(x, y,  get_pair());
    printf("pair.a = %d, pair.b = %d\n", x, y);

#define maybe(R_, N_, FN_)  \
	( ((R_) = (FN_)), (R_).ok ? (N_) = (R_).result : 0, (R_).ok)

	ok_result res;
	if (maybe(res, x, maybe_get_number(-11)))
//	if ( (res = maybe_get_number(1)), res.ok ? x = res.result : 0, res.ok )
		printf("maybe_get_number returned %d\n", x); //res.result);
	else
		printf("maybe_get_number returned not ok\n");
	
	return 0;
}

