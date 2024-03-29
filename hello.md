% Kevin Richey's C Library

Create a C library to make it easier, safer, more fun to code. 

# Issues & Todo

string_reserve() doesn't resize properly when bigger size is less than 2x current size.


# Outline

- Build Config
	- Build mode: test, debug, release
	- Versioning
- Instrumentation & Debugging
	- Assertions
	- Unit testing
	- Error handling
	- Exceptions
	- Tracing & logging
	- Memory debugging
- Signal handling
- Configuration
	- Command line args
	- Config files
	- Environment variables
- Documentation
- Concurrency & Parallelism
	- Forking
	- Event-driven FSM
- Networking, Inter-Process Communication
- Security
	- Encryption
	- Authentication & Authorization
- Localization
- Storage, de/serialization
- Code generation & meta-programming tools

## Things Every C Programmer Needs to Know

- Translation units
- Linkage: internal & external, static & external keywords
- Scope:  file/translation unit,function, block, function proto
- Storage class & duration
- Namespaces: tags, members, labels, identifiers
- Pointers
- Undefined behavior
- Preprocessor

Data structures

- Dynamic arrays
- stacks, queues
- tables (associative arrays)
- strings
- vector (tuple)
- chain (linked list)
- unit testing

Algos

- list comprehension
- collection operations
- random, shuffle, noise

Low-level concepts

- error handling
- assertions / contracts
- memory management
- preproc tools

Application Concepts

- tracing
- logging
- configuration

Error handling strategy

- low-level: return codes

# Guidelines

- Standard C17, avoid compiler extensions, ignore C++ compat
- Keep it C, don't try to imitate other languages
- Use "modern" C features
- Don't pre-over-optimize! (gets in the way, and you're not that good)

# Style Guide

## Naming Conventions

Variables, parameters, struct, union, enum, members
: Lower case with underscores: `lower_snake_case`.
: Use array notation (`type varname[]`) for passing arrays.

typedefs
: `PascalCase`

Constants, Enumerations, Preproc symbols
: Upper case with underscores.
: Enumerations prefixed by module or enum type name.

Functions
: Lower case with underscores: `function_name()`

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

## Function and Type Abbreviations

Functions that have multiple versions for different types use an abbreviated type prefix. 
For example: int_max() and fl_min().

- char - ch
- int - int
- double - fl
- bool - bool
- string - str
- array - arr
- pointer - p
- number/length/count - n
- file pointer - fp
- variable arguments - va
- function pointer - fp
- unsigned - prefix 'u', as in 'uint'


## Matching Pairs

- init, dispose - object value
- create, destroy - object lifetime
- start, stop
- begin, end - iteration
- enter, exit / leave
- acquire, release
- first, last - member/element in container
- front, back - ends of a span of elements/members
- head, tail
- get, put
- set, unset
- add, remove / subtract
- push, pop
- enqueue, dequeue
- insert, delete
- enable, disable
- load, save
- import, export
- open, close

See also [Max Truxa's Antonym List](https://gist.github.com/maxtruxa/b2ca551e42d3aead2b3d)

## Common Function Name Meanings

- initialize - put object into a known default usable state.


## Formatting

- Indent with tabs.
- Tab stop 4 columns.
- Align with spaces following tab indentation.
- Function, struct, union braces in column 0.

## Control Statement Formatting

if, for, do, while, switch, etc

- Single space between keyword and open parens.
- Single statements on next line, indented, no braces.
- Block open brace on same line, once space after close parens.
- Close brace on separate line, same column as keyword.

## C Idioms, Guildeline, Tips

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
: Typedef struct types: `typedef struct name { ... } name;`
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

Avoid Switch
: Prefer if-else-if over switch.
: Set -Wimplicit-fallthrough to warn about missing breaks.

Avoid Variable-Length Arrays
: Used fixed-length or malloc'd arrays.
: Set -Wvla option to detect VLAs.


# Build Modes

- Test - unit testing
- Debug - debugging
- Release - production

Enum, global constant, pre-proc symbols for current build mode.

Select build mode at compile time with define symbols.

# Unit testing

- Test case isolation
- Test discovery & execution
- Testing for data alignment
- Handle errors & assertions as test failures
- Fixture setup & tear down

## Test Output

Success or Failure

Test Failure
: `FILE.c:LINE: Test FUNCTION failed: CONDITION`

Statistics (# tests, # failures)

Process returns 0 on all success, non-zero on any failures.

### Components

- test_assert(condition, file, line, fmt, ...)
- test_assert_eq_T("a", a, "b", b, file, line, fmt, ...)
- test_failure(file, line, fmt, ...)

# Debug Categories

Types of errors.

- Input
	- Bad data
	- User input
	- Malicious
- System
	- Failed malloc
	- File open/acces
	- OS service failure
- Defects
	- Programmer error, "bugs"



- Terminal
- Assertion
	- Precondition
	- Postcondition
	- Invariant
- Failure
- Error
	- Item not found (in container)
- Warning
- Debug
	- Trace
	- Watch
	- Info
	- Scope begin/end
- Profile
    - Timer
    - Sample
    - Count
- Breakpoint
- Process Loop
	- Startup
	- Begin
	- Input
	- Process
	- Output
	- End
	- Shutdown
- Allocation
- Bad input
- Test

# Assertions

- Detect failed assertion
- Collect debug info: source line, assertion, message, error code
- Trace/log assertion failure details.
- Display user error message
- abruptly terminate program
- enable/disable asserts by level
- call custom assert handler

Other options

- trigger breakpoint in debugger
- test failure in unit test mode

## Assertion Names & Types

- require
- ensure
- expect
- check
- precondition
- prereq
- postcondition
- invariant
- given
- depend-on
- assume


# Error Handling 

Fault
: Root cause of the error. The bug or defect in code.

Error
: Deviation of program state from a known and correct state, caused by a Fault.

Failure
: Observably incorrect behavior or output caused by an Error.


- do nothing, ignore, off/disabled
- trace/log a message
- pause, prompt user
- return error code
- exception
- halt program
- breakpoint
- raise signal


## Status codes

- OK, no error
- Error, general
- Math overflow
- Array overflow
- Null pointer
- Bad pointer
- Allocation failure
- Not found
- Test case failure
- Assertion failure
- File/disk error
- Bad user input

Operations

- status to string

## Error Info

Information about specific error.

- Debug info: file & line
- Status code
- Message

# Memory Integrity

- Check bad inputs (null/bad pointer, zero/negative size)
- Initialize memory with special value
- Init pointers with special value
- Over-allocate and check for overflows
- Track memory usage (pointer, size, file/line at allocation)

# Tracing & Logging

- Logging is tracing to a file
- Delimited output for easy parsing
- Thread-safe

## What to Log

- Date/time stamp
- Category
- Volume level
- Thread ID
- Message

## How to Log

- Application Start-up & shut-down
- Config info (option name, value, source/origin)
- Begin/end main loop, transactions, events, requests, etc
- Errors, Assertion failures
- Debugging, watch variables

## Log Configuration

- Volume throttle (0 = always)
- Category on/off (null handler)
- Rotation, archiving, cleanup

# Basic Types 

- span - pair of pointers to front & back of elements
- range - start, stop, step
- vector - fixed-length, named & random access
- array  - size, flex array member

# string & strand

Requirements

- any storage method
- stores end pointer for faster concat & length
- can be null or empty
- value member of arrays, lists, tables
- overflow protection & bounds checking

## String Storage

- constant literal
- local array
- compound literal array
- char pointer + length
- heap alloc

## String Composition

- copy / dupe
- printf-style formatting
- concat / join
- template
- comprehension
- fill

## String Properties

- length: # chars in value
- size: # chars available for storage
- not/null: whether it has a value or not
- not/empty: whether the length > 0
- not/equals, ignore case
- compare

## Substring Operations

- slice(a,b) - substring from positions a to b.
- front(n)  - slice 0 to n
- back(n)  - slice -n to -1
- contains(sub)  - true if string contains substring
- find(c|sub, iter) - search for position of character or sub-str
- split(sub)   - list of sub-strings delimited by sub
- iterate

## String Modifiers 

Returns new string.

- lowercase
- uppercase
- capitalize(n)
- reverse
- trim whitespace 
- replace(char|substr, iter)
- delete(char|substr, iter)
- [Ordering]

# Sequences

Enumerated, ordered objects.

- counter - linear range of numbers
- random
- Fibonacci
- stream

# Containers

- list - resizeable array, random access
- table - associative array
- chain - binary linked list, 1-dimensional graph
- tree - branching acyclic graph
- graph - nodes & edges

# Grid

Generic dynamic 2D array of elements.

## Grid Cell

- row, column -  position of cell in grid
- state - OK, INVALID

## Grid operations

- create(r,c) - create grid with *r* rows and *c* columns.
- init(o) - initialized grid, set each element to *o*.
- nrows() - # of rows
- ncols() - # of columns
- cell(p) - pointer to cell at position *p*, null if out of bounds
- cell_try(p,x) - pointer to cell at *p*, exception if out of bounds
- begin() - iteration
- pending()
- row(r)  - return span of row *r*
- above(p) - return position in row above *p*
- below(p) - return position in row below *p*
- before(p) - return position in column left of *p*
- after(p)  - return position in column right of *p*
- includes(p)  - true if position is within the grid


# Standard Operations

## Object Existence

- create:    Bring new object into existence on heap.
- destroy:   Put an end to an object's existence.
- init:      Set existing object to a usable starting state.
- copy:      Create a duplicate object (deep copy).

## Dimensions

- length:    number of elements
- size:      max number of elements a container may hold.
- empty:     length is zero, no elements to get or remove
- nonempty:  length is at least 1
- full:      length equals size, cannot add more elements
- unfilled:  length is less than size, one/more elements can be added

## Sequences & Containers

- peek:     see next item without removing
- add:      new element in next available empty location
- get:      access element at key
- put:      replace element at key with new item
- insert:   add element at key, shift up
- delete:   remove element at key, shift down

- find:      search for element's key by value
- replace:   overwrite (first/some/all) elements by value
- remove:    delete (first/some/all) elements by value 

- first:     element at beginning
- last:      element before end
- slice:     sub-set of elements by begin & end keys

## Random access

- First index at 0. 
- Negative indices count from the end, with -1 being the last element.
- check(i) returns the absolute index. If *i* is negative, it is converted to the positive position from 0. If *i* is out of range, triggers an assertion.

## Ordering

- sort
- reverse
- shuffle

## Iterator

Properties

- start   -  Beginning position
- stop    -  Ending position, past last element 
- step     - direction & number

State

- position - curent index, ID, or location within the range
- complete - *position* equals *stop*, no remaining elements
- pending  - begun and not completed
- peek     - Current element, fails if done.

Operations

- begin()    - Start itertion from first element of the *container* or *sequence*.
- next()     - advance to next element, no-op if *complete*.
- end()      - move to *stop*

Common inits

- first(n):  start=0, step=1, count=n
- last(n):   start=-1, step=-1, count=n

## Collection Pipeline Operations

operations -> new list/array

- copy
- concat
- diff (new list = x : x not in [set])
- intersect (new list = x : x in [set])
- union (new list = x : x in [old] and x in [new])
- distinct/unique
- filter (new list = x : p(x) is true), aka select
- reject (new list = x : p(x) is false)
- group-by 
- map (new list = f(x), x in old)
- reduce
- slice
- sort(comp)

# Configuration

- Command line arguments
- Environment variables
- Config files
- Default values

# Files

- Text Input & Parsing
- Structured data
- Tabular data
- Markup

