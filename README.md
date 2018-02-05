# interpreter
This project aims to provide a very minimal scripting language for embedded systems. Many other languages already exist, such as Lua, Tcl, or BASIC; however, most implementations require certain system calls like a read() and write(), as they expect a filesystem. This interpreter aims to be as independent and portable as possible: parsing script from strings one at a time, having minimal built-in functions (so the user can define their own prints and such), and only requiring a few standard library functions.  
  
To use this program with your own device, you need some malloc/free implementation, and string functions like those in string.h, atoi, and snprintf. Some of these functions may become coded in so that a standard library isn't required.  
  
Only a few commands are built in to the interpreter:  
* set - set variables
* func/end - define functions
* jmp - jump to line
  
Other features:  
* function/variable defining in c
* expression solving  
  
Soon:  
* conditionals
* error messages
  
This project is still in heavy development, so don't expect much. To include it in your own project, just link in parser.o and use the header files.
