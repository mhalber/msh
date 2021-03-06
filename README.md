# msh - Maciej's Toolbox

[Examples repository](https://github.com/mhalber/msh_examples)

This repository contains a number of libraries and utilities that I use for my daily development, written in C. 

## Libraries

Name                     |  version   | description  
:------------------------|:----------:|:---------------------------------------------
**msh_std.h**            |    0.6     | various helper functions and typedefs
**msh_containers.h**     |    0.5     | Dynamic Array, Hash table, Disjoint union set
**msh_argparse.h**       |    0.8     | command-line argument parsing
**msh_vec_math.h**       |    0.82    | glsl inspired vector math (vectors, matrices and quaternions)
**msh_ply.h**            |    1.01    | Fast PLY File I/O [Benchmark](https://github.com/mhalber/ply_io_benchmark)

## Utilites

Name                     |  version   | description                         | dependencies
:------------------------|:----------:|:-----------------------------------:|:-------
**msh_camera.h**         |    0.65    | various types of camera controls    | msh_vec_math.h

## Experimental folder

Experimental folder contains some code that I am working on during my spare time, and is
generally very unstable, might be completely changed or deleted. Would not encourage anyone to use
these files.

## Disclaimer

These libraries have been inspired by excellent [libraries](https://github.com/nothings/stb) by Sean Barret. 
I'd also like to point you to many other single file libraries:

- [gb](https://github.com/gingerBill/gb)
- [cute_headers](https://github.com/RandyGaul/cute_headers)
- [mattias gustavsson's libs](https://github.com/mattiasgustavsson/libs)
- [rjm](https://github.com/rmitton/rjm)
- [sokol](https://github.com/floooh/sokol)

They are excellent and you should do yourself a favor and try them out!
