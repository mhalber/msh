/*
  ==============================================================================
  
  MSH_CAMERAERA.H - v0.5
  
  A single header library for camera manipulation 

  To use the library you simply add:
  
  #define MSH_CAMERA_IMPLEMENTATION
  #include "msh_cam.h"

  ==============================================================================
  API DOCUMENTATION

  ==============================================================================
  TODOs:
  [ ] API Implementation
  [ ] API docs

  ==============================================================================
  AUTHORS
    Maciej Halber (macikuh@gmail.com)

  Licensing information can be found at the end of the file.
  
  ==============================================================================
  DEPENDENCES

  This library depends on following standard headers:
    <stdlib.h>  - qsort, atof, atoi
    <stdio.h>   - printf, sprintf
    <string.h>  - strncmp
    <ctype.h>   - isdigit
    <stdbool.h> - bool type

  By default this library does not import these headers. Please see 
  docs/no_headers.md for explanation. Importing heades is enabled by:

  #define MSH_ARGPARSE_INCLUDE_HEADERS
  
  ==============================================================================
  REFERENCES:
  [1] nlguillemot/arcball_camera.h    https://github.com/nlguillemot/arcball_camera/blob/master/arcball_camera.h
  [2] nlguillemot/flythrough_camera.h https://github.com/nlguillemot/flythrough_camera/blob/master/flythrough_camera.h
  [3] vurtun/camera.c                 https://gist.github.com/vurtun/d41914c00b6608da3f6a73373b9533e5
  [4] Euler/Quaternion                https://gamedev.stackexchange.com/questions/13436/glm-euler-angles-to-quaternion
 */

/
#ifndef MSH_CAMERA
#define MSH_CAMERA

#ifndef MSH_VEC_MATH
#error "Please include msh_vec_math first!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_CAMERA_INCLUDE_HEADERS
#include <float.h>
#include <math.h>
#include <stdbool.h>
#endif


#ifdef MSH_CAMERA_STATIC
#define MSHCAMDEF static
#else
#define MSHCAMDEF extern
#endif


typedef struct msh_camera
{
  /* View Matrix Params */
  msh_vec3_t origin;
  msh_vec3_t offset;
  msh_quat_t orientation;

  /* Projection Matrix Params */
  int32_t viewport[4];
  float znear, zfar;
  float fovy;
  int8_t ortho;

  /* Options */
  float momentum;
  float pan_speed;
  float zoom_speed;
  float rot_speed;

  /* Generated */
  msh_vec3_t location;
  msh_mat4_t view;
  msh_mat4_t proj;
} msh_camera_t;

void msh_camera_rotate( msh_camera_t* cam, msh_vec2_t prev_pos, msh_vec2_t curr_pos );
void msh_camera_pan( msh_camera_t* cam, msh_vec2_t prev_pos, msh_vec2_t curr_pos );
void msh_camera_zoom( msh_camera_t* cam, float zoom_amount );
void msh_camera_move( msh_camera_t* cam, msh_vec3_t translation );

void msh_camera_update_view( msh_camera_t * cam );
void msh_camera_update_proj( msh_camera_t * cam );
void msh_camera_update( msh_camera_t * cam );

void msh_camera_init_arcball( msh_camera_t * cam, msh_vec3_t eye, msh_vec3_t target, msh_vec3_t up,
                              msh_vec4_t viewport, float fovy, float znear, float zfar );
void msh_camera_init_firstperson( msh_camera_t * cam, msh_vec3_t eye, msh_vec3_t target, msh_vec3_t up,
                                  msh_vec4_t viewport, float fovy, float znear, float zfar );

void msh_camera_ray_through_pixel( msh_camera_t* camera, msh_vec2_t p, msh_vec3_t* origin, msh_vec3_t* direction );

#ifdef __cplusplus
}
#endif

#endif /*MSH_CAMERA_H*/

#ifdef MSH_CAMERA_IMPLEMENTATION

#endif

/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2019 Maciej Halber

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to do 
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.

------------------------------------------------------------------------------

ALTERNATIVE B - Public Domain (www.unlicense.org)

This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.

In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

------------------------------------------------------------------------------
*/