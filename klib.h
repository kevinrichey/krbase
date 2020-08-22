#include <stdbool.h>
#include <stdio.h>

//@library klib - Core Library

//@module Utility Macros

#define UNUSED(VAR_)  (void)(VAR_)
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

#define STRINGIFY(x)            #x
#define STRINGIFY_EXPAND(x)     STRINGIFY(x)

#define LINE_STR                EXPAND_STRINGIFY(__LINE__)
#define SOURCE_LINE_STR         __FILE__ ":" LINE_STR ":"

#define VA_NARGS_N(P0, P1, P2, P3, P4, P5, P6, P7, P8, P9, PA, PB, PC, PD, PE, PF, PN, ...) PN
#define VA_NARGS(...) VA_NARGS_N(__VA_ARGS__, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)
/*@doc VA_NARGS(...)
 @returns number of arguments passed.
*/

#define ARRAY_LENGTH(A_)  (sizeof(A_) / sizeof(*A_))
/*@doc ARRAY_LENGTH(a)
 @returns length of statically sized array *a*.  
 Undefined behavior if *a* is dynamically allocated array.
*/

typedef unsigned char byte;


//@module Debugging & Error Checking

#define STATUS_X_TABLE \
  X(OK) \
  X(Test_Failure) \
  X(Error)  \
  X(Alloc_Error) \

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
/*@doc TEST(condition_)
  If boolean expression *condition_* is false, the test fails.
*/

//@module vector - tuple with named and random access

#define vector(TYPE_, ...)   \
  union { \
      struct { TYPE_ __VA_ARGS__; }; \
      TYPE_ at[VA_NARGS(__VA_ARGS__)]; \
  }
/*@doc vector(TYPE, members...)
 Declare a new vector of *TYPE* with *members*.
 Access elements through the named members or by random access
 through the *at[]* array member.

 Example - declare an x,y point
 : `vector(int, x, y)  point;`
 : `point.x = 100;`
 : `point.at[1] = 200;   // at[1] is random access to member y`

 Example - declare an RGB color type
 : `typedef vector(unsigned, red, green, blue)  rgb;`
 */

#define vec_length(V_)    (int)(ARRAY_LENGTH((V_).at))
/*@doc vec_length(v)
 @returns length (number of members) in vector *v*.
*/

//@module list - Dynamic Resizeable Arrays

typedef struct { int cap, length; } list_header;

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
/*@doc list_resize(l, s)
 Resize list *l* to store at least *s* elements.

 Side Effects
 : Pointer *l* may be modified. 
*/

static inline int list_capacity(void *l)
/*@func list_capacity(l)
 @returns int max capacity of list *l*; 0 if *l* is NULL.
*/
{
	return l? ((list_header*)l)->cap: 0;
}

static inline int list_length(void *l)
/*@func list_length(l)
 @returns int, number of elements in list *l*; 0 if *l* is NULL.
*/
{
	return l? ((list_header*)l)->length: 0;
}

static inline bool list_is_full(void *l)
/*@func list_is_full(l)
 @returns *true* if list *l* is cannot hold more elements; *true* if *l* is NULL.
*/
{
	// assert list -> actual list
	return list_length(l) >= list_capacity(l);
}

static inline bool list_is_empty(void *l)
/*@func list_is_empty(l)
 @returns *true* if list *l* is contains no elements; *true* if *l* is NULL.
*/
{
	// assert list is actual list
	return list_length(l) == 0;
}

void list_dispose(void *l);
/*@func list_dispose(list)
 Free *list* when it's not longer used.
*/


typedef void (*closure_fn)(void*, void*);

void sum_ints(void *total, void *next_i);


//@module Binode - Double linked list


typedef struct Binode_struct {
	struct Binode_struct *right, *left;
} Binode;


void *Binode_next(Binode *n);
void *Binode_prev(Binode *b);
_Bool Binode_is_linked(Binode *n);
_Bool Binode_not_linked(Binode *n);
_Bool Binodes_are_linked(Binode *a, Binode *b);

void Binode_link(Binode *a, Binode *b);
void Binode_insert(Binode *l, Binode *n);
void Binode_remove(Binode *n);
void *Binode_foreach(Binode *node, closure_fn fn, void *closure, int offset);

typedef struct {
	Binode *head, *tail;
} Chain;

Chain Binode_chain_va(void *first, ...);

#define BINODE_CHAIN(...)   Binode_chain_va(__VA_ARGS__, NULL)


