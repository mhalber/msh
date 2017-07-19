/*
  ==============================================================================
  
  MSH_3D_DESCRIPTORS.H - WIP!
  
  A single header library for computing 3d descriptors for pointcloud data. 

  To use the library you simply add:
  
  #define MSH_3D_DESCRIPTORS_IMPLEMENTATION
  #include "msh_3d_descriptors.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh_3d_descriptors.h"

  All functions can be made static by definining:

  #ifdef MSH_3D_DESCRIPTORS_STATIC

  before including the "msh_3d_descriptors.h"

  ==============================================================================
  DEPENDENCIES

  This library requires anonymous structs, which is a C11 extension.

  This library depends on following standard headers:

  TODO: Add dependencies list

  By default this library does not import these headers. Please see 
  docs/no_headers.md for explanation. Importing heades is enabled by:

  #define MSH_3D_DESCRIPTORS_INCLUDE_HEADERS

  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)

  ==============================================================================
  LICENSE
  The MIT License (MIT)

  Copyright (c) 2017 Maciej Halber

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  
  
  ==============================================================================
  NOTES 

  ==============================================================================
  TODOs
  [ ] kdtree functions - first test with off-the-shelf kdtree and then see what
      happens with that.
  [ ]

  ==============================================================================
  REFERENCES:

 */


#ifndef MSH_3D_DESCRIPTORS_H
#define MSH_3D_DESCRIPTORS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_3D_DESCRIPTORS_INCLUDE_HEADERS
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#endif

#ifdef MSH_3D_DESCRIPTORS_STATIC
#define MSH3DDDEF static
#else
#define MSH3DDDEF extern
#endif

typedef struct msh3dd_ctx
{
  void* kd_tree;
  void* 
} msh3dd_ctx_t;

void msh3dd_init( );

void msh3dd_quadric_descriptor( const msh3dd_ctx_t* ctx, 
                                const float* positions, const float* normals, 
                                const size_t n_points,
                                float* quadric_descriptor );

#ifdef __cplusplus
}
#endif

#endif /*MSH_3D_DESCRIPTORS_H*/

#ifdef MSH_3D_DESCRIPTORS_IMPLEMENTATION


#endif /*MSH_3D_DESCRIPTORS_IMPLEMENTATION*/