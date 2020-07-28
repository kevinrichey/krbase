----------------------------------------

# klib - Core Library

----------------------------------------

## Utility Macros

### `UNUSED(var)`

Suppress "unused parameter" warnings for variable *var*.

### STANDARD_ENUM_VALUES(EnumName_)
Add useful enum values, prefixed with *EnumName_*:

- *EnumName_*First - equals 0.
- *EnumName_*Last - equals last enum value.
- *EnumName_*End - equals last enum value + 1.

----------------------------------------

## Debugging & Error Checking

### `const char *ErrorCode_String(ErrorCode error);`

Return string rep of *error* code.

### `const char *DebugCategory_String(DebugCategory category);`

Return string rep of *category*.

### `void error_fprint(FILE *out, const char *file, int line, DebugCategory category, ErrorCode errcode, const char *msg, const char *func);`

Print error info to file stream *out*. 

----------------------------------------

## Unit Testing

### `TEST_CASE(test_name_)`

Define new test case *test_name_*. 

### `test(condition_)`

Test that the *condition_* is true. If it is false, the test fails.

