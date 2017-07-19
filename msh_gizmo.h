/*
  ==============================================================================
  
  MSH_GIZMO.H - WIP!
  
  A single header library for manipulation gizmo. 

  To use the library you simply add:
  
  #define MSH_GIZMO_IMPLEMENTATION
  #include "msh_gizon.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh_gizmo.h"

  All functions can be made static by definining:

  #ifdef MSH_GIZMO_STATIC

  before including the "msh_gizmo.h"

  ==============================================================================
  DEPENDENCIES

  This library requires anonymous structs, which is a C11 extension.

  This library depends on following standard headers:

  TODO: Add dependencies list

  By default this library does not import these headers. Please see 
  docs/no_headers.md for explanation. Importing heades is enabled by:

  #define MSH_GIZMO_INCLUDE_HEADERS

  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)

  ==============================================================================
  LICENSE
  The MIT License (MIT)

  Copyright (c) 2016 Cedric Guillemet 
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
  This code requires some more serious thinking about rendering and 
  intersections.

  I do need to look at ImGui and NanoVG rendering code and design my after it...
  Or should I use nuklear?

  ==============================================================================
  TODOs
  [ ] Drawing

  ==============================================================================
  REFERENCES:
  [1] CedricGuillemet/ImGuizmo    https://github.com/CedricGuillemet/ImGuizmo
 */


#ifndef MSH_GIZMO_H
#define MSH_GIZMO_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_GIZMO_INCLUDE_HEADERS
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#endif

#ifdef MSH_GIZMO_STATIC
#define MSHCAMDEF static
#else
#define MSHCAMDEF extern
#endif

typedef struct msh_gizmo
{
  msh_mat4_t xform;
} msh_gizmo_t;

enum msh_gizmo_operations
{
  MSH_GIZMO_ROTATION,
  MSH_GIZMO_TRANSLATE,
  MSH_GIZMO_SCALE
};


bool msh_gizmo_is_cursor_over( msh_gizmo_t* gizmo );
bool msh_gizmo_is_active( msh_gizmo_t* gizmo );

void msh_gizmo_init( msh_gizmo_t* gizmo );
void msh_gizmo_draw( msh_gizmo_t* gizmo );

#ifdef __cplusplus
}
#endif

#endif /*MSH_GIZMO_H*/

#ifdef MSH_GIZMO_IMPLEMENTATION

void
msh_gizmo_init( msh_gizmo_t *gizmo )
{
  mshgfx_geometry_data_t
}

#endif