# interpreter
This project aims to provide a very minimal scripting language for embedded systems. Many other languages already exist, such as Lua, Tcl, or BASIC; however, most implementations require certain system calls like a read() and write(), as they expect a filesystem. This script interpreter intends to be as independent and portable as possible: parsing script from strings one at a time, having minimal built-in functions (so the user can define their own prints and such), and only requiring a few standard library functions.  
  
To use this program with your own device, you need some malloc/free implementation, atoi, strtof, and snprintf. Some other standard library functions are needed, but something like newlib should be able to provide them without needing any system calls.  
  
Features:  
* function/variable defining through C
* functions and variables in script
* conditionals - if/else/end, do/while
* solve - solve expressions stored in string variables

Todo list:
* scopes for variables
* error messages
* arrays?
* maybe for loops
  
This project is still in heavy development, so don't expect much. To include it in your own project, just link in libinterp.a (for ARM, link all .o's for x86) and use the header files.
