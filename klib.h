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


//@module list - Dynamic Resizeable Arrays

typedef struct { int size, length; } list_header;

#define list(EL_TYPE)  struct { list_header head; EL_TYPE begin[]; }
/*@doc list(type) 
 Declare a new list of *type* objects.

 Example - declare list if int
 : `list(int) *numbers = NULL;`

 Example - define new list of doubles type
 : `typedef list(double)  list_doubles;`
*/

void *list_resize_f(list_header *a, int sizeof_base, int sizeof_item, int capacity);

#define list_resize(L_, NEW_SIZE)   do{ (L_) = list_resize_f((list_header*)(L_), sizeof(*(L_)), sizeof(*(L_)->begin), (NEW_SIZE)); }while(0)
/*@doc list_resize(list, s)
 Resize *list* to store at least *s* elements.

 Side Effects
 : Pointer *list* may be modified. 
*/

static inline int list_size(void *list)
/*@func list_size(list)
 @returns int size (max capacity) of *list*.
*/
{
	return list? ((list_header*)list)->size: 0;
}

static inline int list_length(void *list)
/*@func list_length(list)
 @returns int number of elements in *list*.
*/
{
	return list? ((list_header*)list)->length: 0;
}

static inline bool list_is_full(void *list)
/*@func list_is_full(list)
 @returns *true* if *list* is full (cannot add more elemnets).
*/
{
	// assert list -> actual list
	return list_length(list) >= list_size(list);
}

static inline bool list_is_empty(void *list)
/*@func list_is_empty(list)
 @returns *true* if *list* is empty (no elements in list)
*/
{
	// assert list is actual list
	return list_length(list) == 0;
}

void list_dispose(void *list);
/*@func list_dispose(list)
 Free *list* when it's not longer used.
*/

