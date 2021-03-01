% Error Handling

# Intro

There are three general types of errors in computer science:
syntax errors
logic errors
runtime errors

Syntax errors are programming mistakes in the code grammar. The are detected by the compiler or interpreter and must be corrected for the program to run. In C, syntax errors are caught by the C compiler, reported to the programmer, and the source compilation fails. Obvious examples include forgetting to end a statement with semicolon or referencing an undeclared identifier. Syntax errors are preventable by writing grammatically correct code. 

Logic errors are programming mistakes that are not caught by the compiler and result in incorrect program execution. The compiler cannot detect them because they use proper code grammar, the code still compiles and runs. Dereferencing an uninitialized pointer and passing invalid parameter values are typical examples. Logic errors are preventable by writing logically correct code and scanning code with analysis tools. However, logic errors still frequently occur, and runtime checking is often useful to catch them. 

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

# Compilation & Runtime Modes

- Test - running under a testing framework
- Debug - running in a debugger
- Release - running in production
- Optimized - optimized for production

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



