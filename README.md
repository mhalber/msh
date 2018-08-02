# msh

[Examples repository](https://github.com/mhalber/msh_examples)

Set of C99/C11 libraries under the public-domain / MIT license (please see particular files for details).  
These are libraries that I use for daily development and intend to extend the set and improve them over time.

Currently libraries are grouped in two tiers:

- Tier 1: Truly single header file library with no dependencies other than cstdlib

- Tier 2: Work in progress libraries that might require other msh lib to be used

Overtime hopefully every single library in this repo will be moved to Tier 1.


## TIER 1

library                  |  version   | description  
:------------------------|:----------:|:---------------------------------------------
**msh_std.h**            |    0.5     | standard library functionality
**msh_argparse.h**       |    0.75    | command-line argument parsing
**msh_vec_math.h**       |    0.75    | glsl inspired vector math (vectors, matrices and quaternions)
**msh_gfx.h**            |    0.35    | openGL wrapper
**msh_ply.h**            |    0.6     | PLY File I/O
**msh_hash_grid.h**      |    0.5     | Hash grid data structure for radius queries in 3D


## TIER 2

library                  |  version   | description                         | dependencies
:------------------------|:----------:|:-----------------------------------:|:-------
**msh_cam.h**            |    0.2     | various types of camera controls    | msh_vec_math.h
**msh_geometry.h**       |    0.01    | geometrical primitives manipulation | msh_vec_math.h



## Experimental folder

Experimental folder contains some code that I am working on during my spare time, and is
generally very unstable, might be completly changed or deleted. Would not encourage anyone to use it.

## Disclamer

These libraries have been inspired and modelled after excellent
[libraries](https://github.com/nothings/stb) by Sean Barret. I'd also like to point you
to many other single file libraries:

- [gb](https://github.com/gingerBill/gb)

- [cute_headers](https://github.com/RandyGaul/cute_headers)

- [matthias gustavsson's libs](https://github.com/mattiasgustavsson/libs)

They are excellent quality and you should do yourself a favor to try them out!
