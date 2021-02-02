% Error Handling

# Intro

There are three general types of errors in computer science:
syntax errors
logic errors
runtime errors

Syntax errors are programming mistakes in the code grammer. The are detected by the comiler or interpreter and must be corrected for the program to run. In C, syntax errors are caught by the C compiler, reported to the programmer, and the source compilation fails. Obvious examples include forgetting to end a statement with semicolon or referencing an undeclared identifier. Syntax errors are preventable by writing gramaicaly correct code. 

Logic errors are programming mistakes that are not caught by the compiler and result in incorrect program execution. The compiler cannot detect them because they use proper code grammer, the code still compiles and runs. Dereferencing an uninitialized pointer and passing invalid parameter values are typical examples. Logic errors are preventable by writing logically correct code and scanning code with analysis tools. However, logic errors still frequently occur, and runtime checking is often useful to catch them. 

Runtime errors are not programming mistakes. They are errors the programmer cannot prevent. Therefore, our code must be prepared to handle them. Invalid user input and trying to open a file that does not exist are common runtime errors. 

The general process of dealing with errors is the same for all three types:
detect the error
report the error
handle the error
This is evident in the C compiler's handling of syntax errors. The parser reads invalid code grammer (detection), prints an error message to the programmer (reporting), fails the compilation and returns an error code (handling). 

I want an error handling mechanism that is extensible, easy, fast

There are a few ways that we can report an error, depending on the context:
print to the console (stdout or stderr)
write to a log file
display a GUI pop-up message
send over a network connection

Handling an error is deciding what to do about it. Each error is different so we want several options
terminate the process
cancel the current operation
ask the user what to do
report a warning and continue
ignore it / do nothing

Within your program, the point of error dection may not be near the point of handling. An error may occur in a low-level function, and the most appropriate place to deal with it is in high-level code. We also need a way to internally communicate an error from the detection point to the handler. This is called error propogagion. Common propogation schemes include 
throw an exception
return an error code
set an error state
invoke an error handler callback


I use two strategies for catching logic errors: unit testing and assertions. 
Both utilize the same basic element: the assertion function.
Libc provides the standard assert() macro, which is pretty useful, but lacking in some features. I prefer a more 

# Principles

- Flexible
- Decoupled
- Extensible
- Secure
- Readable
- Clean
- Solid

# Compilation & Runtime Modes

- Test - running under a testing framework
- Debug - running in a debugger
- Release - running in production
- Optimized - optimized for production

# Unit Testing

- Test cases are C functions with test assertions.
- Test isolation
- Fixture setup & teardown
- Discovery & auto-execution

### Test Assertions

- test(condition)
- testf(condition, fmt, ...)
- test_false(condition)
- test_equals_T(a, b)   // T: i=int, d=double, s=string, b=bool

## Test Results Output

Test Failure
: `FILE.c:LINE: Test FUNCTION failed: CONDITION`

Statistics (# tests, # failures)

Process returns 0 on all success, non-zero on any failures.

### Components

- test_assert(condition, file, line, fmt, ...)
- test_assert_eq_T("a", a, "b", b, file, line, fmt, ...)
- test_failure(file, line, fmt, ...)

# Assertions

- Detect failures (false assert condition)
- Test mode: return/throw up to test case
- Debug mode: report to console & abort
- Release mode: report to log & abort
- Optimized mode: compiled out

## Interface

- require(condition, caller_file, caller_line)
- fail(category, fmt, ...)

## Components

- assert_check(condition, category, file, line, func, fmt, ...)
- assert_failure(category, file, line, func, fmt, ...)

## Ouput

		FILE.c:LINE: CATEGORY failed: condition/message in FUNCTION

## Categories

- precondition
- postcondition
- invariant
- failure

# Error Checking

- Test: throw error up to test case & fail
- Debug: report to console
- Release: report to log

### Error Checking Interface

- error(&err, status, fmt, ...)
- error_clear(&err)
- error_init(&err, filename)
- check(&err, status, msg [,file, line])
- throw(status, jmp, file, line, fmt, ...)
- error_set_handler(handler_fn, user_data)

### Error Output

		FILE.c:LINE: Error STATUS: MESSAGE

### Error Handling Methods

- None (off/disabled)
- Return code
- Set error state
- Exceptions
- Terminate/abort
- Pause (stop and prompt user)
- Debug (stop at breakpoint)
- Raise signal


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
- Errors
- Assertion failures
- Debugging, Watch variables

## Log Management & Control

- Volume throttle (0 = always)
- Category on/off (null handler)
- Rotation, archiving, cleanup
- Test: tracing off
- Debug: trace to console, throttled
- Release: trace to log, throttled

### Trace Interface

- trace(category, fmt, ...)
- debug()
- watch(variable)
	- watch_T("var_name", T val)

### Trace Output

		FILE.c:LINE: CATEGORY-LEVEL: MESSAGE

### Log Interface

- log(category, level, fmt, ...)
- log_init(&log)
- log_set_volume(&log, level)
- log_rotate(&log)
- log_archive(&log, archive)
- log_cleanup(&log, age)

### Log Output

		YYYY-MM-DD HH:MM:SS: CATEGORY-LEVEL: MESSAGE

Profiling

- Clock time, high-res
- Source file:line
- Function
- Begin & end

# Categories

- Assertion
	- Precondition
	- Postcondition
	- Invariant
- Item not found (in container)
- Error
- Warning
- Trace
	- Debug
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
		- Progress
		- Output
	- End
	- Shutdown

# Status

- Okay
- Failure
- Error
- Out of memory
- File not found
- Null pointer
- Out of bounds

# General Output Format

Assertion: FILE.c:LINE: CATEGORY failed: condition/message in FUNCTION
Test:      FILE.c:LINE: Test FUNCTION failed: CONDITION
Error:     FILE.c:LINE: Error STATUS: MESSAGE
Trace:     FILE.c:LINE: CATEGORY-LEVEL: MESSAGE
Log:       YYYY-MM-DD HH:MM:SS: CATEGORY-LEVEL: MESSAGE

FILE:LINE: CATEGORY STATUS: MESSAGE in FUNCTION

