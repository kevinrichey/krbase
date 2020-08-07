----------------------------------------

# klib - Core Library

----------------------------------------

## Utility Macros

### UNUSED(var)

Suppress unused parameter warnings for variable *var*.

### STANDARD_ENUM_VALUES(EnumName_)

Insert the standard enumerators to an enum, prefixed with *EnumName_*:

- *EnumName_*First == 0.
- *EnumName_*Last == last enum value.
- *EnumName_*End == last enum value + 1.

----------------------------------------

## Debugging & Error Checking

### Status_String

	const char *Status_string(Status stat);

Convert *stat* code to string.

Returns
:  Pointer to static constant global string. Do not modify or free().


### Error_print

	void Error_print(FILE *out, ErrorInfo *e);

Print error to file stream *out*. 

----------------------------------------

## Unit Testing

### TEST_CASE(test_name_)

Define new test case *test_name_*. 

### test(condition_)

If boolean expression *condition_* is false, the test fails.

