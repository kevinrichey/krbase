% Hello, World
% by Kevin Richey

# Outline

- Concurrency & Parallelism, multi-threading & forking
- Networking, Inter-Process Communication
- Security
	- Vulnerability prevention
	- Encryption
	- authentication
- Localization
- Storage, de/serialization
- Documentation
- Code generation & meta-programming tools
- names? Deep C, Dark C, Fathom the C

## What Every C Programmer Needs to Know

- Scope: function, block, file/translation unit, function proto
- Namespaces: tags, members, labels, identifiers
- Calling conventions & return values
- Stack & heap
- Storage class & duration
- Pointers
- Undefined behavior

# Software Engineering Quality

- Signal handling
- Tracing & Logging

## Unit testing

- Test case isolation
- Test discovery & execution
- Testing for data alignment
- Handle errors & assertions as test failures

## Error handling 

Failure
: Function is unable to complete succesfully. 

Status codes

- Math overflow
- Array overflow
- Null pointer
- Un-init pointer
- Out of memory
- Not found
- Test case failure
- Assertion failure

Error Reporting

- Status failure(err, code, debug_info, message)
	- Set status code
	- Collect error & debug info
	- Push error onto stack
	- Call handler
	- If handler returns, return status code
- raise(err, code, debug, message)  - propogate error up, adding debug info

Error Handler Module

- List handler functions for each error code
	- Possible outcomes: return code, terminate, long jump
- Error stack
- log handle
- long jump location

Error 

- Information about specific error occurrance 
	- Status code
	- Debug info: file & line
	- Message

## Assertions

- check bool conditions
- on failure, condition is false:
	- collect error info: file, line #, expression, message
	- call assert handler
- configure assert handler on app init

## Memory Integrity

- Check bad inputs (null/bad pointer, zero/negative size)
- Initialize memory with special value
- Init pointers with special value
- Over-allocate and check for overflows
- Track memory usage (pointer, size, file/line at allocation)

# Basic Types 

- span - pointer & length
- range - start, stop, step
- vector - fixed-length, named & random access
- array  - size, flex array member

## string

### String Requirements

- compatible with any storage method
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
- char pointer
- local array
- compound literal array
- internal short array
- dynamic allocation, must be freed

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
- not/equals
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

- size:      storage consumption (bytes) of sequence

## Object Existence

- create:    Bring new object into existence on heap.
- destroy:   Put an end to an object's existence.
- init:      Set existing object to a usable starting state.
- copy:      Create a duplicate object (deep copy).

## Dimensions

- length:    number of elements
- size:      max number of elements a container may hold.
- capacity:  maxiumum length of sequence
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
- next(n)  - advance n elements (default 1), fails if done.

- first(n):  start=0, step=1, count=n
- last(n):   start=-1, step=-1, count=n

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

