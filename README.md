<div align="center">
	<h1>Aqua Scheme</h1>
	<blockquote>
		🌊 an embeddable Scheme dialect focused on speed and low memory footprint
	</blockquote>
  <p align="center">
		<a href="https://github.com/garfr/aqua/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22"><img src="https://img.shields.io/github/issues/garfr/aqua/help%20wanted?color=green"></a>
		<a href="LICENSE"><img src="https://img.shields.io/badge/license-ISC-blue.svg"></a>
	</p>
</div>

Aqua Scheme is a lightweight interpreter for a Scheme-like language, with a first class C API for embedding.  Inspired by projects like Lua and Chibi Scheme, Aqua Scheme seeks to be a powerful, expressive language that can fit into a tiny package.  

Despite being in a small package, Aqua doesn't aim to be another "single header Lisp" project. Instead, it has an efficient bytecode VM and compiler wrapped up in a zero dependency C library.  Other features planned include:

* Full support for continuations
* UTF-8 string handling
* Arbitrary precision math  
* Both low and high level macros

## WIP

This project is still WIP, both the VM and bytecode compiler are still under construction, and the features above are still in the works.  Besides the internals, documentation of the language or its embedding API doesn't exist yet.  This means contributions are welcome!

## Building

Currently, Aqua Scheme has only been tested on 64-bit Linux, but it is written in pure C99 with no external dependencies, so porting to other POSIX platforms should be smooth.

### Dependencies

* ``meson``
* ``ninja``

### Steps 

Clone the repo and enter it.
```
git clone https://github.com/garfr/aqua
```
```
cd aqua
```
Initialize the Meson build folder and enter it.
```
mkdir build
```
```
meson setup build
```

```
cd build/
```
Build the project.
```
ninja
```
The library will be placed in the ``build/`` directory as ``libaqua.so``, and an REPL executable that can be used to interact with the language will be placed in the same directory as ``aqua``.
