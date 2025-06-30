% Kevin Richey's C Library

C base library to make it easier, safer, more fun to code. 

# Todo

- enums (first, last, count, to-string).
- hash
- random generator, shuffle 
- strand (string begin/end pointer range)
- functors/monads (option/maybe/nullible, any, either/status/error)
- closures & partial application
- status codes
- exceptions
- arrays (fat pointer, index from end, checked accessors) 
- memory arena allocator
- memory error handler & debugger (debug tracking, leak checker, overflow checker)
- math overflow handling

- Dynamic array
- Table (associative array)

- Versioning
- Signal handling
- Configuration
	- Command-line args
	- Config files
	- Environment variables
- Docstrings, Docblocks 
- Storage, de/serialization
- Code generation & meta-programming tools
- Security, Encryption, Authentication & Authorization
- Localization

# Style Guide
## Laws of Programming

- "For every complex problem there is an answer that is clear, simple, and wrong." - H. L. Mencken
- Every program has at least one bug, and at least one line of unnecessary code. 
- [90-90 Rule](https://en.wikipedia.org/wiki/Ninety%E2%80%93ninety_rule): "The first 90 percent of the code accounts for the first 90 percent of the development time. The remaining 10 percent of the code accounts for the other 90 percent of the development time." - Tom Cargill, Bell Labs
- [Conway's Law](https://en.wikipedia.org/wiki/Conway's_law): "organizations which design systems (in the broad sense used here) are constrained to produce designs which are copies of the communication structures of these organizations." - Melvin E. Conway, How Do Committees Invent? (1968)
- [Wirth's Law](https://en.wikipe(dia.org/wiki/Wirth's_law): Software is getting slower more rapidly than hardware is becoming faster.  - Niklaus Wirth, his "A Plea for Lean Software". (1995) 
- [Knuth's optimization principle](https://en.wikipedia.org/wiki/Program_optimization#When_to_optimize): "We should forget about small efficiencies, say about 97% of the time: premature optimization is the root of all evil. Yet we should not pass up our opportunities in that critical 3%" - Donald Knuth, "Structured Programming with go to Statements". ACM Computing Surveys. (1974)
- Kernighan's Law: "Everyone knows that debugging is twice as hard as writing a program in the first place. So if you’re as clever as you can be when you write it, how will you ever debug it?" - Brian Kernighan, The Elements of Programming Style, 2nd Edition 2nd Edition (1974)
- [Greenspun's tenth rule of programming](https://en.wikipedia.org/wiki/Greenspun%27s_tenth_rule): Any sufficiently complicated C or Fortran program contains an ad hoc, informally-specified, bug-ridden, slow implementation of half of Common Lisp.


## Principals

- Keep it C: avoid compiler extensions, don't add sugar.
- Clarity first. Don't pre-optimize. 
- Prefer basic C primitives: int, double, char, bool. 
- Use unsigned only for modulus arithmetic and bit operations. 
- Use signed size and index types. 

## C Guidelines & Idioms

- Wrap statement macros in `do{ ... }while(0)`.
- Wrap macro parameters in parens.
- Postfix macro params with an underscore to avoid name collisions. 
- Initialize struct objects with `= { 0 };`
- Use struct designated initializers: `{ .m1 = x, .m2 = y };`
- End all enumeration lists with a COUNT. 
- Use enumerations for array designated initializers: `{ [ENUM_A] = a, [ENUM_B] = b };`
- Goes-To Operator: `while (x --> 0)`, iteration stops when x equals 0.
- Bang Bang Operator: `x += !!length()`, two logical not operators force integer to 0 or 1.
- Push Onto Pointer: `*p++ = value`, set value to an element and advance the to the next.
- Pop Off Pointer: `x = *--p`, get value and move back to previous element. 
- Set compiler -Wimplicit-fallthrough to check for missing breaks.
- Do not use variable-length arrays. 
- Watch for numeric overflow.
- Bounds check all array and pointer access. 
- [X Macros](https://www.drdobbs.com/the-new-c-x-macros/184401387)
- [Overloading Functions in C](http://locklessinc.com/articles/overloading/)

## Formatting

- Indent with tabs. Tab stop 4 columns.
- Align with spaces following tab indentation.
- Function open braces in column 0.
- Struct, union, enum open braces after declaration.
- Single space between control keyword (if, for, etc.) and parens.
- Single statements on next line, indented, no braces.
- Open brace on same line, once space after close parens.
- Close brace on separate line, same column as keyword.

## Naming Conventions

- Identifiers and tags use `lower_snake_case`.
- Typedefs in `PascalCase`.
- Constants, Enumerations, Preprocessor symbols in `UPPER_SNAKE_CASE`.
- Enumerations prefixed by module or enum type name.
- Macro parameters are upper case with trailing underscore: `VAR_`

## Function and Type Abbreviations

Functions that have multiple versions for different types use an abbreviated type prefix. 
For example: int_max() and fl_min().

- char - ch
- int - int
- double - num
- bool - bool
- string - str
- pointer - p
- number/length/count - n
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

## Standard Names & Operations

Object Existence

- make:     Construct and return an initalized struct by value.
- create:   Bring new object into existence on heap.
- destroy:  Put an end to an object's existence.
- init:     Set existing object to a known usable state.
- copy:     Make/create a new object that is deep-copied from an existing object.

Dimensions

- length:    number of elements
- capacity:  total available space
- size:      number of bytes / char
- empty:     length is zero, no elements to get or remove
- full:      length == capacity, cannot add more elements

Sequences & Containers

- peek:     see next item without removing
- add:      new element in next available empty location
- get:      access element at key
- put:      replace element at key with new item
- ins:      add element at key, shift up
- del:      remove element at key, shift down

- find:      search for element's key by value
- replace:   overwrite (first/some/all) elements by value
- remove:    delete (first/some/all) elements by value 

- first:     element at beginning
- last:      element before end
- slice:     sub-set of elements by begin & end keys

Random access

- First index at 0. 
- Negative indices count from the end, with -1 being the last element.
- check(i) returns the absolute index. If *i* is negative, it is converted to the positive position from 0. If *i* is out of range, triggers an assertion.

Ordering

- sort
- reverse
- shuffle

Iterator Properties

- start   -  Beginning position
- stop    -  Ending position, past last element 
- step     - direction & number

Iterator State

- position - curent index, ID, or location within the range
- complete - *position* equals *stop*, no remaining elements
- pending  - begun and not completed
- peek     - Current element, fails if done.

Iterator Operations

- begin()    - Start itertion from first element of the *container* or *sequence*.
- next()     - advance to next element, no-op if *complete*.
- end()      - move to *stop*

Collection Pipeline Operations

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

## Things Every C Programmer Should Know

- Translation units
- Linkage: internal & external, static & external keywords
- Scope:  file/translation unit,function, block, function proto
- Storage class & duration: auto, static, extern, 
- Namespaces: tags, members, labels, identifiers
- Pointers
- Undefined behavior
- Preprocessor

# Build Modes

- Test - unit testing
- Debug - debugging
- Release - production

Enum, global constant, pre-proc symbols for current build mode.

Select build mode at compile time. 

# GDB Reference

- run: Start program.
- file FILENAME: load binary.
- l / list: list source.
- bt: backtrace
- restart 0: restart the current program. 

# Debug Categories

- Fatal
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

# Unit Testing
- Handle errors & assertions as test failures
- Uncaught exceptions are test failures.
- Testing for data alignment
- TEST_EQ(), TEST_STREQ()

# Assertions
- throwing failure
- write to log
- enable/disable asserts by level
- trigger breakpoint in debugger

## Assertion Types
- precondition, prereq, require
- postcondition, ensure
- expect
- check
- invariant
- given
- depend-on
- assume

# Pointer Safety & Memory Integrity

- Init pointers with special value
- Bounds checking
- Lock-and-key (dangling pointer detection)
- Over-allocate boundaries, init with special values, check for overflows.
- Reference counting
- Check bad inputs (null/bad pointer, zero/negative size)
- Initialize memory with special value
- Track memory usage (pointer, size, file/line at allocation)

## Fat Pointer
- key value
- size/length

## Memory Headers
- lock value
- ref counter
- size

	struct MemHeader {
		uint32 lock;
		int refcount;
		int size;
		SourceLocation source;
	}

# Error Handling 

- ignore, do nothing, off/disabled
- trace/log a message
- pause, prompt user
- return error code
- exception
- abort, halt program
- debug, breakpoint
- raise signal
- custom error function

# Error Codes

- OK, no error
- Error, general
- Math overflow
- Array overflow
- Null pointer
- Bad pointer
- Allocation failure
- Not found
- Test case failure
- File/disk error
- Bad user input

Operations

- status to string


# Arena Allocator

[null program: Arena allocator tips and tricks](https://nullprogram.com/blog/2023/09/27/)

Dependencies:

- Error handling (out of space error). 
	- Abort
	- Exceptions
		- Error codes, error info context
	- Error return code, monad
	- Custom error handler function
- Numeric overflow detection (for safely computing array sizes). 


# Tracing & Logging

- Logging is tracing to a file
- Structured output for easy parsing
- Thread-safe?

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

# Functors & Monads

[Fabulous adventures in coding: Monads](https://ericlippert.com/2013/02/21/monads-part-one/)


# Compound Types 
- Interval - min & max numerical range. 
- Vector - fixed-length, named & random access
- span - pair of pointers to front & back of elements
- slice, strip, strand, ?

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

