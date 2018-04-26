# interpreter

*interpreter* is a minimal scripting language aimed at low-spec embedded systems. 

Many other scripting languages currently exist for embedded use, including Lua, Tcl, and BASIC; however, their implementations require certain system calls (e.g. read()/write()) that may not be available or used on an embedded device. As a result, *interpreter* was made with the goal to allow advanced scripts to be run in minimal conditions. This included things like having only a few built-in functions, and only loading script from C-strings one at a time.  
  
To use *interpreter* with your own device, you will need some malloc/free implementation and a freestanding standard library. Newlib can work well for this, although functions like atoi() and snprintf() will probably need to be rewritten (if you don't have an \_sbrk defined).  
  
### interpreter features:  
* Variable/function definition - in C and in script
* if/else conditionals
* while loops
* a solve function to parse strings at script runtime

### inconvenient features:  
* some code formatting is enforced due to parser limitation
* all variables are global
* whitespace is not always ignored

### some TODO items:  
* add better error messages
* better array support
* for loops
* variable scopes

## building and running

This project can be made for the host system (```mv shell.c.bak shell.c; make```) or an ARM system (```mv shell.c shell.c.bak; make arm```). Make's ```-j``` argument may be used to multithread compilation.  
  
To run on the host system, run ```./shell some_script_file```. Note that no IO functions are built in, so you must define your own.  
To use on an ARM device, simply link ```libinterp.a``` into your program and use the header files. See ```shell.c``` for an idea of how to the interpreter.  

&nbsp;  

This project is still in heavy development, so don't expect too much. See the wiki for a scripting guide.
