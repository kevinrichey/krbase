% Hello, World
% by Kevin Richey

# Outline

- Concurrency & Parallelism, multi-threading & forking
- Networking, Inter-Process Communication
- Security
	- bounds checking
	- Vulnerability prevention
	- Encryption
	- authentication
- Localization
- Storage, de/serialization
- Documentation
- Code generation & meta-programming tools
- names?
	- Deep C, Dark C, Fathom the C, 
	- explore C, C explorer, 
	- C tools, C toolbox, C toolkit

## Things Every C Programmer Needs to Know

- Scope: function, block, file/translation unit, function proto
- Namespaces: tags, members, labels, identifiers
- Calling conventions & return values
- Stack & heap
- Storage class & duration
- Pointers
- Undefined behavior
- Preprocessor

# Build Modes

- Test
- Debug
- Release

enum & global constant for current build mode.

# Categories

- Error
- Assertion
- Debug
- Allocation
- Undefined Behavior
- Bad input
- Testing

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

# Error Handling 

- do nothing, ignore, off/disabled
- print/log a message
- pause, prompt user
- return error code to client
- set error state
- longjmp/exception
- halt program
- breakpoint
- raise signal

Unit Test Mode
: Print to stderr
: Propagate error up to test case
: Test failure if test case does not handle error
: Test passes if test case handles error

Debug Mode
: Print to stderr
: Break in debugger

Release Mode
: Print to log
: Inform user

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

## Error 

Information about specific error.

- Status code
- Message
- Debug info: file & line

Operations

- init
- is OK
- is error
- print error

## Diagnostics Module

- List of handler functions for each category
- Error stack
- tracing/logging output stream
- long jump on/off, location

Operations

- init
- set handler
- dispose
- has error
- push error
- pop error
- clear errors
- print error
- print error stack
- set jump
- set stream

## Assertions

- check bool conditions
- on failure, condition is false:
	- collect error info: file, line #, expression, message
	- call assert handler
- configure assert handler on app init

Categories

- precondition
- postcondition
- invariant

Failure Handling

- print/log
- ignore
- halt
- throw/long jump
- breakpoint

Return error code is not an option.

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

- span - pointer & length
- range - start, stop, step
- vector - fixed-length, named & random access
- array  - size, flex array member

## string

### String Requirements

- any storage method
- stores end pointer for faster concat & length
- can be null/empty
- function pass & return by value
- value member of arrays, lists, tables
- overflow protection & bounds checking
- minimize dynamic allocation

// int-aligned fixed-length string
union strand {
	int data[4];
	char characters[sizeof(int)*4];
};


### String Storage

- constant literal
- char pointer + length
- local array
- compound literal array
- internal short array
- heap

### String Composition

- formatting
- concat
- join
- template
- comprehension
- fill
- a-z range

### String Operations

- length
- not/equals, ignore case
- compare
- copy

### Substring Operations

- slice(a,b) - substring from positions a to b.
- left(n)  - slice 0 to n
- right(n)  - slice -n to -1
- contains(sub)  - has; true if string contains substring
- find(c|sub, iter) - search for position of character or sub-str
- split(sub)   - list of sub-strings delimited by sub
- iterate

### String Modifiers 

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
- count    - number of steps before stopping; 1/n/all

State

- position 
- done:      true if position equals stop, no more elements
- get:       Current element, fails if done.

Operations

- begin:     Start itertion 
- next(n)  - advance n steps (default 1), fails if done.

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

