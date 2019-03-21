/*
  ==============================================================================
  
  MSH_CAM.H - v0.4
  
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
  as a quaternion, and output the view matrix as needed.

  Current implementation relies heavily on the msh_vec_math.h. Once both 
  msh_cam.h and msh_vec_math.h are thoroughly tested, I'll inline these 
  functions and remove dependency.
  
  While writing this file I have been looking at camera implementations by
  nlguillemot[1,2] and vurtun[3]. Some tricks that they've linked have
  been used, but no explicit code copying was done. 

  ==============================================================================
  TODOS
  [ ] Revise arcball controls.
  [ ] Add explicit center of rotation
  [ ] Add tweening
  [ ] Add quaternions / vectors if no vec math is provided 
  [ ] Remove dependency on msh_vec_math.h
  [ ] Add the capability to use a real camera parameters like focal length
         and center of projection.
  [ ] What is the interface for generating perspective?
  
  ==============================================================================
  REFERENCES:
  [1] nlguillemot/arcball_camera.h    https://github.com/nlguillemot/arcball_camera/blob/master/arcball_camera.h
  [2] nlguillemot/flythrough_camera.h https://github.com/nlguillemot/flythrough_camera/blob/master/flythrough_camera.h
  [3] vurtun/camera.c                 https://gist.github.com/vurtun/d41914c00b6608da3f6a73373b9533e5
  [4] Euler/Quaternion                https://gamedev.stackexchange.com/questions/13436/glm-euler-angles-to-quaternion
 */

/*
 * =============================================================================
 *       INCLUDES, TYPES AND DEFINES
 * =============================================================================
 */


#ifndef MSH_CAM
#define MSH_CAM

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_CAM_INCLUDE_HEADERS
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#endif


#ifdef MSH_CAM_STATIC
#define MSHCAMDEF static
#else
#define MSHCAMDEF extern
#endif


typedef struct msh_camera
{
  /* State */
  msh_vec3_t position;
  msh_quat_t orientation;
  
  /* Params */
  float fovy;
  float aspect_ratio;
  real32_t znear;
  real32_t zfar;

  /* Generated -- Not sure if should be stored, or computed per request*/
  msh_mat4_t view;
  msh_mat4_t proj;

} msh_camera_t;

MSHCAMDEF void
msh_camera_init( msh_camera_t * camera,
                         const msh_vec3_t eye,
                         const msh_vec3_t center, 
                         const msh_vec3_t up,
                         const float fovy,
                         const float aspect_ratio,
                         const float znear,
                         const float zfar );

MSHCAMDEF void
msh_camera_update_perspective( msh_camera_t *camera,
                               const float fovy, 
                               const float aspect_ratio,
                               const float znear, 
                               const float zfar );
MSHCAMDEF void
msh_camera_update_ortho( msh_camera_t *camera,
                         const float left,  const float right, 
                         const  float bottom, const float top, 
                         const float znear, const float zfar );

MSHCAMDEF void 
msh_arcball_camera_update( msh_camera_t *camera, 
                           const msh_vec2_t scrn_p0,  
                           const msh_vec2_t scrn_p1,
                           const int lmb_state,
                           const int rmb_state,
                           const float scroll_state, 
                           const msh_vec4_t viewport );



/* TODO: Design better simple api for basic version of the camera
 A) Use of msh_camera_t should be optional 
 B) Implement simple camera for flythrough.
 C) Make the camera use lookat point

MOST BASIC / ADVANCED API --->  Everything else will build from this method.

void msh_arcball_camera_update( float pos[3],
                                float look_at[3]
                                float up[3],

                                float fovy,
                                float aspect_ratio,
                                float near, float far,
                                
                                float p0_x, float p1_y,
                                float p1_x, float p1_y,
                                int lmb_press, int rmb_press,
                                float scroll,
                                
                                float w, ms_scalar_t h,
                                
                                float * view,
                                float * quaternion,
                                float * proj );
*/


/* TODO: Specify control schemes that will mimic certain software */
/* basic:
    - lmb: rotation
    - rmb: pan
    - scroll: zoom;
  blender:
    - mmb: rotation
    - shift+mmb: pan
    - ctrl+mmb: zoom(smooth)
    - scroll: zoom
    - ctrl+scroll: pan in x
    - shift+scroll: pan in y
*/

#ifdef __cplusplus
}
#endif

#endif /*MSH_CAM_H*/

#ifdef MSH_CAM_IMPLEMENTATION

static msh_vec3_t
msh__screen_to_sphere( float x, float y, msh_vec4_t viewport )
{
  float w = viewport.data[2] - viewport.data[0];
  float h = viewport.data[3] - viewport.data[1];
  float r = (w > h)? h : w;
  msh_vec3_t p = msh_vec3( (x - w * 0.5f) / r, 
                           ((h - y) - h*0.5f) / r, 
                           0.0f );
  float l_sq = p.x * p.x + p.y * p.y;
  float l = (float)sqrt( l_sq );
  p.z = (float)((l_sq > 0.5) ? (0.5 / l) : (sqrt( 1.0 - l_sq)));
  p = msh_vec3_normalize(p);
  return p;
}



MSHCAMDEF void
msh_camera_init( msh_camera_t * camera,
                         const msh_vec3_t eye,
                         const msh_vec3_t center, 
                         const msh_vec3_t up,
                         const float fovy,
                         const float aspect_ratio,
                         const float znear,
                         const float zfar )
{
  camera->view = msh_look_at( eye, center, up );
  camera->proj = msh_perspective( fovy, aspect_ratio, znear, zfar );
  camera->fovy = fovy;
  camera->aspect_ratio = aspect_ratio;
  camera->znear = znear;
  camera->zfar = zfar;

  msh_mat4_t view_inverse = msh_mat4_inverse( camera->view );

  camera->orientation     = msh_mat4_to_quat( view_inverse );
  camera->position        = msh_vec4_to_vec3( view_inverse.col[3] );
}

MSHCAMDEF void
msh_camera_update_perspective( msh_camera_t *camera,
                               const float fovy, 
                               const float aspect_ratio,
                               const float znear, 
                               const float zfar )
{
  camera->proj = msh_perspective( fovy, aspect_ratio, znear, zfar );
  camera->fovy = fovy;
  camera->aspect_ratio = aspect_ratio;
  camera->znear = znear;
  camera->zfar = zfar;
}

MSHCAMDEF void
msh_camera_update_ortho( msh_camera_t *camera,
                         const float left,  const float right, 
                         const  float bottom, const float top, 
                         const float znear, const float zfar )
{
  camera->proj = msh_ortho( left, right, bottom, top, znear, zfar );
  camera->znear = znear;
  camera->zfar = zfar;
}

MSHCAMDEF void 
msh_arcball_camera_update( msh_camera_t *camera, 
                           const msh_vec2_t scrn_p0,  
                           const msh_vec2_t scrn_p1,
                           const int lmb_state,
                           const int rmb_state,
                           const float scroll_state, 
                           const msh_vec4_t viewport ) 
{
  if( scroll_state )
  {
    msh_mat3_t cur_rot = msh_quat_to_mat3( camera->orientation );
    msh_vec3_t forward = msh_vec3_scalar_mul( cur_rot.col[2], -scroll_state );
    camera->position = msh_vec3_add( camera->position, forward );
  }
  else if(rmb_state)
  {
    msh_mat3_t cur_rot = msh_quat_to_mat3( camera->orientation );
    float w = viewport.data[2] - viewport.data[0]; 
    float h = viewport.data[3] - viewport.data[1];
    msh_vec2_t disp = msh_vec2( (scrn_p1.x - scrn_p0.x) / w,
                                (scrn_p1.y - scrn_p0.y) / h );
    msh_vec3_t u = msh_vec3_scalar_mul( cur_rot.col[0], 5 * -disp.x );
    msh_vec3_t v = msh_vec3_scalar_mul( cur_rot.col[1], 5 * disp.y );
    camera->position = msh_vec3_add( camera->position, u );
    camera->position = msh_vec3_add( camera->position, v );
  }
  else if(lmb_state) 
  { 
    /* Current orientation and position */
    msh_quat_t q = camera->orientation;
    msh_quat_t p = msh_quat(camera->position.x, 
                            camera->position.y, 
                            camera->position.z, 0.0);
    /* Compute the quaternion rotation from inputs */
    msh_mat3_t cur_rot = msh_quat_to_mat3( camera->orientation );
    msh_vec3_t p0 = msh_mat3_vec3_mul( cur_rot, 
                          msh__screen_to_sphere(scrn_p0.x, scrn_p0.y, viewport) );
    msh_vec3_t p1 = msh_mat3_vec3_mul( cur_rot, 
                          msh__screen_to_sphere(scrn_p1.x, scrn_p1.y, viewport) );
    
    /* compute rotation */
    msh_quat_t r = msh_quat_from_vectors( p1, p0 ); 
    r = msh_quat_slerp( msh_quat_identity(), r, 3.5 );

    /* Modify orientation and position */
    q = msh_quat_normalize( msh_quat_mul( r, q ) );
    p = msh_quat_mul( msh_quat_mul( r, p ), msh_quat_conjugate( r ) );
    
    camera->position = msh_vec3( p.x, p.y, p.z );
    camera->orientation = q;
  }

  /* Orientation and position are esentailly an inverse of view matrix.
     Thus we will convert them to a matrix form and invert to obtain
     view matrix. Note that we can use affine inverse formula */
  msh_mat3_t inv_o = msh_mat3_transpose(msh_quat_to_mat3( camera->orientation ));
  msh_vec3_t inv_p = msh_mat3_vec3_mul( inv_o, camera->position );
  camera->view = msh_mat3_to_mat4(inv_o);
  camera->view.col[3].x = -inv_p.x;
  camera->view.col[3].y = -inv_p.y;
  camera->view.col[3].z = -inv_p.z;
  camera->view.col[3].w = 1.0f;
}

void
msh_flythrough_camera( msh_camera_t *camera,
                       msh_vec2_t scrn_p0, msh_vec2_t scrn_p1,
                       int lmb_state,
                       int key_w_state, 
                       int key_s_state,
                       int key_a_state,
                       int key_d_state )
{
  msh_mat3_t r  = msh_quat_to_mat3( camera->orientation );
  float speed = 0.2;
  msh_vec3_t n  = msh_vec3( -speed * r.col[2].x, -speed * r.col[2].y, -speed * r.col[2].z );
  msh_vec3_t in = msh_vec3(  speed * r.col[2].x,  speed * r.col[2].y,  speed * r.col[2].z );
  msh_vec3_t v  = msh_vec3(  speed * r.col[0].x,  speed * r.col[0].y,  speed * r.col[0].z );
  msh_vec3_t iv = msh_vec3( -speed * r.col[0].x,  -speed * r.col[0].y, -speed * r.col[0].z );


  if( key_w_state ) camera->position = msh_vec3_add( camera->position, n );
  if( key_s_state ) camera->position = msh_vec3_add( camera->position, in );
  if( key_a_state ) camera->position = msh_vec3_add( camera->position, iv );
  if( key_d_state ) camera->position = msh_vec3_add( camera->position, v );

  if( lmb_state )
  {
    /* Current orientation and position */
    msh_quat_t q = camera->orientation;
    msh_vec4_t viewport = msh_vec4( 0, 0, 1024, 1024 );

    /* Compute the quaternion rotation from inputs */
    msh_mat3_t cur_rot = msh_mat3_identity();
    msh_vec3_t p0 = msh_mat3_vec3_mul( cur_rot, 
                          msh__screen_to_sphere(scrn_p0.x, 512.0, viewport) );
    msh_vec3_t p1 = msh_mat3_vec3_mul( cur_rot, 
                          msh__screen_to_sphere(scrn_p1.x, 512.0, viewport) );

    msh_quat_t r1 = msh_quat_from_vectors( p0, p1 ); 
    q = msh_quat_normalize( msh_quat_mul( r1, q ) );

    cur_rot = msh_quat_to_mat3( q );
    msh_vec3_t p2 = msh_mat3_vec3_mul( cur_rot, 
                          msh__screen_to_sphere(512.0, scrn_p0.y, viewport) );
    msh_vec3_t p3 = msh_mat3_vec3_mul( cur_rot, 
                          msh__screen_to_sphere(512.0, scrn_p1.y, viewport) );
    msh_quat_t r2 = msh_quat_from_vectors( p2, p3 ); 
    q = msh_quat_normalize( msh_quat_mul( r2, q ) );

    camera->orientation = q;
  }

  msh_mat3_t inv_o = msh_mat3_transpose(msh_quat_to_mat3( camera->orientation ));
  msh_vec3_t inv_p = msh_mat3_vec3_mul( inv_o, camera->position );
  
  camera->view = msh_mat3_to_mat4(inv_o);
  camera->view.col[3].x = -inv_p.x;
  camera->view.col[3].y = -inv_p.y;
  camera->view.col[3].z = -inv_p.z;
  camera->view.col[3].w = 1.0f;
}
#endif