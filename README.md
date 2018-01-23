# interpreter
This project aims to provide a very minimal scripting language for embedded systems. Many other languages already exist, such as Lua, Tcl, or BASIC; however, most implementations require certain system calls like a read() and write(), expecting a filesystem. This interpreter aims to be as independent as possible: parsing script from strings at a time, having minimal built-in functions (so the user can define their own prints and such), and only requiring a few library functions.  
  
To use this program with your own device, only two functions are truly needed: malloc, and strcmp.  
  
This project is still in heavy development, so don't expect much. Right now only function calls are supported, without variable name expansion. To include it in your own project, just link in parser.o and use the header files.
