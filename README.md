# interpreter
This project aims to provide a very minimal scripting language for embedded systems. Many other languages already exist, such as Lua, Tcl, or BASIC; however, most implementations require certain system calls like read() and write(), as they expect a filesystem. This interpreter wants to be as system-independent and portable as possible: parsing script from strings one at a time, having minimal built-in functions (so the user can define their own IO calls and such), and only requiring a few standard library functions.  
  
To use this program with your own device, you'll need some malloc/free implementation, and a freestanding standard library. Newlib works well for this; however, functions like atoi() and snprintf() will probably need to be rewritten (if you don't have an \_sbrk defined).  
  
Interpreter features:
* Variable/function definition - in C and in script
* if/else and while loops
* a solve function to parse strings at runtime

Inconvenient features: 
* no local variables
* whitespace sometimes ignored
  
Some TODO items: 
* fix all memory leaks
* add better error messages
* arrays?
* for loops 
 

This project can be made for the host system (```make```) or an ARM system (```make arm```). 
This project is still in heavy development, so don't expect much. To include it in your own project, just link in libinterp.a (for ARM) and use the header files.

