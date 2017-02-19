/*
  ==============================================================================
  
  MSH_CAM.H - WIP!
  
  A single header library for first person and arcball camera manipulation 

  To use the library you simply add:
  
  #define MSH_CAM_IMPLEMENTATION
  #include "msh_cam.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh_cam.h"

  All functions can be made static by definining:

  #ifdef MSH_CAM_STATIC

  before including the "msh_cam.h"

  ==============================================================================
  DEPENDENCIES

  This library requires anonymous structs, which is a C11 extension.
  Tested compilers:
    clang 3.9.1
    apple clang (Apple LLVM version 8.0.0 (clang-800.0.42.1))
    gcc 5.3.0

  This library depends on following standard headers:

  TODO: Add dependencies list

  By default this library does not import these headers. Please see 
  docs/no_headers.md for explanation. Importing heades is enabled by:

  #define MSH_CAM_INCLUDE_HEADERS

  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)

  ==============================================================================
  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  ==============================================================================
  NOTES 

  ==============================================================================

  TODO: Everything!
  TODO: Add quaternions / vectors if no vec math is provided 
  TODO: Remove dependency on msh_vec_math.h


  ==============================================================================
  REFERENCES:

 */

/*
 * =============================================================================
 *       INCLUDES, TYPES AND DEFINES
 * =============================================================================
 */


#ifndef MSH_CAM_H
#define MHSCAM_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_CAM_INCLUDE_HEADERS
#include <stdio.h>
#include <float.h>
#include <math.h>
#endif

#ifdef MSH_CAM_STATIC
#define MSHCAMDEF static
#else
#define MSHCAMDEF extern
#endif

#ifndef MSH_VEC_MATH
typedef msh_scalar_t float 
#endif

typedef struct msh_camera
{
  msh_vec3_t position;
  msh_quat_t orientation;  
} msh_camera_t;

typedef struct mshcam_first_person_controls
{
  msh_scalar_t speed;
} mshcam_first_person_controls_t;

typedef struct mshcam_trackball_controls
{
  msh_scalar_t speed;
} mshcam_trackball_controls_t;


#ifdef __cplusplus
}
#endif

#endif /*MSH_CAM_H*/

#ifdef MSH_CAM_IMPLEMENTATION

#endif
