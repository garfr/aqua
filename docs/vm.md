# VM Design

## Goals

### Library Structure

* The VM should be organized as a library to be embedded in other projects.
* The entire lib should be reentrant.
* A currently running instance should be a regular C object, with each instance able to specify different garbage collector characteristics, allocators, etc.
* The instantiating code should be able to extract/inject values, call functions, and have extreme granularity of control over the executing code.

### Efficient Closures

* Closures are extremely powerful and expressive, and in order to promote their use they should be implemented efficiently.
* Control flow analysis is complex and slow, and out of scop:we for an embedded language.
* The naive approach is to allocate an environment for every scope and simply dynamically lookup each variable, but this makes register allocation impossible.
* One technique is the use of "upvalues", used by the reference Lua implementation, a variation on this may be useful.

### Numerical Tower

* Full infinite-precision support should be in the language, but it doesn't need to be heap allocated from the start.
* Numbers can start as doubles or even integers and be moved into heap allocated BigInts by the VM silently.
* This should be implemented portably (no 128-bit integer support), and at least somewhat efficiently (high precision arithmetic isn't especially common in embedded scripting).

### Trade Extreme Flexibility for Performance 

* Full first-class continuations are not certain yet, and an efficient implementation technique will need to be found if they are to be included.
* There should be at least some advanced flow control technique in the language, whether it be continuations, coroutines, or something else.
* Functions like "define", "lambda", and "if" dont' need to be actual functions and can be considered special forms that are removed during compilation.

## High Level Design

* Register based VM.
* Compacting mark and sweep garbage collector (maybe mark and copy).
* Small object representation

### Stack vs. Register

In the general, a register based VM is more efficient, due to the smaller bytecode size needed and less time spent in operation dispatch.
If this project is to be written in a portable C standard, operation dispatch will be extra expensive due to the lack of calculated goto, so a register VM is an obvious choice.

### Garbage Collector

I am not an expert in garbage collectors, and I am still researching what there is in efficient techniques. However, the biggest thing is that the garbage collector must be incremental and not stop-the world.
Nobody wants their embedded language pausing execution of the entire program for 500ms.

Scheme heavily utilizes linked lists, which when allocated far apart are extremely inefficient in their cache usage. A compacting garbage collector can help solve this problem, and makes repeated allocation cheaper.

In order to keep the size of the project down, it should be usable with only 1 garbage collector compiled in.
Reference counting requires another garbage collector to clear up circular references.
If you don't use a compacting garbage collector, a compaction algorithm will be needed occasionally to preserve cache locality and stop fragmentation.
A compacting garbage collector is the obvious choice in this scenario.

### Continuations

First class continuations are immutable, and when they are captured the entire call stack is allocated onto the heap and saved, then when the continuation is executed it is simply copied back onto the stack.  This lets the call stack be used as normal by the program and keeps continuations as a zero-cost abstraction, if you don't use them, you don't pay in memory.

### Object Representation

Some common objects shouldn't be heap allocated, but the copying of objects should still be cheap.  We can keep items like bools, ints, nil, in a single integer if we align the heap.
This limits the size of integers, and requires that doubles be heap allocated.
If this method is to be used, portability is limited as the C standard doesn't define the way that regular integers and pointers can be casted to each other (you must use uintptr_t).

## Object Encoding

There supports these types:

### Value Types
* Integer (61 bit signed)
* Bool
* Nil
* Char

### Heap Allocated Values (tag + a bit shifted pointer aligned to 16 bytes)
* Pair
* Closure
* Array
* Table
* Continuation
* Symbol 
* Bignum

They are encoded as so:

* Integer - 0b000 followed by 61 bits of integer value
* Bool    - 0b101111 followed by 1 or 0
* Char    - 0b011111 followed by 32 bits of a Unicode codepoint
* Nil     - 0b001111

The rest of the encodings are followed by 60 bits of a pointer into the heap.

* Pair    - 0b0001
* Closure - 0b0010
* Array   - 0b0011
* Table   - 0b0100
* Contin. - 0b0101
* Bignum  - 0b0110
* Symbol  - 0b0111

This encoding leaves several possible encodings for future datatypes, including C functions or C types.

## Embedding Interface

The embedder will be able to interface via the construction and evaluation of Lisp forms, making the creation of values and executino of code the same process.
They can select their allocators, configure the garbage collector, and pause the garbage collector on certain values.

## Instruction Encoding

Instructions are encoded into 32 bit unsigned integers.  There are 2 possible instruction enccodings, ABC and AD.  ABC is 3 8-bit fields, and AD is one 8 bit field and a 16 bit fields, used for jumps and operations that require more than 256 registers, upvalues, or constants (rare). R(A), R(B), R(C), and R(D) are used to state that the value stored in the register numerated by A, B, C, or D. The same goes for K (integers), S (strings).

## Evaluation Model

The key observation that leads to great optimizations is that user code will only ever be executed at the toplevel, and that eval only has access to global variables.  This means that only global variables need to be placed in lookup tables, and local variables can be allocated to registers.
Similar to the Lua VM, the VM uses two stacks, one used for function activation records, and the other used for local variables.  This lets functions use a dynamic amount of registers without needing an O(n) algorithm to search for activation records on a single stack.  

The basic component of execution is a closure, an instantiation of a template.  The template object stores the code, literals, name, debug info and the closure holds a pointer to a template and the variables captured during the closures creation.

Literals (integers, strings, symbols) are placed in an array and can be loaded into registers, or used directly in instructions.

## Operations

* OP_RETR - AD - returns the value of R(D)
* OP_RETK - AD - offset encoding - returns the value of K(D)
* OP_RETNIL - (no encoding) - returns nil

* OP_MOVR - AD - moves R(D) into R(A)
* OP_MOVK - AD - moves K(D) into R(A)

* OP_NIL - AD - moves the value nil into R(D)

* OP_ADDRR - ABC - stores the result of adding R(B) and R(C) into R(A)
* OP_SUBRR - ABC - stores the result of subtracting R(B) and R(C) into R(A)
* OP_MULRR - ABC - stores the result of multiplying R(B) and R(C) into R(A)
* OP_DIVRR - ABC - stores the result of dividing R(B) and R(C) into R(A)

* OP_ADDRK - ABC - stores the result of adding R(B) and K(C) into R(A)
* OP_SUBRK - ABC - stores the result of subtracting R(B) and K(C) into R(A)
* OP_MULRK - ABC - stores the result of multiplying R(B) and K(C) into R(A)
* OP_DIVRK - ABC - stores the result of dividing R(B) and K(C) into R(A)

* OP_ADDKR - ABC - stores the result of adding K(B) and R(C) into R(A)
* OP_SUBKR - ABC - stores the result of subtracting K(B) and R(C) into R(A)
* OP_MULKR - ABC - stores the result of multiplying K(B) and R(C) into R(A)
* OP_DIVKR - ABC - stores the result of dividing K(B) and R(C) into R(A)

* OP_ADDKK - ABC - stores the result of adding K(B) and K(C) into R(A)
* OP_MULKK - ABC - stores the result of multiplying K(B) and K(C) into R(A)
* OP_SUBKK - ABC - stores the result of subtracting K(B) and K(C) into R(A)
* OP_DIVKK - ABC - stores the result of dividing K(B) and K(C) into R(A)

* OP_CONSRR - ABC - creates a new pair stored in R(A) with R(B) and R(C) as car and cdr
* OP_CONSRK - ABC - creates a new pair stored in R(A) with R(B) and K(C) as car and cdr
* OP_CONSKR - ABC - creates a new pair stored in R(A) with K(B) and R(C) as car and cdr
* OP_CONSKK - ABC - creates a new pair stored in K(A) with K(B) and R(C) as car and cdr

* OP_CAR - AD - stores the car of R(D) in R(A)
* OP_CDR - AD - stores the cdr of R(D) in R(A)

* OP_TABNEW - AD - creates a new table and places it in R(A), with at least R(D) buckets

* OP_TABGETR - ABC - sets R(A) to the value of the table in R(B) indexed by R(C)
* OP_TABGETK - ABC - sets R(A) to the value of the table in R(B) indexed by K(C)

* OP_TABSETRR - ABC - adds to the table in R(A) an entry with key R(B) and value R(C)
* OP_TABSETRK - ABC - adds to the table in R(A) an entry with key R(B) and value K(C)
* OP_TABSETKR - ABC - to the table in R(A) an entry with key K(B) and value R(C)
* OP_TABSETKK - ABC - adds to the table in R(A) an entry with key K(B) and value K(C)

* OP_EQRR - ABC - R(A) is set to R(B) == R(C) 
* OP_EQRK - ABC - R(A) is set to R(B) == K(C) 
* OP_EQKK - ABC - R(A) is set to K(B) == K(C) 

* OP_LTRR - ABC - R(A) is set to R(B < R(B)
* OP_LTRK - ABC - R(A) is set to R(B < K(B)
* OP_LTKR - ABC - R(A) is set to K(B < R(B)
* OP_LTKK - ABC - R(A) is set to K(B < K(B)

* OP_LTERR - ABC - R(A) is set to R(B <= R(B)
* OP_LTERK - ABC - R(A) is set to R(B <= K(B)
* OP_LTEKR - ABC - R(A) is set to K(B <= R(B)
* OP_LTEKK - ABC - R(A) is set to K(B <= K(B)

* OP_JMPF - AD - jumps D instructions forward if A evaluates to truthy
* OP_JMPB - AD - jumps D instructions back if A evaluates to truthy

* OP_GGETR - AD - sets R(A) to the value indexed in the global table by R(D)
* OP_GGETK - AD -  sets R(A) to the value indexed in the global table by K(D)

* OP_GSETRR - AD - sets the table value in the global table with key R(A) and val R(D)
* OP_GSETKR - AD - sets the table value in the global table with key K(A) and val R(D)
* OP_GSETRK - AD - sets the table value in the global table with key R(A) and val K(D)
* OP_GSETKK - AD - sets the table value in the global table with key K(A) and val K(D)

* OP_DISPLAYR - AD - prints to stdout the value in R(D)
* OP_DISPLAYK - AD - prints to stdout the value in K(D)
