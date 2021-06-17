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

This encoding leaves several possible encodings for future datatypes, including C functions or C types.

## Embedding Interface

The embedder will be able to interface via the construction and evaluation of Lisp forms, making the creation of values and executino of code the same process.
They can select their allocators, configure the garbage collector, and pause the garbage collector on certain values.
