#include <stdlib.h>
#include "krclib.h"
#include "krstring.h"


#define KR_DEBUG_CAT_X_TABLE \
  X(ERROR) \
  X(ASSERT) \
  X(WATCH) \
  X(TRACE) \
  X(TEST)  \
  

#define X(EnumName_)  DEBUG_##EnumName_,
typedef enum {
	KR_DEBUG_CAT_X_TABLE 
    STANDARD_ENUM_VALUES(DEBUG)
} debug_category;
#undef X

const char *debug_category_str(debug_category cat)
{
	if (!in_bounds_enum(cat, DEBUG))
		return "Unknown debug_category";

#define X(EnumName)  [DEBUG_##EnumName] = #EnumName,
	return (const char*[]) { KR_DEBUG_CAT_X_TABLE } [cat];
#undef X
}



void debug_timestamp_gmt(FILE *out)
{
	char str[25]; // at least 24 chars for "YYYY-MM-DD HH.MM.SS TMZ" + null
	time_t now = time(NULL);
	strftime(str, sizeof(str), "%Y-%m-%d %H.%M.%S %Z", gmtime(&now));
	fprintf(out, "[%s] ", str);
}


void debug_print(FILE *out, debug_category cat, debug_info db, const char *message, ...)
{
	if (!out)  out = stderr;

	fprintf(out, "%s:%d: %s: ", db.file, db.line, debug_category_str(cat));
	if (message) {
		va_list args;
		va_start(args, message);
		vfprintf(out, message, args);
		va_end(args);
	}
	fputc('\n', out);
}


typedef int (*assert_handler)(debug_info, const char *);

int assert_nop(debug_info db, const char *s)
{
	UNUSED(db);
	UNUSED(s);
	return false;
}

int assert_exit(debug_info db, const char *s)
{
	debug_print(stderr, DEBUG_ASSERT, db, s);
	exit(EXIT_FAILURE);
	return false;
}

static assert_handler KR_ASSERT_HANDLER = assert_exit;

assert_handler set_assert_handler(assert_handler new_handler)
{
	assert_handler old_handler = KR_ASSERT_HANDLER;
	KR_ASSERT_HANDLER = new_handler;
	return old_handler;
}

void kr_assert_failure(debug_info source, const char *s)
{
	KR_ASSERT_HANDLER(source, s);
}

#define KR_ASSERT(CONDITION_) \
	do{ if (CONDITION_); else kr_assert_failure(SOURCE_HERE, #CONDITION_); } while(0)

#define KR_ASSERT_MSG(CONDITION_, MSG_) \
	do{ if (CONDITION_); else kr_assert_failure(SOURCE_HERE, #CONDITION_ ", " MSG_); } while(0)


int main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	printf("Hello, world\n");

	int length = 100;
	int *numbers = calloc(length, sizeof(*numbers));

	for (int i = 0; i < 200; ++i) {
		KR_ASSERT_MSG(0 <= i && i < 100, "index i out of range.");
		numbers[i] = i;
	}

    return 0;
}

