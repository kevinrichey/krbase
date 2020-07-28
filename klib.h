#include <stdbool.h>
#include <stdio.h>

//@library klib - Core Library

//@module Utility Macros

#define UNUSED(var)  (void)(var)
/*@doc UNUSED(var)
Suppress unused parameter warnings for variable *var*.
*/

#define STANDARD_ENUM_VALUES(EnumName_) \
  EnumName_##_End,  \
  EnumName_##_Last  = EnumName_##_End - 1, \
  EnumName_##_First = 0,
/*@doc STANDARD_ENUM_VALUES(EnumName_)
  Insert the standard enumerators to an enum, prefixed with *EnumName_*:

  - *EnumName_*First == 0.
  - *EnumName_*Last == last enum value.
  - *EnumName_*End == last enum value + 1.
*/


//@module Debugging & Error Checking

#define STATUS_X_TABLE \
  X(OK) \
  X(Test_Failure) \
  X(Error) 

#define X(EnumName_)  Status_##EnumName_,
typedef enum {
    STATUS_X_TABLE
    STANDARD_ENUM_VALUES(Status)
} Status;
#undef X

const char *Status_string(Status stat);
/*@func Status_String
  Convert *stat* code to string.
  @returns Pointer to static constant global string. Do not modify or free().
*/


typedef struct Error_struct {
	Status         stat;
	const char    *filename;
	int            fileline;
	const char    *funcname;
	const char    *message;
} ErrorInfo;

void Error_print(FILE *out, ErrorInfo *e);
/*@func Error_print
  Print error to file stream *out*. 
*/

//@module Unit Testing

typedef struct TestCounter_struct {
	int test_count, failure_count;
	const char *test_name;
} TestCounter;

typedef void (*TestCase_fn)(TestCounter*);

void Test_assert(TestCounter *counter, bool test_condition, const char *file, int line, const char *msg);

#define TEST_CASE(test_name_)  void TestCase_##test_name_(TestCounter *test_counter)
/*@doc TEST_CASE(test_name_)
  Define new test case *test_name_*. 
*/

#define TEST(condition_)  Test_assert(test_counter, (condition_), __FILE__, __LINE__, #condition_)

/*@doc test(condition_)
  If boolean expression *condition_* is false, the test fails.
*/


