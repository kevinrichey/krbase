% Hello, World
% by Kevin Richey

# To Do

- Use sanitizers
- Unit testing (isolate tests, fixture setup/teardown, auto-discovery, data alignment)
- Assertions
- Error detection & handling (error codes, handlers, "exceptions", runtime file:line debug info)
- Tracing & Logging
- Memory integrity (leak detection, memory pools, auto-release pools, convenience functions).
- Signal handling
- Localization & language support
- Security (avoiding security flaws in code)
- Configuration (command-line, config file, env vars, defaults)
- Data structures (string, dyn array, hash, list, span)
- Data de/serialization 
- Data file storage
- Parallel processing, multi-threading & forking
- Networking, inter-process communication
- Encryption, authentication
- Code generation & meta-programming tools
- Source & API documentation

## Five things to improve software quality

- Unit testing
- Assertions
- Error handling 
- Tracing & Logging
- Configuration

# Coding Style

## C Naming Conventions

Universal Code
: Types, functions, etc that are commonly used across code.
: Stand-along and universal functions may use lower_snake_case().
: Universal functions are not part of a module and used frequently throughout the code.

Modules
: A collection of types and functions that work together. Usually (not always) in a single translation unit.
: Modules are named in UpperCamelCase and used as a namespace prefix in the collection of types and functions.
: Functions in a module are named as ModuleName_FunctionName().
: Enums in a module are named as ModuleName_EnumName. 
: Enum members are ModuleName_EnumName_MemberName.


- Variables, parameters, struct & union members:  lower_snake_case
- Structs & Unions:    UpperCamelCase
- Struct & Union Functions:   UpperCamelCase()
- Enum tag/typedef:   EnumName
- Enum members:       EnumName_ValueName
- Preprocessor symbols: UPPER_SNAKE_CASE
- Preprocessor macros:  MACRO()
- Global Constants:   MODULE_CONSTANT_NAME                                               


### Common Meanings & Abbreviations

- Init:    Initialize object to a known state.
- Make:    Create object, init from params, return by value.
- New:     Create object on heap, init, return pointer.
- Dispose: Destructor, dispose of object.
- Length:  number of elements in a sequence (array, string, list, etc).
- Size:    number of bytes in an object, as in sizeof().
- Cap:     max number of elements a sequence may contain.

### Matching Pairs

- create, destroy
- init, dispose
- start, stop
- begin, end
- put, get
- set, unset
- add, remove
- enable, disable
- insert, delete
- load, save
- push, pop
- first, last


## Preprocessor

- Symbols without value
- Constants with a single literal value
- Shortcut symbol evaluates into an expression
- Expression macro
- Statement macro

## Formatting

- Indentation: 4 spaces (not tabs).
- Code blocks & braces
- Control statements (if, for, do, while, switch, etc)

## C Guildelines & Tips

Preprocessor Guidelines
: - Wrap statement macros in `do{ ... }while(0)`.
: - Wrap macro parameters in ()
: - Ensure variables declared within macros have unique names. Eg. use a prefix.
: - Use `_Bool` inside macros so user doesn't need to include stdbool.h.

Typedef Structs:
: `typedef struct NAME { ... } NAME;`

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
- [Duff's Device](http://www.catb.org/~esr/jargon/html/D/Duffs-device.html)
- [Overloading Functions in C](http://locklessinc.com/articles/overloading/)

# Ten Languages Every Programmer Should Know

- C
- awk
- regular expressions
- assembler
- Lisp-ish

# Resource Management 

acquire: Request the resource. This may fail if the resource is unavailable.
dispose: Return resource to the computer.
is_empty: Return true if the handle does not contain a resource. 
raw_data: Access the raw underlying resource. Requires !empty()
release: Nullify the handle and return the raw resource handle. Requires !empty()
reset: Dispose of the current resource (if exists) and assign new raw resource.  If parameter is null/empty, the object is left empty.
copy: Deep copy, replace resource with a duplicate of the parameter. If this resource !empty() then dispose it. This operation may not be applicable to all resources. 
swap: Exchange the resource with another resource handle.
move: Take the resource from another handle and leave that handle empty. Previous resource is disposed.

# Configuration

- Config File
- Command Line Arguments

# Text Output Formatting

Custom formatting escape codes

- Follow convention of `%spec` where 'spec' is a format specifier string.

Mechanism to map specifier names, struct members, and formatting procedures.
Static array of `struck_mem_ptr`, columns: specifier string, pointer offset to struct member, fn pointer to formatter.

    struct struct_mem_ptr;

    typedef void (*struct_mem_fn)(void* object, struct_mem_ptr* member);

    typedef struct struct_mem_ptr {
        const char* name;
        int8_t offset;
        struct_mem_fn func;
    } struct_mem_ptr;

Specifier      Type       Width   Purpose
-----------    ---------  ------  -----------------------
date           `time_t`   10      Date as YYYY-MM-DD
time           `time_t`   8       Time as HH:MM:SS
ticks          `clock_t`  10      System clock ticks
category       `char*`    10      Category name
lvl            `int`      3       Volume level
sfile          `char*`    *       Source filename
sline          `int`      6       Source line number
func           `char*`    *       Source function name as "function()"
msg            `char*`    *       Text message

# Data Structures

- Array
- Linked List
- Hash Table
- Associative Array
- Dynamically Sized Array
- Any/variant type
- Memory block manager?

## Associative Array

aka map, dictionary, hash table

Depends on

- Hash function (eg. Murmur3 32 bit)
- Linked list (for chaining)
- Dynamic Array (for hash table)


# Outline

- Data Structures
  - sequence: enumerated, ordered collection of objects.
  - array:  finite, fixed length sequence of data
  - string: finite sequence of characters
  - stream: infinite/unbounded sequence
  - list:   finite, dynamic sequence
  - vector
  - range
  - tuple

- Data structure operations
  - length: number of elements in a squence
  - size:   storage consumption (bytes) of sequence
  - capacity:  maxiumum length of sequence
  - first:     first element in sequence
  - last:      last element in sequence
  - end:       end of sequence after last element
  
- Files
  - Text Input & Parsing
  - Structured data
  - Tabular data
  - Markup

- Instrumentation

- Source Control
- Backups

- Process
    - SDLC
    - Requirements
    - Self-Organizing Teams
    - Progress Reporting

- Software Engineering
    - Architecture
    - Object-Oriented Programming
    - SOLID
    - Design Patterns
    - Test-Driven Development
    - Refactoring
    - Clean Code & Readability
    - You Aren't Gonna Need it (YAGNI)
    - Once and Only Once, Don't Repeat Yourself (DRY)
    - Interfaces
    - Technical Debt
    - Scripting 
    - Key Commands and Shortcuts
    - Documentation

- Tools, Compilers, Editors, Interpreters
    - Hardware, Keyboards, Screens

- Computer Science
    - CPU, memory, disk
    - Assembly
    - Memory models, stack & heap
    - Formal Data Structures
    - Algorithms
    - Compilers & Interpreters


# Terminology

Runtime errors
: Any error that occurs at runtime. 

Syntax errors
: An error in the program code that prevents the computer from executing the instructions. 
: This can occur at compile-time (for compiled languages) or runtime (for interpreted languages). 

