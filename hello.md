% Hello, World
% by Kevin Richey

# Outline

- Concurrency & Parallelism, multi-threading & forking
- Inter-Process Communication
- Security
	- Vulnerability prevention
	- Encryption
	- authentication
- Localization
- Storage (de/serialization)
- Documentation
- Code generation & meta-programming tools

# Software Engineering Quality

- Unit testing
- Signal handling
- Tracing & Logging

## Assertions

- check bool conditions
- on failure, condition is false:
	- collect error info: file, line #, expression, message
	- call assert handler
- configure assert handler on app init

## Error handling 

- Check for numeric overflow
- Status return codes
	- OK is 0
	- Errors are negative (error is less than ok)
	- Other status results are positive
	- Compare with enum value, not numeric constant

## Memory Integrity

- Check bad inputs (null/bad pointer, zero/negative size)
- Initialize memory with special value
- Over-allocate and check for overflows
- Track memory usage (pointer, size, file/line at allocation)
- init unused pointers to special value

# Basic Types 

- span - pointer & length
- range - start, stop, step
- string
- vector - fixed-length, named & random access
- array  - size, flex array member

## string

- compatible with any string
	- constant literal
	- char pointer
	- local array
	- dynamic allocation
	- compound literal array
- short string: strand? cord?
	- small fixed-length
	- can be passed/returned by value
	- don't store length

- copy:     create new copy of string
- concat:   create new string from two

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
- copy:      Create a duplicate object.

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

## Traversal

- begin:     Start itertion over range/sequence/container
- get:       Current element, fails if done.
- done:      position at the end, no more elements
- end:       position beyond last element 
- next:      advance n elements (default 1), fails if done.
- stop:      stop iteration, done is *true*

# Configuration

- Command line arguments
- Environment variables
- Config files

# Files

- Text Input & Parsing
- Structured data
- Tabular data
- Markup

