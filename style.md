% Coding Style

## C Naming Conventions

Variables, parameters, struct & union members, enumerators
: Lower case with underscores: `variable_name`.

Types (struct, union, enum)
: Capialize each word: `UpperCamelCase`

Functions
: Lower case with underscores: `function_name()`

Preprocessor symbols
: Upper case with underscores: `PREPROC_SYMBOL`.

Preprocessor macros
: Upper case with underscores: `MACRO_CASE()`
: Parameters are upper case with trailing underscore: `VAR_`

Global Constants
: Upper case with underscores: `MODULE_CONSTANT_NAME`


Modules
: A collection of types and functions that work together. 
: Module names are UpperCamelCase and used as a namespace prefix in the collection of types and functions.
: Functions in a module are named as ModuleName_function_name().
: Enums in a module are named as ModuleName_EnumName. 
: Enum members are ModuleName_EnumName_MemberName.


### Common Meanings & Abbreviations

- Init:      Put object into to a valid usable starting state.
- Make:      Return initialized object by value.
- Create:    Return pointer to initialized object on heap.
- Destroy:   Clean up object and free memory.

- Length:  number of elements in a sequence (array, string, list, etc).
- Size:    number of bytes in an object, as in sizeof().
- Cap:     max number of elements a container may contain.

### Function and Type Suffixes

- i - int
- d - double
- c - char
- b - bool
- a - array
- n - number, length, count

### Matching Verb Pairs

- init, dispose
- start, stop
- begin, end
- put, get
- set, unset
- add, remove
- create, destroy
- enable, disable
- insert, delete
- load, save
- push, pop
- first, last


## Formatting

- Indent 4 spaces with tabs.
- Align with spaces (after indentation tabs).
- Function braces in column 0.
- Control statements (if, for, do, while, switch, etc)
	- Single space between keyword and open parens.
	- Single-statemets on next line, indented, no braces.
	- Block open brace on same line, once space after close parens.
	- Close brace on separate line, same column as keyword.


## C Guildelines & Tips

Preprocessor Guidelines
: - Wrap statement macros in `do{ ... }while(0)`.
: - Wrap macro parameters in parens.
: - Ensure variables declared within macros have unique names. Eg. use a prefix.
: - Use `_Bool` inside macros so user doesn't need to include stdbool.h.

Typedef Structs:
: `typedef struct Name_struct { ... } Name;`

Initialize empty struct objects:
: `struct_type var_name = { 0 };`

Prefer Designated Initializers for structs and arrays:
: `struct_type var_name = { .m1 = x, .m2 = y };`
: If the struct members are rearranged, the designated initializer still works regardless of order.

Goto Guidelines
: - `goto` is not evil.
: - Use `goto` to exit early from compound statement blocks. Target label shall be in outer scope below the block.
: - Jump to clean-up code for early exit from a function. Label for the clean-up code should be named "finally:".
: - Avoid jumping to labels above the goto or in inaccesible scopes.
: - Prefer using `break` from loops.

Special Enum Members
: - First: equal to first enum in range (usually 0).
: - Last:  equal to last enum in range.
: - End:   equal to last + 1.
: - Count: number of enums, usually equal to End-First, can be used to allocate an array.

Use enum values to name designated array Initializers
:   `type_name  array_name[] = { [enum_a] = value, [enum_b] = value };`
: So initializers do not depend on position.

- [X Macros](https://www.drdobbs.com/the-new-c-x-macros/184401387)
- [Overloading Functions in C](http://locklessinc.com/articles/overloading/)


