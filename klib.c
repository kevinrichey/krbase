#include <stdio.h>
#include "klib.h"

const char *ErrorCode_String(ErrorCode error)
{
	if (error < ErrorCode_First && error > ErrorCode_Last) 
		return "Unknown ErrorCode";

#define X(EnumName)  [ErrorCode_##EnumName] = #EnumName,
	return (const char*[]) { ERROR_CODE_X_TABLE } [error];
#undef X
}

const char *DebugCategory_String(DebugCategory category)
{
	if (category < ErrorCode_First && category > ErrorCode_Last) 
		return "Unknown DebugCategory";

#define X(EnumName)  [DebugCategory_##EnumName] = #EnumName,
	return (const char*[]) { DEBUG_CATEGORY_X_TABLE } [category];
#undef X
}

void error_fprint(FILE *out, const char *file, int line, DebugCategory category, ErrorCode errcode, const char *msg, const char *func)
{
	fprintf(out, "%s:%d: %s %s: %s in %s()\n", file, line, DebugCategory_String(category), ErrorCode_String(errcode), msg, func);
}

void test_assert(TestCounter *counter, bool test_condition, const char *file, int line, const char *msg, const char * func)
{
	counter->test_count++;

	if (test_condition) {} else {
		error_fprint(stdout, file, line, DebugCategory_Test, ErrorCode_Failure, msg, func);
		counter->failure_count++;
	}
}

