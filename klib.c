#include <stdlib.h>
#include <stdio.h>
#include "klib.h"

const char *Status_string(Status stat)
{
	if (stat < Status_First && stat > Status_Last) 
		return "Unknown Status";

#define X(EnumName)  [Status_##EnumName] = #EnumName,
	return (const char*[]) { STATUS_X_TABLE } [stat];
#undef X
}


void Error_printf(FILE *out, ErrorInfo *e, const char *fmt, ...)
{
	fprintf(out, "%s:%d: %s: %s", 
	        e->filename, 
			e->fileline, 
			Status_string(e->stat), 
			e->message);

	if (fmt && fmt[0]) {
		va_list args;
		va_start(args, fmt);
		vfprintf(out, fmt, args);
		va_end(args);
	}

	putc('\n', out);
}

void Error_print(FILE *out, ErrorInfo *e)
{
	Error_printf(out, e, "");
}

void Test_assert(TestCounter *counter, bool test_condition, const char *file, int line, const char *msg)
{
	++counter->test_count;

	if (!test_condition) {
		ErrorInfo err = {
			.stat = Status_Test_Failure,
			.filename = file,
			.fileline = line,
			.funcname = NULL,
			.message  = msg,
		};
		Error_printf(stdout, &err, " in test %s()", counter->test_name);
		counter->failure_count++;
	}
}



void *list_resize_f(list_header *a, int sizeof_base, int sizeof_item, int capacity)
{
	list_header *b = a;
	if ((b = realloc(a, sizeof_base + sizeof_item * capacity))) {
		b->size = capacity;
		if (!a)
			b->length = 0;
		if (b->length > b->size)
			b->length = b->size;
	}
	return b; 
}

void list_dispose(void *list)
{
	free(list);
}

