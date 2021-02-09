% My C Coding Style

# Naming Conventions

Variables, parameters, struct & union members
: Lower case with underscores: `variable_name`.

Types (struct, union, enum, typedef)
: Capialized: `UpperCamelCase`

Enumerations
: Enum type name: `UpperCamelCase`.
: Enumerators: lowercase with underscores, prefixed by enum type. `EnumName_enumerator`

Functions
: Lower case with underscores: `function_name()`

Preprocessor symbols
: Upper case with underscores: `PREPROC_SYMBOL`.

Preprocessor macros
: Upper case with underscores: `MACRO_CASE()`
: Macros made to look like functions may be `lowercase_underscore()`
: Parameters are upper case with trailing underscore: `VAR_`

Global Constants
: Upper case with underscores: `MODULE_CONSTANT_NAME`

Modules
: Types and functions that work together. 
: Module names are UpperCamelCase and used as a namespace prefix for the types and functions.
: Functions in a module are named as `ModuleName_function_name()`.
: Module enums are named as `ModuleName_EnumName`.
: Enum members are `ModuleName_EnumName_MemberName`.

## Function and Type Suffixes

- i - int
- d - double
- c - char
- b - bool
- a - array
- n - number, length, count
- s - string

## Matching Verb Pairs

- init, dispose
- create, destroy
- start, stop
- begin, end
- first, last
- front, back
- head, tail
- put, get
- set, unset
- add, remove
- push, pop
- insert, delete
- enable, disable
- load, save

# Formatting

- Indent with tabs.
- Tab stop 4 spaces.
- Align with spaces (after indentation tabs).
- Function braces in column 0.

## Control Statement Formatting

if, for, do, while, switch, etc

- Single space between keyword and open parens.
- Single statements on next line, indented, no braces.
- Block open brace on same line, once space after close parens.
- Close brace on separate line, same column as keyword.

# C Idioms, Guildeline, Tips

Prefer basic C primitives
: int, double, char, bool
: Use unsigned only for overflow & bit operations

Preprocessor Macros
: - Wrap statement macros in `do{ ... }while(0)`.
: - Wrap macro parameters in parens.
: - Ensure variables declared within macros have unique names. Eg. use a prefix.
: - Use `_Bool` inside macros so user doesn't need to include stdbool.h.
: - [X Macros](https://www.drdobbs.com/the-new-c-x-macros/184401387)
: - [Overloading Functions in C](http://locklessinc.com/articles/overloading/)

Structs
: Typedef struct types: `typedef struct Name_struct { ... } Name;`
: Initialize struct objects: `struct_type var_name = { 0 };`
: Use designated initializers: `struct_type var_name = { .m1 = x, .m2 = y };`

Goto 
: - `goto` is not evil.
: - Use `goto` to exit early from compound statement blocks. Target label shall be in outer scope below the block.
: - Jump to clean-up code for early exit from a function. Label for the clean-up code should be named "finally:".
: - Avoid jumping to labels above the goto or in inaccesible scopes.
: - Prefer using `break` from loops.

Enumerations
: Special Enum Members:
:  - First: equal to first enum in range (usually 0).
:  - Last:  equal to last enum in range.
:  - End:   equal to last + 1.
:  - Count: number of enums, usually equal to End-First, can be used to allocate an array.

Arrays
: Use enum values for designated array Initializers
:   `type_name  array_name[] = { [ENUM_A] = a, [ENUM_B] = b };`

Goes-To Operator
: `while (x --> 0)`
: Combination of unary postfix decrment and greater-than: -- >
: Compares x to 0 and then decrements it. Iteration stops when x equals 0.

Bang Bang Operator
: `x += !!length_of_thing()`
: Two logical not operators. Forces an integer into zero or one.
: If an expression is any non-zero value, !! returns 1, otherwise 0.
: This example increments x iif `length_of_thing()` returns non-zero.

Push Onto Pointer
: `*p++ = value`
: Pointer de-reference assignment and pointer unary post increment.
: Precondition: *p* points to element in array[n] where n < array length.
: If *p* is pointer to the first unused element of an array, 
: assign *value* to the element 
: advance the pointer to the next unused element.

Pop Off Pointer
: `x = *--p`
: Pointer unary prefix decrement and assignment to pointer dereference.
: Precondition: *p* must point to element array[n] where n > 0 and n <= array length.
: If *p* is pointer to the first unused element of an array,
: and the previous element contains a value,
: move pointer back to the last used element,
: assign the element value to x.



