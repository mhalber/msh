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

  PLAN : Create camera that only uses the position and orientation represented 
  as a quaternion.

  ==============================================================================

  TODO: Everything!
  TODO: Add quaternions / vectors if no vec math is provided 
  TODO: Remove dependency on msh_vec_math.h
  TODO: Add the capability to use a real camera parameters like focal length
  and center of projection.
  TODO: What is the interface for generating perspective?

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
  /* State */
  msh_vec3_t position;
  msh_quat_t orientation;  
  
  /* Params */
  msh_scalar_t fov_y;
  msh_scalar_t aspect_ratio;
  msh_scalar_t near;
  msh_scalar_t far;

  /* Generated -- Not sure if should be stored, or computed per request*/
  msh_mat4_t view;
  msh_mat4_t proj;

} msh_camera_t;


/* NOTE: Possibly hold all the ogl related stuff in controls. But probably it
should just be a way to configure the controls to specific needs. 
I like this more. It should store things like the speed, interface interaction */

// typedef struct mshcam_controls
// {
// 
// } mshcam_controls_t;


msh_trackball_controls_update( msh_camera_t *camera );
msh_first_person_controls_update( msh_camera_t *camera );

#ifdef __cplusplus
}
#endif

#endif /*MSH_CAM_H*/

#ifdef MSH_CAM_IMPLEMENTATION

static bool 
msh__trackball_zoom( msh_camera_t *camera, const int wheel_tick )
{
  if( wheel_tick )
  {
    /* TODO: Implement zooming */
    return false;
  }
  return false;
}

static bool 
msh__trackball_pan( msh_camera_t *camera, 
                    const msh_vec2_t prev_pos,
                    const msh_vec2_t cur_pos )
{
  if( prev_pos.x != cur_pos.x || prev_pos.y != cur_pos.y )
  {
    /* TODO: implement panning */
    return false;
  }
  return false;
}

static msh_vec3_t
msh__screen_to_sphere( msh_scalar_t x, msh_scalar_t y, msh_vec4_t viewport )
{
  msh_scalar_t w = viewport[2] - viewport[0];
  msh_scalar_t h = viewport[3] - viewport[1];

  msh_vec3_t p = msh_vec3( 2.0 * x / w - 1.0, 1.0 - 2.0 * y / h, 0.0 );
  msh_scalar_t l_sq = p.x * p.x + p.y * p.y + p.z * p.z;
  msh_scalar_t l = sqrt( l_sq );

  p.z = l_sq > 0.5 ? 0.5 / l : sqrt( 1.0 - l_sq);

  msh_scalar_t denom = 1.0 / l;
  p.x *= denom; p.y *= denom; p.z *= denom;
  return p;
}


static bool
msh__trackball_rotate( msh_camera *camera, 
                       const msh_vec2_t prev_pos,
                       const msh_vec2_t cur_pos )
{
  if( prev_pos.x != cur_pos.x || prev_pos.y != cur_pos.y )
  {
    /* TODO: implement rotation */
    return false;
  }
  return false;

}

MSHCAMDEF void 
msh_trackball_camera_update( msh_camera_t * camera, 
                             const msh_vec2_t prev_pos, 
                             const msh_vec2_t cur_pos,
                             const int wheel_tick,
                             const msh_vec4_t viewport )
{
  bool update = false;
  update |= msh__tracball_zoom( wheel_tick );
  update |= msh__trackball_pan();
  update |= msh__trackball_rotate();

  if( update )
  {
    /* new look at */
  }
}

#endif
