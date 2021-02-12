#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "klib.h"

//-----------------------------------------------------------------------------
// Error Handling
//

static void source_info_test_fn(TestCounter *test_counter, int line, SourceInfo source)
{
	TEST(!strcmp(source.file, "test_dev.c"));
	TEST(source.line == line);
}

TEST_CASE(pass_source_info_parameter)
{
	int line = __LINE__ + 1;
	source_info_test_fn(test_counter, line, SOURCE_HERE);
}



struct Error_module_handle;
typedef StatusCode (*ErrorHandler)(struct Error_module_handle *errh);

typedef struct Error_module_handle {
	ErrorInfo error;
	ErrorHandler handlers[Status_End];
} ErrHand;

bool in_bounds(int n, int lower, int upper)
{
	return lower <= n && n <= upper;
}

#define in_enum_bounds(VAL_, ENUM_) \
	in_bounds((VAL_), (ENUM_##_First), (ENUM_##_Last))

#define in_array_bounds(I_, ARR_) \
	in_bounds((I_), 0, ARRAY_SIZE(ARR_))

StatusCode Error_failure(ErrHand *errh, StatusCode code, const char *message, SourceInfo source)
{
	errh->error = (ErrorInfo){ 
		.status  = code,
		.source  = source,
		.message = message,
	};

	if (in_array_bounds(code, errh->handlers)) {
		ErrorHandler handler = errh->handlers[code];
		if (handler)
			code = handler(errh);
	}

	return code;
}

bool test_error_handler_called = false;

static StatusCode test_error_handler(ErrHand *errh)
{
	test_error_handler_called = true;
	return errh->error.status;
}

TEST_CASE(function_fails_call_error_handler)
{
	// Given an error handler
	ErrHand errh = {0};
	errh.handlers[Status_Error] = test_error_handler;

	// When a function fails
	int lineno = __LINE__ + 1;
	StatusCode stat = Error_failure(&errh, Status_Error, "Error message here", SOURCE_HERE);

	// Then error status is set and the handler was called
	TEST(stat == Status_Error);
	TEST(errh.error.status == Status_Error);
	TEST(!strcmp(errh.error.source.file, "test_dev.c"));
	TEST(errh.error.source.line == lineno);
	TEST(!strcmp(errh.error.message,  "Error message here"));
	TEST(test_error_handler_called);
}

void Error_clear(ErrHand *errh)
{
	errh->error = (ErrorInfo){0};
}

TEST_CASE(clear_error_status)
{
	// Given an error handler with an error status
	ErrHand errh = {0};
	Error_failure(&errh, Status_Error, "Testing clear error", SOURCE_HERE);
	TEST(errh.error.status == Status_Error);

	// Clear the error
	Error_clear(&errh);

	TEST(errh.error.status == Status_OK);
	TEST(errh.error.source.file == NULL);
	TEST(errh.error.source.line == 0);
	TEST(errh.error.message == NULL);
}
