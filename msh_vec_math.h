/*
  ==============================================================================
  
  MSH_VEC_MATH.H 
  
  A single header library for a simple 2-4 dimensional vector/
  matrix operations (and quaternions). 

  To use the library you simply add:
  
  #define MSH_VEC_MATH_IMPLEMENTATION
  #include "msh_vec_math.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh_vec_math.h"

  All functions can be made static by definining:

  #ifdef MSH_VEC_MATH_STATIC

  before including the "msh_vec_math.h"

  By default all calculations are done using floating point operations
  If you wish to use double, please define(as usual, before including the file):

  #define MSH_VEC_MATH_USE_DOUBLE

  ==============================================================================
  DEPENDENCIES

  This library requires anonymous structs, which is a C11 extension.
  Tested compilers:
    clang 3.9.1
    apple clang (Apple LLVM version 8.0.0 (clang-800.0.42.1))
    gcc 5.3.0

  This library depends on following standard headers:
    <float.h>  -- FLT_EPSILON
    <math.h>   -- sinf, cosf, sqrtf
    <stdio.h>  -- fprintf, printf

  By default this library does not import these headers. Please see 
  docs/no_headers.md for explanation. Importing heades is enabled by:

  #define MSH_VEC_MATH_INCLUDE_HEADERS


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

    1. For small structs the pass/return by value has no performance penalty 
       as compared to passing pointers

  ==============================================================================

  TODO: Come up with an example when the static definition is necessary and
        helps.
  TODO: Simplify lerp
  TODO: Cpp overloads?
  TODO: Double check quaternion operations
  TODO: Add quat from axis-angle
  TODO: Test performance of matrices... Without optimization it will probably 
        be slow.
  TODO: Fix x_equal functions. The epsilon comparison is apparently wrong.
  TODO: Add quat from euler angles!
  TODO: Normalize the quaternion after lerping!
  TODO: Add testing
  TODO: Typedef the scalar type and add an option

  ==============================================================================
  REFERENCES:
    [1] Quaternion To Matrix  http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/
    [2] Matrix To Quaternion  http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
    [3] Axis Angle To Quat.   http://www.euclideanspace.com/maths/geometry/rotations/conversions/angleToQuaternion/
    [4] Euler To Quaternion   http://www.euclideanspace.com/maths/geometry/rotations/conversions/eulerToQuaternion/index.html
    [5] Matrix Inverse        http://download.intel.com/design/PentiumIII/sml/24504301.pdf
    [6] Quat. Normalization   https://www.mathworks.com/help/aerotbx/ug/quatnormalize.html
    [7] Quaternions 1         http://www.cs.virginia.edu/~gfx/Courses/2010/IntroGraphics/Lectures/29-Quaternions.pdf
    [8] Quaternions 2         http://www.3dgep.com/understanding-quaternions/
 
 */

/*
 * =============================================================================
 *       INCLUDES, TYPES AND DEFINES
 * =============================================================================
 */

#ifndef MSH_VEC_MATH
#define MSH_VEC_MATH

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_VEC_MATH_INCLUDE_HEADERS
#include <stdio.h>
#include <float.h>
#include <math.h>
#endif

#ifndef MSH_FLT_EPSILON
#define MSH_FLT_EPSILON FLT_EPSILON
#endif

#ifdef MSH_VEC_MATH_STATIC
#define MSHVMDEF static
#else
#define MSHVMDEF extern
#endif

#ifdef MSH_VEC_MATH_USE_DOUBLE
typedef double msh_scalar_t; 
#else
typedef float msh_scalar_t;
#endif

typedef union vec2
{
  msh_scalar_t data[2];
  struct { msh_scalar_t x; msh_scalar_t y; };
  struct { msh_scalar_t r; msh_scalar_t g; };
} msh_vec2_t;

typedef union vec3
{
  msh_scalar_t data[3];
  struct { msh_scalar_t x; msh_scalar_t y; msh_scalar_t z; };
  struct { msh_scalar_t r; msh_scalar_t g; msh_scalar_t b; };
} msh_vec3_t;

typedef union vec4
{
  msh_scalar_t data[4];
  struct { msh_scalar_t x; msh_scalar_t y; msh_scalar_t z; msh_scalar_t w; };
  struct { msh_scalar_t r; msh_scalar_t g; msh_scalar_t b; msh_scalar_t a; };
} msh_vec4_t;

typedef union quaternion
{
  msh_scalar_t data[4];
  struct { msh_scalar_t x, y, z, w; };
  struct { msh_vec3_t im; msh_scalar_t re; };
} msh_quat_t;

typedef union mat2
{
  msh_scalar_t data[4];
  msh_vec2_t col[2];
} msh_mat2_t;

typedef union mat3
{
  msh_scalar_t data[9];
  msh_vec3_t col[3];
} msh_mat3_t;

typedef union mat4
{
  msh_scalar_t data[16];
  msh_vec4_t col[4];
} msh_mat4_t;

typedef msh_vec2_t msh_point2_t;
typedef msh_vec3_t msh_point3_t;
typedef msh_vec3_t msh_point4_t;

/*
 * =============================================================================
 *       VECTORS
 * =============================================================================
 */

#define msh_vec2_zeros() (msh_vec2_t){{0, 0}}
#define msh_vec3_zeros() (msh_vec3_t){{0, 0, 0}}
#define msh_vec4_zeros() (msh_vec4_t){{0, 0, 0, 0}}
#define msh_vec2_ones() (msh_vec2_t){{1, 1}}
#define msh_vec3_ones() (msh_vec3_t){{1, 1, 1}}
#define msh_vec4_ones() (msh_vec4_t){{1, 1, 1, 1}}
#define msh_vec2_value(x) (msh_vec2_t){{x, x}}
#define msh_vec3_value(x) (msh_vec3_t){{x, x, x}}
#define msh_vec4_value(x) (msh_vec4_t){{x, x, x, x}}
#define msh_vec2(x,y) (msh_vec2_t){{x, y}}
#define msh_vec3(x,y,z) (msh_vec3_t){{x, y, z}}
#define msh_vec4(x,y,z,w) (msh_vec4_t){{x, y, z, w}}

MSHVMDEF msh_vec2_t msh_vec3_to_vec2( msh_vec3_t v );
MSHVMDEF msh_vec2_t msh_vec4_to_vec2( msh_vec4_t v );
MSHVMDEF msh_vec3_t msh_vec2_to_vec3( msh_vec2_t v );
MSHVMDEF msh_vec3_t msh_vec4_to_vec3( msh_vec4_t v );
MSHVMDEF msh_vec4_t msh_vec2_to_vec4( msh_vec2_t v );
MSHVMDEF msh_vec4_t msh_vec3_to_vec4( msh_vec3_t v );

MSHVMDEF msh_vec2_t msh_vec2_add( msh_vec2_t a, msh_vec2_t b );
MSHVMDEF msh_vec3_t msh_vec3_add( msh_vec3_t a, msh_vec3_t b );
MSHVMDEF msh_vec4_t msh_vec4_add( msh_vec4_t a, msh_vec4_t b );

MSHVMDEF msh_vec2_t msh_vec2_scalar_add( msh_vec2_t v, float s );
MSHVMDEF msh_vec3_t msh_vec3_scalar_add( msh_vec3_t v, float s );
MSHVMDEF msh_vec4_t msh_vec4_scalar_add( msh_vec4_t v, float s );

MSHVMDEF msh_vec2_t msh_vec2_sub( msh_vec2_t a, msh_vec2_t b );
MSHVMDEF msh_vec3_t msh_vec3_sub( msh_vec3_t a, msh_vec3_t b );
MSHVMDEF msh_vec4_t msh_vec4_sub( msh_vec4_t a, msh_vec4_t b );

MSHVMDEF msh_vec2_t msh_vec2_scalar_sub( msh_vec2_t v, float s );
MSHVMDEF msh_vec3_t msh_vec3_scalar_sub( msh_vec3_t v, float s );
MSHVMDEF msh_vec4_t msh_vec4_scalar_sub( msh_vec4_t v, float s );

MSHVMDEF msh_vec2_t msh_vec2_mul( msh_vec2_t a, msh_vec2_t b );
MSHVMDEF msh_vec3_t msh_vec3_mul( msh_vec3_t a, msh_vec3_t b );
MSHVMDEF msh_vec4_t msh_vec4_mul( msh_vec4_t a, msh_vec4_t b );

MSHVMDEF msh_vec2_t msh_vec2_scalar_mul( msh_vec2_t v, float s );
MSHVMDEF msh_vec3_t msh_vec3_scalar_mul( msh_vec3_t v, float s );
MSHVMDEF msh_vec4_t msh_vec4_scalar_mul( msh_vec4_t v, float s );

MSHVMDEF msh_vec2_t msh_vec2_div( msh_vec2_t a, msh_vec2_t b );
MSHVMDEF msh_vec3_t msh_vec3_div( msh_vec3_t a, msh_vec3_t b );
MSHVMDEF msh_vec4_t msh_vec4_div( msh_vec4_t a, msh_vec4_t b );

MSHVMDEF msh_vec2_t msh_vec2_scalar_div( msh_vec2_t v, float s );
MSHVMDEF msh_vec3_t msh_vec3_scalar_div( msh_vec3_t v, float s );
MSHVMDEF msh_vec4_t msh_vec4_scalar_div( msh_vec4_t v, float s );

MSHVMDEF msh_vec2_t msh_vec2_abs( msh_vec2_t v );
MSHVMDEF msh_vec3_t msh_vec3_abs( msh_vec3_t v );
MSHVMDEF msh_vec4_t msh_vec4_abs( msh_vec4_t v );

MSHVMDEF msh_vec2_t msh_vec2_sqrt( msh_vec2_t v );
MSHVMDEF msh_vec3_t msh_vec3_sqrt( msh_vec3_t v );
MSHVMDEF msh_vec4_t msh_vec4_sqrt( msh_vec4_t v );

MSHVMDEF msh_vec2_t msh_vec2_clamp( msh_vec2_t v, float min, float max);
MSHVMDEF msh_vec3_t msh_vec3_clamp( msh_vec3_t v, float min, float max);
MSHVMDEF msh_vec4_t msh_vec4_clamp( msh_vec4_t v, float min, float max);

MSHVMDEF msh_vec2_t msh_vec2_invert( msh_vec2_t v );
MSHVMDEF msh_vec3_t msh_vec3_invert( msh_vec3_t v );
MSHVMDEF msh_vec4_t msh_vec4_invert( msh_vec4_t v );

MSHVMDEF msh_vec2_t msh_vec2_normalize( msh_vec2_t v );
MSHVMDEF msh_vec3_t msh_vec3_normalize( msh_vec3_t v );
MSHVMDEF msh_vec4_t msh_vec4_normalize( msh_vec4_t v );

MSHVMDEF float msh_vec2_dot( msh_vec2_t a, msh_vec2_t b );
MSHVMDEF float msh_vec3_dot( msh_vec3_t a, msh_vec3_t b );
MSHVMDEF float msh_vec4_dot( msh_vec4_t a, msh_vec4_t b );

MSHVMDEF msh_vec3_t msh_vec3_cross( msh_vec3_t a, msh_vec3_t b );

MSHVMDEF float msh_vec2_norm( msh_vec2_t v );
MSHVMDEF float msh_vec3_norm( msh_vec3_t v );
MSHVMDEF float msh_vec4_norm( msh_vec4_t v );

MSHVMDEF float msh_vec2_norm_sq( msh_vec2_t v );
MSHVMDEF float msh_vec3_norm_sq( msh_vec3_t v );
MSHVMDEF float msh_vec4_norm_sq( msh_vec4_t v );

MSHVMDEF int msh_vec2_equal( msh_vec2_t a, msh_vec2_t b );
MSHVMDEF int msh_vec3_equal( msh_vec3_t a, msh_vec3_t b );
MSHVMDEF int msh_vec4_equal( msh_vec4_t a, msh_vec4_t b );

MSHVMDEF void msh_vec2_fprint( msh_vec2_t v, FILE *stream );
MSHVMDEF void msh_vec3_fprint( msh_vec3_t v, FILE *stream );
MSHVMDEF void msh_vec4_fprint( msh_vec4_t v, FILE *stream );
MSHVMDEF void msh_vec2_print( msh_vec2_t v );
MSHVMDEF void msh_vec3_print( msh_vec3_t v );
MSHVMDEF void msh_vec4_print( msh_vec4_t v );

/*
 * =============================================================================
 *       MATRICES
 * =============================================================================
 */

#define msh_mat2_zeros() (msh_mat2_t){{0, 0, 0, 0}}
#define msh_mat3_zeros() (msh_mat3_t){{0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define msh_mat4_zeros() (msh_mat4_t){{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define msh_mat2_identity() (msh_mat2_t){{1, 0, 0, 1}}
#define msh_mat3_identity() (msh_mat3_t){{1, 0, 0, 0, 1, 0, 0, 0, 1}}
#define msh_mat4_identity() (msh_mat4_t){{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}}
#define msh_mat2_diag(x) (msh_mat2_t){{x, 0, 0, x}}
#define msh_mat3_diag(x) (msh_mat3_t){{x, 0, 0, 0, x, 0, 0, 0, x}}
#define msh_mat4_diag(x) (msh_mat4_t){{x, 0, 0, 0, 0, x, 0, 0, 0, 0, x, 0, 0, 0, 0, x}}

MSHVMDEF msh_mat2_t msh_mat3_to_mat2( msh_mat3_t m );
MSHVMDEF msh_mat2_t msh_mat4_to_mat2( msh_mat4_t m );
MSHVMDEF msh_mat3_t msh_mat2_to_mat3( msh_mat2_t m );
MSHVMDEF msh_mat3_t msh_mat4_to_mat3( msh_mat4_t m );
MSHVMDEF msh_mat4_t msh_mat2_to_mat4( msh_mat2_t m );
MSHVMDEF msh_mat4_t msh_mat3_to_mat4( msh_mat3_t m );

MSHVMDEF msh_mat2_t msh_mat2_add( msh_mat2_t a, msh_mat2_t b );
MSHVMDEF msh_mat3_t msh_mat3_add( msh_mat3_t a, msh_mat3_t b );
MSHVMDEF msh_mat4_t msh_mat4_add( msh_mat4_t a, msh_mat4_t b );

MSHVMDEF msh_mat2_t msh_mat2_scalar_add( msh_mat2_t m, float s );
MSHVMDEF msh_mat3_t msh_mat3_scalar_add( msh_mat3_t m, float s );
MSHVMDEF msh_mat4_t msh_mat4_scalar_add( msh_mat4_t m, float s );

MSHVMDEF msh_mat2_t msh_mat2_sub( msh_mat2_t a, msh_mat2_t b );
MSHVMDEF msh_mat3_t msh_mat3_sub( msh_mat3_t a, msh_mat3_t b );
MSHVMDEF msh_mat4_t msh_mat4_sub( msh_mat4_t a, msh_mat4_t b );

MSHVMDEF msh_mat2_t msh_mat2_scalar_sub( msh_mat2_t m, float s );
MSHVMDEF msh_mat3_t msh_mat3_scalar_sub( msh_mat3_t m, float s );
MSHVMDEF msh_mat4_t msh_mat4_scalar_sub( msh_mat4_t m, float s );

MSHVMDEF msh_mat2_t msh_mat2_mul( msh_mat2_t a, msh_mat2_t b );
MSHVMDEF msh_mat3_t msh_mat3_mul( msh_mat3_t a, msh_mat3_t b );
MSHVMDEF msh_mat4_t msh_mat4_mul( msh_mat4_t a, msh_mat4_t b );

MSHVMDEF msh_mat2_t msh_mat2_scalar_mul( msh_mat2_t m, float s );
MSHVMDEF msh_mat3_t msh_mat3_scalar_mul( msh_mat3_t m, float s );
MSHVMDEF msh_mat4_t msh_mat4_scalar_mul( msh_mat4_t m, float s );

MSHVMDEF msh_vec2_t msh_mat2_vec2_mul( msh_mat2_t m, msh_vec2_t v );
MSHVMDEF msh_vec3_t msh_mat3_vec3_mul( msh_mat3_t m, msh_vec3_t v );
MSHVMDEF msh_vec4_t msh_mat4_vec4_mul( msh_mat4_t m, msh_vec4_t v );

MSHVMDEF msh_mat2_t msh_mat2_scalar_div( msh_mat2_t m, float s );
MSHVMDEF msh_mat3_t msh_mat3_scalar_div( msh_mat3_t m, float s );
MSHVMDEF msh_mat4_t msh_mat4_scalar_div( msh_mat4_t m, float s );

MSHVMDEF float msh_mat2_trace( msh_mat2_t m );
MSHVMDEF float msh_mat3_trace( msh_mat3_t m );
MSHVMDEF float msh_mat4_trace( msh_mat4_t m );

MSHVMDEF float msh_mat2_determinant( msh_mat2_t m );
MSHVMDEF float msh_mat3_determinant( msh_mat3_t m );
MSHVMDEF float msh_mat4_determinant( msh_mat4_t m );

MSHVMDEF float msh_mat2_frobenius_norm( msh_mat2_t m );
MSHVMDEF float msh_mat3_frobenius_norm( msh_mat3_t m );
MSHVMDEF float msh_mat4_frobenius_norm( msh_mat4_t m );

MSHVMDEF msh_mat2_t msh_mat2_inverse( msh_mat2_t m );
MSHVMDEF msh_mat3_t msh_mat3_inverse( msh_mat3_t m );
MSHVMDEF msh_mat4_t msh_mat4_inverse( msh_mat4_t m );

MSHVMDEF msh_mat2_t msh_mat2_transpose( msh_mat2_t m );
MSHVMDEF msh_mat3_t msh_mat3_transpose( msh_mat3_t m );
MSHVMDEF msh_mat4_t msh_mat4_transpose( msh_mat4_t m );

MSHVMDEF msh_mat4_t msh_look_at( msh_vec3_t eye, 
                        msh_vec3_t center, 
                        msh_vec3_t up );

MSHVMDEF msh_mat4_t msh_frustum( float left,   float right, 
                                 float bottom, float top, 
                                 float z_near, float z_far );

MSHVMDEF msh_mat4_t msh_perspective( float fovy, 
                                     float aspect, 
                                     float z_near, 
                                     float z_far);

MSHVMDEF msh_mat4_t msh_ortho( float left,   float right, 
                               float bottom, float top, 
                               float z_near, float z_far );

MSHVMDEF msh_vec3_t msh_project( msh_vec4_t obj, 
                                 msh_mat4_t model, 
                                 msh_mat4_t project, 
                                 msh_vec4_t viewport );

MSHVMDEF msh_vec4_t msh_unproject( msh_vec3_t win, 
                                   msh_mat4_t model, 
                                   msh_mat4_t project, 
                                   msh_vec4_t viewport );

MSHVMDEF msh_mat4_t msh_translate( msh_mat4_t m, msh_vec3_t t );
MSHVMDEF msh_mat4_t msh_scale( msh_mat4_t m, msh_vec3_t s );
MSHVMDEF msh_mat4_t msh_rotate( msh_mat4_t m, 
                       float angle, 
                       msh_vec3_t axis );

MSHVMDEF int msh_mat2_equal( msh_mat2_t a, msh_mat2_t b );
MSHVMDEF int msh_mat3_equal( msh_mat3_t a, msh_mat3_t b );
MSHVMDEF int msh_mat4_equal( msh_mat4_t a, msh_mat4_t b );

MSHVMDEF void msh_mat2_fprint( msh_mat2_t v, FILE *stream );
MSHVMDEF void msh_mat3_fprint( msh_mat3_t v, FILE *stream );
MSHVMDEF void msh_mat4_fprint( msh_mat4_t v, FILE *stream );
MSHVMDEF void msh_mat2_print( msh_mat2_t v );
MSHVMDEF void msh_mat3_print( msh_mat3_t v );
MSHVMDEF void msh_mat4_print( msh_mat4_t v );

/*
msh_mat4_t pca( msh_vec3_t * points, i32 n_points );
void eig( msh_mat3_t * mat, msh_mat3_t * eigvec, msh_vec3_t * eigval );
*/

/*
 * =============================================================================
 *       QUATERNIONS
 * =============================================================================
 */

MSHVMDEF msh_quat_t msh_quat_from_axis_angle( msh_vec3_t axis, float angle );

MSHVMDEF msh_mat3_t msh_quat_to_mat3( msh_quat_t q );
MSHVMDEF msh_mat4_t msh_quat_to_mat4( msh_quat_t q );
MSHVMDEF msh_quat_t msh_mat3_to_quat( msh_mat3_t m );
MSHVMDEF msh_quat_t msh_mat4_to_quat( msh_mat4_t m ); 

MSHVMDEF msh_quat_t msh_quat_add( msh_quat_t a, msh_quat_t b );
MSHVMDEF msh_quat_t msh_quat_scalar_add( msh_quat_t v, float s );

MSHVMDEF msh_quat_t msh_quat_sub( msh_quat_t a, msh_quat_t b );
MSHVMDEF msh_quat_t msh_quat_scalar_sub( msh_quat_t v, float s );

MSHVMDEF msh_quat_t msh_quat_mul( msh_quat_t a, msh_quat_t b );
MSHVMDEF msh_quat_t msh_quat_scalar_mul( msh_quat_t v, float s );

MSHVMDEF msh_quat_t msh_quat_div( msh_quat_t a, msh_quat_t b );
MSHVMDEF msh_quat_t msh_quat_scalar_div( msh_quat_t v, float s );

MSHVMDEF float msh_quat_dot( msh_quat_t a,  msh_quat_t b );
MSHVMDEF float msh_quat_norm( msh_quat_t q );
MSHVMDEF float msh_quat_norm_sq( msh_quat_t q );

MSHVMDEF msh_quat_t msh_quat_normalize( msh_quat_t q );
MSHVMDEF msh_quat_t msh_quat_conjugate( msh_quat_t q );
MSHVMDEF msh_quat_t msh_quat_inverse( msh_quat_t q );

MSHVMDEF msh_quat_t msh_quat_lerp( msh_quat_t q, 
                                   msh_quat_t r, 
                                   float t );
MSHVMDEF msh_quat_t msh_quat_slerp( msh_quat_t q, 
                                    msh_quat_t r, 
                                    float t );

#ifdef __cplusplus
}
#endif


/*
 * =============================================================================
 *       OPERATORS
 * =============================================================================
 */

#ifdef __cplusplus
MSHVMDEF msh_vec2_t operator+( msh_vec2_t a, msh_vec2_t b ); 
MSHVMDEF msh_vec3_t operator+( msh_vec3_t a, msh_vec3_t b ); 
MSHVMDEF msh_vec4_t operator+( msh_vec4_t a, msh_vec4_t b ); 

MSHVMDEF msh_vec2_t &operator+=( msh_vec2_t &a, msh_vec2_t &b ); 
MSHVMDEF msh_vec3_t &operator+=( msh_vec3_t &a, msh_vec3_t &b ); 
MSHVMDEF msh_vec4_t &operator+=( msh_vec4_t &a, msh_vec4_t &b ); 
#endif


#endif /* MSH_VEC_MATH */


#ifdef MSH_VEC_MATH_IMPLEMENTATION

/*
 * =============================================================================
 *       VECTOR IMPLEMENTATION
 * =============================================================================
 */

MSHVMDEF inline msh_vec2_t 
msh_vec3_to_vec2( msh_vec3_t v )
{
  return (msh_vec2_t){{ v.x, v.y }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec4_to_vec2( msh_vec4_t v )
{
  return (msh_vec2_t){{ v.x, v.y }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec2_to_vec3( msh_vec2_t v )
{
  return (msh_vec3_t){{ v.x, v.y, 0.0f }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec4_to_vec3( msh_vec4_t v )
{
  return (msh_vec3_t){{ v.x, v.y, v.z }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec2_to_vec4( msh_vec2_t v )
{
  return (msh_vec4_t){{ v.x, v.y, 0.0f, 0.0f }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec3_to_vec4( msh_vec3_t v )
{
  return (msh_vec4_t){{ v.x, v.y, v.z, 0.0f }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_add( msh_vec2_t a, msh_vec2_t b )
{
  return (msh_vec2_t){{ a.x + b.x, a.y + b.y }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_add( msh_vec3_t a, msh_vec3_t b )
{
  return (msh_vec3_t){{ a.x + b.x, a.y + b.y, a.z + b.z }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_add( msh_vec4_t a, msh_vec4_t b )
{
  return (msh_vec4_t){{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_scalar_add( msh_vec2_t v, float s )
{
  return (msh_vec2_t){{ v.x + s, v.y + s }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_scalar_add( msh_vec3_t v, float s )
{
  return (msh_vec3_t){{ v.x + s, v.y + s, v.z + s }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_scalar_add( msh_vec4_t v, float s )
{
  return (msh_vec4_t){{ v.x + s, v.y + s, v.z + s, v.w + s }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_sub( msh_vec2_t a, msh_vec2_t b )
{
  return (msh_vec2_t){{ a.x - b.x, a.y - b.y }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_sub( msh_vec3_t a, msh_vec3_t b )
{
  return (msh_vec3_t){{ a.x - b.x, a.y - b.y, a.z - b.z }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_sub( msh_vec4_t a, msh_vec4_t b )
{
  return (msh_vec4_t){{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_scalar_sub( msh_vec2_t v, float s )
{
  return (msh_vec2_t){{ v.x - s, v.y - s }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_scalar_sub( msh_vec3_t v, float s )
{
  return (msh_vec3_t){{ v.x - s, v.y - s, v.z - s }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_scalar_sub( msh_vec4_t v, float s )
{
  return (msh_vec4_t){{ v.x - s, v.y - s, v.z - s, v.w - s }};
}


MSHVMDEF inline msh_vec2_t 
msh_vec2_mul( msh_vec2_t a, msh_vec2_t b )
{
  return (msh_vec2_t){{ a.x * b.x, a.y * b.y }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_mul( msh_vec3_t a, msh_vec3_t b )
{
  return (msh_vec3_t){{ a.x * b.x, a.y * b.y, a.z * b.z }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_mul( msh_vec4_t a, msh_vec4_t b )
{
  return (msh_vec4_t){{ a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_scalar_mul( msh_vec2_t v, float s )
{
  return (msh_vec2_t){{ v.x * s, v.y * s }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_scalar_mul( msh_vec3_t v, float s )
{
  return (msh_vec3_t){{ v.x * s, v.y * s, v.z * s }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_scalar_mul( msh_vec4_t v, float s )
{
  return (msh_vec4_t){{ v.x * s, v.y * s, v.z * s, v.w * s }};
}


MSHVMDEF inline msh_vec2_t 
msh_vec2_div( msh_vec2_t a, msh_vec2_t b )
{
  return (msh_vec2_t){{ a.x / b.x, a.y / b.y }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_div( msh_vec3_t a, msh_vec3_t b )
{
  return (msh_vec3_t){{ a.x / b.x, a.y / b.y, a.z / b.z }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_div( msh_vec4_t a, msh_vec4_t b )
{
  return (msh_vec4_t){{ a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_scalar_div( msh_vec2_t v, float s )
{
  float denom = 1.0f / s;
  return (msh_vec2_t){{ v.x * denom, v.y * denom }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_scalar_div( msh_vec3_t v, float s )
{
  float denom = 1.0f / s;
  return (msh_vec3_t){{ v.x * denom, v.y * denom, v.z * denom }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_scalar_div( msh_vec4_t v, float s )
{
  float denom = 1.0f / s;
  return (msh_vec4_t){{ v.x * denom, v.y * denom, v.z * denom, v.w * denom }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_abs( msh_vec2_t v )
{
  return (msh_vec2_t){{ fabsf(v.x), fabsf(v.y) }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_abs( msh_vec3_t v )
{
  return (msh_vec3_t){{ fabsf(v.x), fabsf(v.y), fabsf(v.z) }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_abs( msh_vec4_t v )
{
  return (msh_vec4_t){{ fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w) }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_sqrt( msh_vec2_t v )
{
  return (msh_vec2_t){{ sqrtf(v.x), sqrtf(v.y) }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_sqrt( msh_vec3_t v )
{
  return (msh_vec3_t){{ sqrtf(v.x), sqrtf(v.y), sqrtf(v.z) }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_sqrt( msh_vec4_t v )
{
  return (msh_vec4_t){{ sqrtf(v.x), sqrtf(v.y), sqrtf(v.z), sqrtf(v.w) }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_clamp( msh_vec2_t v, float min, float max )
{
  if ( min > max )
  {
    return v;
  }
  return (msh_vec2_t){{ fminf( fmaxf( v.x, min ), max ),
                        fminf( fmaxf( v.y, min ), max ) }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_clamp( msh_vec3_t v, float min, float max )
{
  if ( min > max )
  {
    return v;
  }
  return (msh_vec3_t ){{ fminf( fmaxf( v.x, min ), max ),
                         fminf( fmaxf( v.y, min ), max ),
                         fminf( fmaxf( v.z, min ), max ) }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_clamp( msh_vec4_t v, float min, float max )
{
  if ( min > max )
  {
    return v;
  }
  return (msh_vec4_t){{ fminf( fmaxf( v.x, min ), max ),
                        fminf( fmaxf( v.y, min ), max ),
                        fminf( fmaxf( v.z, min ), max ),
                        fminf( fmaxf( v.w, min ), max ) }};
}

MSHVMDEF inline msh_vec2_t 
msh_vec2_invert( msh_vec2_t v )
{
  return (msh_vec2_t){{ -v.x, -v.y }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_invert( msh_vec3_t v )
{
  return (msh_vec3_t){{ -v.x, -v.y, -v.z }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_invert( msh_vec4_t v )
{
  return (msh_vec4_t){{ -v.x, -v.y, -v.z, -v.w }};
}

// Should i do it with fast_inverse_square root?
MSHVMDEF inline msh_vec2_t 
msh_vec2_normalize( msh_vec2_t v )
{
  float denom = 1.0f / sqrtf( v.x * v.x + v.y * v.y );
  return (msh_vec2_t){{ v.x * denom, v.y * denom }};
}

MSHVMDEF inline msh_vec3_t 
msh_vec3_normalize( msh_vec3_t v )
{
  float denom = 1.0f / sqrtf( v.x * v.x + v.y * v.y + v.z * v.z );
  return (msh_vec3_t){{ v.x * denom, v.y * denom, v.z * denom }};
}

MSHVMDEF inline msh_vec4_t 
msh_vec4_normalize( msh_vec4_t v )
{
  float denom = 1.0f / sqrtf( v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w );
  msh_vec4_t o = {{ v.x * denom, v.y * denom, v.z * denom, v.w * denom }};
  return o;
}


MSHVMDEF inline float 
msh_vec2_dot( msh_vec2_t a, msh_vec2_t b )
{
  return a.x * b.x + a.y * b.y; 
}

MSHVMDEF inline float 
msh_vec3_dot( msh_vec3_t a, msh_vec3_t b )
{
  return a.x * b.x + a.y * b.y + a.z * b.z; 
}

MSHVMDEF inline float 
msh_vec4_dot( msh_vec4_t a, msh_vec4_t b )
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; 
}

MSHVMDEF inline msh_vec3_t
msh_vec3_cross( msh_vec3_t a, msh_vec3_t b )
{
  return (msh_vec3_t){{ ( a.y * b.z - a.z * b.y ),
                        ( a.z * b.x - a.x * b.z ),
                        ( a.x * b.y - a.y * b.x ) }};
}

MSHVMDEF inline float 
msh_vec2_norm( msh_vec2_t v )
{
  return sqrtf( v.x * v.x + v.y * v.y ); 
}

MSHVMDEF inline float 
msh_vec3_norm( msh_vec3_t v )
{
  return sqrtf( v.x * v.x + v.y * v.y + v.z * v.z ); 
}

MSHVMDEF inline float 
msh_vec4_norm( msh_vec4_t v )
{
  return sqrtf( v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w ); 
}

MSHVMDEF inline float 
msh_vec2_norm_sq( msh_vec2_t v )
{
  return v.x * v.x + v.y * v.y; 
}

MSHVMDEF inline float 
msh_vec3_norm_sq( msh_vec3_t v )
{
  return v.x * v.x + v.y * v.y + v.z * v.z; 
}

MSHVMDEF inline float 
msh_vec4_norm_sq( msh_vec4_t v )
{
  return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w; 
}

MSHVMDEF inline int 
msh_vec2_equal( msh_vec2_t a, msh_vec2_t b )
{
  return (fabsf(a.x - b.x) <= MSH_FLT_EPSILON) && 
         (fabsf(a.y - b.y) <= MSH_FLT_EPSILON);
}

MSHVMDEF inline int 
msh_vec3_equal( msh_vec3_t a, msh_vec3_t b )
{
  return (fabsf(a.x - b.x) <= MSH_FLT_EPSILON) && 
         (fabsf(a.y - b.y) <= MSH_FLT_EPSILON) &&
         (fabsf(a.z - b.z) <= MSH_FLT_EPSILON);
}

MSHVMDEF inline int 
msh_vec4_equal( msh_vec4_t a, msh_vec4_t b )
{
  return (fabsf(a.x - b.x) <= MSH_FLT_EPSILON) && 
         (fabsf(a.y - b.y) <= MSH_FLT_EPSILON) &&
         (fabsf(a.z - b.z) <= MSH_FLT_EPSILON) &&
         (fabsf(a.w - b.w) <= MSH_FLT_EPSILON);
}

#ifdef __cplusplus

msh_vec2_t operator+( msh_vec2_t a, msh_vec2_t b )
{
  return msh_vec2_add( a, b );
} 

msh_vec3_t operator+( msh_vec3_t a, msh_vec3_t b )
{
  return msh_vec3_add( a, b );
} 

msh_vec4_t operator+( msh_vec4_t a, msh_vec4_t b )
{
  return msh_vec4_add( a, b );
} 

msh_vec2_t &operator+=( msh_vec2_t &a, msh_vec2_t &b )
{
  return (a = a + b);
} 

msh_vec3_t &operator+=( msh_vec3_t &a, msh_vec3_t &b )
{
  return (a = a + b);
} 

msh_vec4_t &operator+=( msh_vec4_t &a, msh_vec4_t &b )
{
  return (a = a + b);
} 

#endif

/*
 * =============================================================================
 *       MATRIX IMPELEMENTATION
 * =============================================================================
 */

MSHVMDEF inline msh_mat2_t 
msh_mat3_to_mat2( msh_mat3_t m )
{
  msh_mat2_t o;
  o.data[0] = m.data[0];
  o.data[1] = m.data[1];
  o.data[2] = m.data[3];
  o.data[3] = m.data[4];
  return o;
}

MSHVMDEF inline msh_mat2_t 
msh_mat4_to_mat2( msh_mat4_t m )
{
  msh_mat2_t o;
  o.data[0] = m.data[0];
  o.data[1] = m.data[1];
  o.data[2] = m.data[4];
  o.data[3] = m.data[5];
  return o;
}

MSHVMDEF inline msh_mat3_t 
msh_mat2_to_mat3( msh_mat2_t m )
{
  msh_mat3_t o;
  o.data[0] = m.data[0];
  o.data[1] = m.data[1];
  o.data[2] = 0.0f;
  o.data[3] = m.data[2];
  o.data[4] = m.data[3];
  o.data[5] = 0.0f;
  o.data[6] = 0.0f;
  o.data[7] = 0.0f;
  o.data[8] = 1.0f;
  return o;
}

MSHVMDEF inline msh_mat3_t 
msh_mat4_to_mat3( msh_mat4_t m )
{
  msh_mat3_t o;
  o.data[ 0] = m.data[ 0];
  o.data[ 1] = m.data[ 1];
  o.data[ 2] = m.data[ 2];
  o.data[ 3] = m.data[ 4];
  o.data[ 4] = m.data[ 5];
  o.data[ 5] = m.data[ 6];
  o.data[ 6] = m.data[ 8];
  o.data[ 7] = m.data[ 9];
  o.data[ 8] = m.data[10];
  return o;
}

MSHVMDEF inline msh_mat4_t 
msh_mat2_to_mat4( msh_mat2_t m )
{
  msh_mat4_t o;
  o.data[0]  = m.data[0];
  o.data[1]  = m.data[1];
  o.data[2]  = 0.0f;
  o.data[3]  = 0.0f;
  o.data[4]  = m.data[2];
  o.data[5]  = m.data[3];
  o.data[6]  = 0.0f;
  o.data[7]  = 0.0f;
  o.data[8]  = 0.0f;
  o.data[9]  = 0.0f;
  o.data[10] = 1.0f;
  o.data[11] = 0.0f;
  o.data[12] = 0.0f;
  o.data[13] = 0.0f;
  o.data[14] = 0.0f;
  o.data[15] = 1.0f;
  return o;
}

MSHVMDEF inline msh_mat4_t 
msh_mat3_to_mat4( msh_mat3_t m )
{
  msh_mat4_t o;
  o.data[ 0] = m.data[0];
  o.data[ 1] = m.data[1];
  o.data[ 2] = m.data[2];
  o.data[ 3] = 0.0f;
  o.data[ 4] = m.data[3];
  o.data[ 5] = m.data[4];
  o.data[ 6] = m.data[5];
  o.data[ 7] = 0.0f;
  o.data[ 8] = m.data[6];
  o.data[ 9] = m.data[7];
  o.data[10] = m.data[8];
  o.data[11] = 0.0f;
  o.data[12] = 0.0f;
  o.data[13] = 0.0f;
  o.data[14] = 0.0f;
  o.data[15] = 1.0f;
  return o;
}

MSHVMDEF inline msh_mat2_t
msh_mat2_add( msh_mat2_t a, msh_mat2_t b )
{
  msh_mat2_t o;
  o.data[0] = a.data[0] + b.data[0];
  o.data[1] = a.data[1] + b.data[1];
  o.data[2] = a.data[2] + b.data[2];
  o.data[3] = a.data[3] + b.data[3];
  return o;
}

MSHVMDEF inline msh_mat3_t
msh_mat3_add( msh_mat3_t a, msh_mat3_t b )
{
  msh_mat3_t o;
  o.data[0] = a.data[0] + b.data[0];
  o.data[1] = a.data[1] + b.data[1];
  o.data[2] = a.data[2] + b.data[2];
  o.data[3] = a.data[3] + b.data[3];
  o.data[4] = a.data[4] + b.data[4];
  o.data[5] = a.data[5] + b.data[5];
  o.data[6] = a.data[6] + b.data[6];
  o.data[7] = a.data[7] + b.data[7];
  o.data[8] = a.data[8] + b.data[8];
  return o;
}

MSHVMDEF inline msh_mat4_t
msh_mat4_add( msh_mat4_t a, msh_mat4_t b )
{
  msh_mat4_t o;
  o.data[ 0] = a.data[ 0] + b.data[ 0];
  o.data[ 1] = a.data[ 1] + b.data[ 1];
  o.data[ 2] = a.data[ 2] + b.data[ 2];
  o.data[ 3] = a.data[ 3] + b.data[ 3];
  o.data[ 4] = a.data[ 4] + b.data[ 4];
  o.data[ 5] = a.data[ 5] + b.data[ 5];
  o.data[ 6] = a.data[ 6] + b.data[ 6];
  o.data[ 7] = a.data[ 7] + b.data[ 7];
  o.data[ 8] = a.data[ 8] + b.data[ 8];
  o.data[ 9] = a.data[ 9] + b.data[ 9];
  o.data[10] = a.data[10] + b.data[10];
  o.data[11] = a.data[11] + b.data[11];
  o.data[12] = a.data[12] + b.data[12];
  o.data[13] = a.data[13] + b.data[13];
  o.data[14] = a.data[14] + b.data[14];
  o.data[15] = a.data[15] + b.data[15];
  return o;
}

MSHVMDEF inline msh_mat2_t
msh_mat2_scalar_add( msh_mat2_t m, float s )
{
  msh_mat2_t o;
  o.data[0] = m.data[0] + s;
  o.data[1] = m.data[1] + s;
  o.data[2] = m.data[2] + s;
  o.data[3] = m.data[3] + s;
  return o;
}

MSHVMDEF inline msh_mat3_t
msh_mat3_scalar_add( msh_mat3_t m, float s )
{
  msh_mat3_t o;
  o.data[0] = m.data[0] + s;
  o.data[1] = m.data[1] + s;
  o.data[2] = m.data[2] + s;
  o.data[3] = m.data[3] + s;
  o.data[4] = m.data[4] + s;
  o.data[5] = m.data[5] + s;
  o.data[6] = m.data[6] + s;
  o.data[7] = m.data[7] + s;
  o.data[8] = m.data[8] + s;
  return o;
}

MSHVMDEF inline msh_mat4_t
msh_mat4_scalar_add( msh_mat4_t m, float s )
{
  msh_mat4_t o;
  o.data[ 0] = m.data[ 0] + s;
  o.data[ 1] = m.data[ 1] + s;
  o.data[ 2] = m.data[ 2] + s;
  o.data[ 3] = m.data[ 3] + s;
  o.data[ 4] = m.data[ 4] + s;
  o.data[ 5] = m.data[ 5] + s;
  o.data[ 6] = m.data[ 6] + s;
  o.data[ 7] = m.data[ 7] + s;
  o.data[ 8] = m.data[ 8] + s;
  o.data[ 9] = m.data[ 9] + s;
  o.data[10] = m.data[10] + s;
  o.data[11] = m.data[11] + s;
  o.data[12] = m.data[12] + s;
  o.data[13] = m.data[13] + s;
  o.data[14] = m.data[14] + s;
  o.data[15] = m.data[15] + s;
  return o;
}


MSHVMDEF inline msh_mat2_t
msh_mat2_sub( msh_mat2_t a, msh_mat2_t b )
{
  msh_mat2_t o;
  o.data[0] = a.data[0] - b.data[0];
  o.data[1] = a.data[1] - b.data[1];
  o.data[2] = a.data[2] - b.data[2];
  o.data[3] = a.data[3] - b.data[3];
  return o;
}

MSHVMDEF inline msh_mat3_t
msh_mat3_sub( msh_mat3_t a, msh_mat3_t b )
{
  msh_mat3_t o;
  o.data[0] = a.data[0] - b.data[0];
  o.data[1] = a.data[1] - b.data[1];
  o.data[2] = a.data[2] - b.data[2];
  o.data[3] = a.data[3] - b.data[3];
  o.data[4] = a.data[4] - b.data[4];
  o.data[5] = a.data[5] - b.data[5];
  o.data[6] = a.data[6] - b.data[6];
  o.data[7] = a.data[7] - b.data[7];
  o.data[8] = a.data[8] - b.data[8];
  return o;
}

MSHVMDEF inline msh_mat4_t
msh_mat4_sub( msh_mat4_t a, msh_mat4_t b )
{
  msh_mat4_t o;
  o.data[ 0] = a.data[ 0] - b.data[ 0];
  o.data[ 1] = a.data[ 1] - b.data[ 1];
  o.data[ 2] = a.data[ 2] - b.data[ 2];
  o.data[ 3] = a.data[ 3] - b.data[ 3];
  o.data[ 4] = a.data[ 4] - b.data[ 4];
  o.data[ 5] = a.data[ 5] - b.data[ 5];
  o.data[ 6] = a.data[ 6] - b.data[ 6];
  o.data[ 7] = a.data[ 7] - b.data[ 7];
  o.data[ 8] = a.data[ 8] - b.data[ 8];
  o.data[ 9] = a.data[ 9] - b.data[ 9];
  o.data[10] = a.data[10] - b.data[10];
  o.data[11] = a.data[11] - b.data[11];
  o.data[12] = a.data[12] - b.data[12];
  o.data[13] = a.data[13] - b.data[13];
  o.data[14] = a.data[14] - b.data[14];
  o.data[15] = a.data[15] - b.data[15];
  return o;
}

MSHVMDEF inline msh_mat2_t
msh_mat2_scalar_sub( msh_mat2_t m, float s )
{
  msh_mat2_t o;
  o.data[0] = m.data[0] - s;
  o.data[1] = m.data[1] - s;
  o.data[2] = m.data[2] - s;
  o.data[3] = m.data[3] - s;
  return o;
}

MSHVMDEF inline msh_mat3_t
msh_mat3_scalar_sub( msh_mat3_t m, float s )
{
  msh_mat3_t o;
  o.data[0] = m.data[0] - s;
  o.data[1] = m.data[1] - s;
  o.data[2] = m.data[2] - s;
  o.data[3] = m.data[3] - s;
  o.data[4] = m.data[4] - s;
  o.data[5] = m.data[5] - s;
  o.data[6] = m.data[6] - s;
  o.data[7] = m.data[7] - s;
  o.data[8] = m.data[8] - s;
  return o;
}

MSHVMDEF inline msh_mat4_t
msh_mat4_scalar_sub( msh_mat4_t m, float s )
{
  msh_mat4_t o;
  o.data[ 0] = m.data[ 0] - s;
  o.data[ 1] = m.data[ 1] - s;
  o.data[ 2] = m.data[ 2] - s;
  o.data[ 3] = m.data[ 3] - s;
  o.data[ 4] = m.data[ 4] - s;
  o.data[ 5] = m.data[ 5] - s;
  o.data[ 6] = m.data[ 6] - s;
  o.data[ 7] = m.data[ 7] - s;
  o.data[ 8] = m.data[ 8] - s;
  o.data[ 9] = m.data[ 9] - s;
  o.data[10] = m.data[10] - s;
  o.data[11] = m.data[11] - s;
  o.data[12] = m.data[12] - s;
  o.data[13] = m.data[13] - s;
  o.data[14] = m.data[14] - s;
  o.data[15] = m.data[15] - s;
  return o;
}

MSHVMDEF inline msh_mat2_t
msh_mat2_mul( msh_mat2_t a, msh_mat2_t b )
{
  msh_mat2_t o;
  o.data[0] = b.data[0]*a.data[0] + b.data[1]*a.data[2];
  o.data[1] = b.data[0]*a.data[1] + b.data[1]*a.data[3];

  o.data[2] = b.data[2]*a.data[0] + b.data[3]*a.data[2];
  o.data[3] = b.data[2]*a.data[1] + b.data[3]*a.data[3];
  return o;
}

MSHVMDEF inline msh_mat3_t
msh_mat3_mul( msh_mat3_t a, msh_mat3_t b )
{
  msh_mat3_t o;
  o.data[0] = b.data[0]*a.data[0] + b.data[1]*a.data[3] + b.data[2]*a.data[6];
  o.data[1] = b.data[0]*a.data[1] + b.data[1]*a.data[4] + b.data[2]*a.data[7];
  o.data[2] = b.data[0]*a.data[2] + b.data[1]*a.data[5] + b.data[2]*a.data[8];

  o.data[3] = b.data[3]*a.data[0] + b.data[4]*a.data[3] + b.data[5]*a.data[6];
  o.data[4] = b.data[3]*a.data[1] + b.data[4]*a.data[4] + b.data[5]*a.data[7];
  o.data[5] = b.data[3]*a.data[2] + b.data[4]*a.data[5] + b.data[5]*a.data[8];

  o.data[6] = b.data[6]*a.data[0] + b.data[7]*a.data[3] + b.data[8]*a.data[6];
  o.data[7] = b.data[6]*a.data[1] + b.data[7]*a.data[4] + b.data[8]*a.data[7];
  o.data[8] = b.data[6]*a.data[2] + b.data[7]*a.data[5] + b.data[8]*a.data[8];
  return o;
}

MSHVMDEF inline msh_mat4_t
msh_mat4_mul( msh_mat4_t a, msh_mat4_t b )
{
  msh_mat4_t o;
  o.data[ 0] = b.data[ 0]*a.data[ 0] + b.data[ 1]*a.data[ 4] +
               b.data[ 2]*a.data[ 8] + b.data[ 3]*a.data[12];
  o.data[ 1] = b.data[ 0]*a.data[ 1] + b.data[ 1]*a.data[ 5] +
               b.data[ 2]*a.data[ 9] + b.data[ 3]*a.data[13];
  o.data[ 2] = b.data[ 0]*a.data[ 2] + b.data[ 1]*a.data[ 6] +
               b.data[ 2]*a.data[10] + b.data[ 3]*a.data[14];
  o.data[ 3] = b.data[ 0]*a.data[ 3] + b.data[ 1]*a.data[ 7] +
               b.data[ 2]*a.data[11] + b.data[ 3]*a.data[15];

  o.data[ 4] = b.data[ 4]*a.data[ 0] + b.data[ 5]*a.data[ 4] +
               b.data[ 6]*a.data[ 8] + b.data[ 7]*a.data[12];
  o.data[ 5] = b.data[ 4]*a.data[ 1] + b.data[ 5]*a.data[ 5] +
               b.data[ 6]*a.data[ 9] + b.data[ 7]*a.data[13];
  o.data[ 6] = b.data[ 4]*a.data[ 2] + b.data[ 5]*a.data[ 6] +
               b.data[ 6]*a.data[10] + b.data[ 7]*a.data[14];
  o.data[ 7] = b.data[ 4]*a.data[ 3] + b.data[ 5]*a.data[ 7] +
               b.data[ 6]*a.data[11] + b.data[ 7]*a.data[15];

  o.data[ 8] = b.data[ 8]*a.data[ 0] + b.data[ 9]*a.data[ 4] +
               b.data[10]*a.data[ 8] + b.data[11]*a.data[12];
  o.data[ 9] = b.data[ 8]*a.data[ 1] + b.data[ 9]*a.data[ 5] +
               b.data[10]*a.data[ 9] + b.data[11]*a.data[13];
  o.data[10] = b.data[ 8]*a.data[ 2] + b.data[ 9]*a.data[ 6] +
               b.data[10]*a.data[10] + b.data[11]*a.data[14];
  o.data[11] = b.data[ 8]*a.data[ 3] + b.data[ 9]*a.data[ 7] +
               b.data[10]*a.data[11] + b.data[11]*a.data[15];

  o.data[12] = b.data[12]*a.data[ 0] + b.data[13]*a.data[ 4] +
               b.data[14]*a.data[ 8] + b.data[15]*a.data[12];
  o.data[13] = b.data[12]*a.data[ 1] + b.data[13]*a.data[ 5] +
               b.data[14]*a.data[ 9] + b.data[15]*a.data[13];
  o.data[14] = b.data[12]*a.data[ 2] + b.data[13]*a.data[ 6] +
               b.data[14]*a.data[10] + b.data[15]*a.data[14];
  o.data[15] = b.data[12]*a.data[ 3] + b.data[13]*a.data[ 7] +
               b.data[14]*a.data[11] + b.data[15]*a.data[15];
  return o;
}


MSHVMDEF inline msh_mat2_t
msh_mat2_scalar_mul( msh_mat2_t m, float s )
{
  msh_mat2_t o;
  o.data[0] = m.data[0]*s;
  o.data[1] = m.data[1]*s;
  o.data[2] = m.data[2]*s;
  o.data[3] = m.data[3]*s;
  return o;
}

MSHVMDEF inline msh_mat3_t
msh_mat3_scalar_mul( msh_mat3_t m, float s )
{
  msh_mat3_t o;
  o.data[0] = m.data[0]*s;
  o.data[1] = m.data[1]*s;
  o.data[2] = m.data[2]*s;
  o.data[3] = m.data[3]*s;
  o.data[4] = m.data[4]*s;
  o.data[5] = m.data[5]*s;
  o.data[6] = m.data[6]*s;
  o.data[7] = m.data[7]*s;
  o.data[8] = m.data[8]*s;
  return o;
}

MSHVMDEF inline msh_mat4_t
msh_mat4_scalar_mul( msh_mat4_t m, float s )
{
  msh_mat4_t o;
  o.data[ 0] = m.data[ 0]*s;
  o.data[ 1] = m.data[ 1]*s;
  o.data[ 2] = m.data[ 2]*s;
  o.data[ 3] = m.data[ 3]*s;
  o.data[ 4] = m.data[ 4]*s;
  o.data[ 5] = m.data[ 5]*s;
  o.data[ 6] = m.data[ 6]*s;
  o.data[ 7] = m.data[ 7]*s;
  o.data[ 8] = m.data[ 8]*s;
  o.data[ 9] = m.data[ 9]*s;
  o.data[10] = m.data[10]*s;
  o.data[11] = m.data[11]*s;
  o.data[12] = m.data[12]*s;
  o.data[13] = m.data[13]*s;
  o.data[14] = m.data[14]*s;
  o.data[15] = m.data[15]*s;
  return o;
}

MSHVMDEF inline msh_vec2_t
msh_mat2_vec2_mul ( msh_mat2_t m, msh_vec2_t v )
{
  msh_vec2_t o;
  o.x = m.data[0]*v.x + m.data[2]*v.y;
  o.y = m.data[1]*v.x + m.data[3]*v.y;
  return o;
}

MSHVMDEF inline msh_vec3_t
msh_mat3_vec3_mul ( msh_mat3_t m, msh_vec3_t v )
{
  msh_vec3_t o;
  o.x = m.data[0]*v.x + m.data[3]*v.y + m.data[6]*v.z;
  o.y = m.data[1]*v.x + m.data[4]*v.y + m.data[7]*v.z;
  o.z = m.data[2]*v.x + m.data[5]*v.y + m.data[8]*v.z;
  return o;
}

MSHVMDEF inline msh_vec4_t
msh_mat4_vec4_mul ( msh_mat4_t m, msh_vec4_t v )
{
  msh_vec4_t o;
  o.x = m.data[0]*v.x + m.data[4]*v.y + m.data[ 8]*v.z + m.data[12]*v.w;
  o.y = m.data[1]*v.x + m.data[5]*v.y + m.data[ 9]*v.z + m.data[13]*v.w;
  o.z = m.data[2]*v.x + m.data[6]*v.y + m.data[10]*v.z + m.data[14]*v.w;
  o.w = m.data[3]*v.x + m.data[7]*v.y + m.data[11]*v.z + m.data[15]*v.w;
  return o;
}

MSHVMDEF inline msh_mat2_t
msh_mat2_scalar_div( msh_mat2_t m, float s )
{
  msh_mat2_t o;
  float denom = 1.0f / s;
  o.data[0] = m.data[0] * denom;
  o.data[1] = m.data[1] * denom;
  o.data[2] = m.data[2] * denom;
  o.data[3] = m.data[3] * denom;
  return o;
}

MSHVMDEF inline msh_mat3_t
msh_mat3_scalar_div( msh_mat3_t m, float s )
{
  msh_mat3_t o;
  float denom = 1.0f / s;
  o.data[0] = m.data[0] * denom;
  o.data[1] = m.data[1] * denom;
  o.data[2] = m.data[2] * denom;
  o.data[3] = m.data[3] * denom;
  o.data[4] = m.data[4] * denom;
  o.data[5] = m.data[5] * denom;
  o.data[6] = m.data[6] * denom;
  o.data[7] = m.data[7] * denom;
  o.data[8] = m.data[8] * denom;
  return o;
}

MSHVMDEF inline msh_mat4_t
msh_mat4_scalar_div( msh_mat4_t m, float s )
{
  msh_mat4_t o;
  float denom = 1.0f / s;
  o.data[ 0] = m.data[ 0] * denom;
  o.data[ 1] = m.data[ 1] * denom;
  o.data[ 2] = m.data[ 2] * denom;
  o.data[ 3] = m.data[ 3] * denom;
  o.data[ 4] = m.data[ 4] * denom;
  o.data[ 5] = m.data[ 5] * denom;
  o.data[ 6] = m.data[ 6] * denom;
  o.data[ 7] = m.data[ 7] * denom;
  o.data[ 8] = m.data[ 8] * denom;
  o.data[ 9] = m.data[ 9] * denom;
  o.data[10] = m.data[10] * denom;
  o.data[11] = m.data[11] * denom;
  o.data[12] = m.data[12] * denom;
  o.data[13] = m.data[13] * denom;
  o.data[14] = m.data[14] * denom;
  o.data[15] = m.data[15] * denom;
  return o;
}

MSHVMDEF inline float
msh_mat2_trace( msh_mat2_t m )
{
  return m.data[0] + m.data[2];
}

MSHVMDEF inline float
msh_mat3_trace( msh_mat3_t m )
{
  return m.data[0] + m.data[4] + m.data[8];
}

MSHVMDEF inline float
msh_mat4_trace( msh_mat4_t m )
{
  return m.data[0] + m.data[5] + m.data[10] + m.data[15];
}

MSHVMDEF inline float
msh_mat2_determinant( msh_mat2_t m )
{
  return m.data[0] * m.data[3] - m.data[2] * m.data[1];
}

MSHVMDEF inline float
msh_mat3_determinant( msh_mat3_t m )
{
  /* get required cofactors */
  float C[3];
  C[0] = m.data[4] * m.data[8] - m.data[5] * m.data[7];
  C[1] = m.data[5] * m.data[6] - m.data[3] * m.data[8]; /* negated */
  C[2] = m.data[3] * m.data[7] - m.data[4] * m.data[6];

  return m.data[0] * C[0] + m.data[1] * C[1] + m.data[2] * C[2];
}

MSHVMDEF inline float
msh_mat4_determinant( msh_mat4_t m )
{
  float C[4];
  float coeffs[6];

  /* coeffs are determinants of 2x2 matrices */
  coeffs[0] = m.data[10] * m.data[15] - m.data[14] * m.data[11];
  coeffs[1] = m.data[ 6] * m.data[11] - m.data[10] * m.data[ 7];
  coeffs[2] = m.data[ 2] * m.data[ 7] - m.data[ 6] * m.data[ 3];
  coeffs[3] = m.data[ 6] * m.data[15] - m.data[14] * m.data[ 7];
  coeffs[4] = m.data[ 2] * m.data[11] - m.data[10] * m.data[ 3];
  coeffs[5] = m.data[ 2] * m.data[15] - m.data[14] * m.data[ 3];

  /* Cofactor matrix */
  /*00*/C[0] = m.data[ 5] * coeffs[0] -
               m.data[ 9] * coeffs[3] +
               m.data[13] * coeffs[1];
  /*01*/C[1] = m.data[ 9] * coeffs[5] -
               m.data[ 1] * coeffs[0] -
               m.data[13] * coeffs[4]; /* negated */
  /*02*/C[2] = m.data[ 1] * coeffs[3] -
               m.data[ 5] * coeffs[5] +
               m.data[13] * coeffs[2];
  /*03*/C[3] = m.data[ 5] * coeffs[4] -
               m.data[ 9] * coeffs[2] -
               m.data[ 1] * coeffs[1]; /* negated */

  /* determinant */
  float det = m.data[0] * C[0] + m.data[4]  * C[1] + 
              m.data[8] * C[2] + m.data[12] * C[3];
  return det;
}

MSHVMDEF inline float 
msh_mat2_frobenius_norm( msh_mat2_t m )
{
  return sqrtf( m.data[0] * m.data[0] +
                m.data[1] * m.data[1] + 
                m.data[2] * m.data[2] +
                m.data[3] * m.data[3] );
}

MSHVMDEF inline float 
msh_mat3_frobenius_norm( msh_mat3_t m )
{
  return sqrtf( m.data[0] * m.data[0] +
                m.data[1] * m.data[1] + 
                m.data[2] * m.data[2] +
                m.data[3] * m.data[3] +
                m.data[4] * m.data[4] +
                m.data[5] * m.data[5] + 
                m.data[6] * m.data[6] +
                m.data[7] * m.data[7] +
                m.data[8] * m.data[8] );
}

MSHVMDEF inline float 
msh_mat4_frobenius_norm( msh_mat4_t m )
{
  return sqrtf( m.data[ 0] * m.data[ 0] +
                m.data[ 1] * m.data[ 1] + 
                m.data[ 2] * m.data[ 2] +
                m.data[ 3] * m.data[ 3] +
                m.data[ 4] * m.data[ 4] +
                m.data[ 5] * m.data[ 5] + 
                m.data[ 6] * m.data[ 6] +
                m.data[ 7] * m.data[ 7] +
                m.data[ 8] * m.data[ 8] +
                m.data[ 9] * m.data[ 9] + 
                m.data[10] * m.data[10] +
                m.data[11] * m.data[11] +
                m.data[12] * m.data[12] +
                m.data[13] * m.data[13] + 
                m.data[14] * m.data[14] +
                m.data[15] * m.data[15] );
}

MSHVMDEF inline msh_mat2_t 
msh_mat2_inverse( msh_mat2_t m )
{
  float denom = 1.0f / (m.data[0] * m.data[3] - m.data[2] * m.data[1]);
  msh_mat2_t mi;
  mi.data[0] =  m.data[3] * denom;
  mi.data[1] = -m.data[1] * denom;
  mi.data[2] = -m.data[2] * denom;
  mi.data[3] =  m.data[0] * denom;
  return mi;
}

MSHVMDEF inline msh_mat3_t 
msh_mat3_inverse( msh_mat3_t m )
{
  /* To calculate inverse :
         1. Transpose M
         2. Calculate cofactor matrix C
         3. Caluclate determinant of M
         4. Inverse is given as (1/det) * C
  
     Access cheat sheat for transpose matrix:
         original indices
          0  1  2
          3  4  5
          6  7  8

         transposed indices
          0  3  6
          1  4  7
          2  5  8
  */

  /* Calulate cofactor matrix */
  float C[9];
  C[0] = m.data[4] * m.data[8] - m.data[7] * m.data[5];
  C[1] = m.data[7] * m.data[2] - m.data[1] * m.data[8]; /*negated*/
  C[2] = m.data[1] * m.data[5] - m.data[4] * m.data[2];
  C[3] = m.data[6] * m.data[5] - m.data[3] * m.data[8]; /*negated*/
  C[4] = m.data[0] * m.data[8] - m.data[6] * m.data[2];
  C[5] = m.data[3] * m.data[2] - m.data[0] * m.data[5]; /*negated*/
  C[6] = m.data[3] * m.data[7] - m.data[6] * m.data[4];
  C[7] = m.data[6] * m.data[1] - m.data[0] * m.data[7]; /*negated*/
  C[8] = m.data[0] * m.data[4] - m.data[3] * m.data[1];

  /* determinant */
  float det = m.data[0] * C[0] + m.data[3] * C[1] + m.data[6] * C[2];
  float denom = 1.0f / det;

  /* calculate inverse */
  msh_mat3_t mi;
  mi.data[0] = denom * C[0];
  mi.data[1] = denom * C[1];
  mi.data[2] = denom * C[2];
  mi.data[3] = denom * C[3];
  mi.data[4] = denom * C[4];
  mi.data[5] = denom * C[5];
  mi.data[6] = denom * C[6];
  mi.data[7] = denom * C[7];
  mi.data[8] = denom * C[8];
  return mi;
}

MSHVMDEF inline msh_mat4_t 
msh_mat4_inverse( msh_mat4_t m )
{
  /* Inverse using cramers rule
         1. Transpose  M
         2. Calculate cofactor matrix C
         3. Caluclate determinant of M
         4. Inverse is given as (1/det) * C

     Access cheat sheat:
         original indices
          0  1  2  3
          4  5  6  7
          8  9 10 11
         12 13 14 15

         transposed indices
          0  4  8 12
          1  5  9 13
          2  6 10 14
          3  7 11 15                              */

  /* Calulate cofactor matrix */
  float C[16];
  float dets[6];

  /* First 8 */
  /* dets are determinants of 2x2 matrices */
  dets[0] = m.data[10] * m.data[15] - m.data[14] * m.data[11];
  dets[1] = m.data[ 6] * m.data[11] - m.data[10] * m.data[ 7];
  dets[2] = m.data[ 2] * m.data[ 7] - m.data[ 6] * m.data[ 3];
  dets[3] = m.data[ 6] * m.data[15] - m.data[14] * m.data[ 7];
  dets[4] = m.data[ 2] * m.data[11] - m.data[10] * m.data[ 3];
  dets[5] = m.data[ 2] * m.data[15] - m.data[14] * m.data[ 3];

  /* Cofactor matrix */
  /*00*/C[0] = m.data[5]*dets[0] - m.data[9]*dets[3] + m.data[13]*dets[1];
  /*01*/C[1] = m.data[9]*dets[5] - m.data[1]*dets[0] - m.data[13]*dets[4]; /* negated */
  /*02*/C[2] = m.data[1]*dets[3] - m.data[5]*dets[5] + m.data[13]*dets[2];
  /*03*/C[3] = m.data[5]*dets[4] - m.data[9]*dets[2] - m.data[ 1]*dets[1]; /* negated */

  /*10*/C[4] = m.data[8]*dets[3] - m.data[4]*dets[0] - m.data[12]*dets[1]; /* negated */
  /*11*/C[5] = m.data[0]*dets[0] - m.data[8]*dets[5] + m.data[12]*dets[4];
  /*12*/C[6] = m.data[4]*dets[5] - m.data[0]*dets[3] - m.data[12]*dets[2]; /* negated */
  /*13*/C[7] = m.data[0]*dets[1] - m.data[4]*dets[4] + m.data[ 8]*dets[2];

  /*Second 8 */

  /* dets are determinants of 2x2 matrices */
  dets[0] = m.data[ 8]*m.data[13] - m.data[12]*m.data[ 9];
  dets[1] = m.data[ 4]*m.data[ 9] - m.data[ 8]*m.data[ 5];
  dets[2] = m.data[ 0]*m.data[ 5] - m.data[ 4]*m.data[ 1];
  dets[3] = m.data[ 4]*m.data[13] - m.data[12]*m.data[ 5];
  dets[4] = m.data[ 0]*m.data[ 9] - m.data[ 8]*m.data[ 1];
  dets[5] = m.data[ 0]*m.data[13] - m.data[12]*m.data[ 1];

  /* actual coefficient matrix */
  /*20*/C[ 8] = m.data[ 7]*dets[0] - m.data[11]*dets[3] + m.data[15]*dets[1];
  /*21*/C[ 9] = m.data[11]*dets[5] - m.data[ 3]*dets[0] - m.data[15]*dets[4]; /* negated */
  /*22*/C[10] = m.data[ 3]*dets[3] - m.data[ 7]*dets[5] + m.data[15]*dets[2];
  /*23*/C[11] = m.data[ 7]*dets[4] - m.data[ 3]*dets[1] - m.data[11]*dets[2]; /* negated */

  /*30*/C[12] = m.data[10]*dets[3] - m.data[ 6]*dets[0] - m.data[14]*dets[1]; /* negated */
  /*31*/C[13] = m.data[ 2]*dets[0] - m.data[10]*dets[5] + m.data[14]*dets[4];
  /*32*/C[14] = m.data[ 6]*dets[5] - m.data[ 2]*dets[3] - m.data[14]*dets[2]; /* negated */
  /*33*/C[15] = m.data[ 2]*dets[1] - m.data[ 6]*dets[4] + m.data[10]*dets[2];

  /* determinant */
  float det = m.data[0]*C[0] + m.data[4]*C[1] + 
              m.data[8]*C[2] + m.data[12]*C[3];
  float denom = 1.0f / det;

  /* calculate inverse */
  msh_mat4_t mi;
  mi.data[ 0] = denom*C[ 0];
  mi.data[ 1] = denom*C[ 1];
  mi.data[ 2] = denom*C[ 2];
  mi.data[ 3] = denom*C[ 3];
  mi.data[ 4] = denom*C[ 4];
  mi.data[ 5] = denom*C[ 5];
  mi.data[ 6] = denom*C[ 6];
  mi.data[ 7] = denom*C[ 7];
  mi.data[ 8] = denom*C[ 8];
  mi.data[ 9] = denom*C[ 9];
  mi.data[10] = denom*C[10];
  mi.data[11] = denom*C[11];
  mi.data[12] = denom*C[12];
  mi.data[13] = denom*C[13];
  mi.data[14] = denom*C[14];
  mi.data[15] = denom*C[15];
  return mi;
}

MSHVMDEF inline msh_mat2_t 
msh_mat2_transpose( msh_mat2_t m )
{
  msh_mat2_t mt;
  mt.data[0] = m.data[0];
  mt.data[1] = m.data[2];
  mt.data[2] = m.data[1];
  mt.data[3] = m.data[3];
  return mt;
}

MSHVMDEF inline msh_mat3_t 
msh_mat3_transpose( msh_mat3_t m )
{
  msh_mat3_t mt;
  mt.data[0] = m.data[0];
  mt.data[1] = m.data[3];
  mt.data[2] = m.data[6];
  mt.data[3] = m.data[1];
  mt.data[4] = m.data[4];
  mt.data[5] = m.data[7];
  mt.data[6] = m.data[2];
  mt.data[7] = m.data[5];
  mt.data[8] = m.data[8];
  return mt;
}

MSHVMDEF inline msh_mat4_t 
msh_mat4_transpose( msh_mat4_t m )
{
  msh_mat4_t mt;
  mt.data[ 0] = m.data[ 0];
  mt.data[ 1] = m.data[ 4];
  mt.data[ 2] = m.data[ 8];
  mt.data[ 3] = m.data[12];
  mt.data[ 4] = m.data[ 1];
  mt.data[ 5] = m.data[ 5];
  mt.data[ 6] = m.data[ 9];
  mt.data[ 7] = m.data[13];
  mt.data[ 8] = m.data[ 2];
  mt.data[ 9] = m.data[ 6];
  mt.data[10] = m.data[10];
  mt.data[11] = m.data[14];
  mt.data[12] = m.data[ 3];
  mt.data[13] = m.data[ 7];
  mt.data[14] = m.data[11];
  mt.data[15] = m.data[15];
  return mt;
}

MSHVMDEF inline msh_mat4_t 
msh_look_at( msh_vec3_t eye, 
             msh_vec3_t center, 
             msh_vec3_t up )
{
  msh_vec3_t z = msh_vec3_normalize( msh_vec3_sub(eye, center) );
  msh_vec3_t x = msh_vec3_normalize( msh_vec3_cross( up, z ) );
  msh_vec3_t y = msh_vec3_normalize( msh_vec3_cross( z, x ) );

  msh_mat4_t o = {{   x.x,                   y.x,                  z.x, 0.0f,
                      x.y,                   y.y,                  z.y, 0.0f,
                      x.z,                   y.z,                  z.z, 0.0f,
      -msh_vec3_dot(eye,x), -msh_vec3_dot(eye,y), -msh_vec3_dot(eye,z), 1.0f }};
  return o;
}

MSHVMDEF inline msh_mat4_t 
msh_frustum( float left,   float right, 
             float bottom, float top, 
             float z_near, float z_far )
{
  float x_diff = right - left;
  float y_diff = top - bottom;
  float z_diff = z_far - z_near;
  float a      = (right + left) / x_diff;
  float b      = (top + bottom) / y_diff;
  float c      = -(z_far + z_near ) / z_diff;
  float d      = -(2.0f * z_far * z_near ) / z_diff;

  msh_mat4_t o = {{ (2.0f*z_near)/x_diff,                 0.0f, 0.0f,  0.0f,
                                    0.0f, (2.0f*z_near)/y_diff, 0.0f,  0.0f,
                                       a,                    b,    c, -1.0f,
                                    0.0f,                 0.0f,    d,  0.0f }};
  return o;
}

MSHVMDEF inline msh_mat4_t  
msh_perspective( float fovy, 
                 float aspect, 
                 float z_near, 
                 float z_far)
{
  float xmin, xmax, ymin, ymax;

  ymax = z_near * tanf( fovy * 0.5f );
  ymin = -ymax;

  xmin = ymin * aspect;
  xmax = ymax * aspect;

  return msh_frustum( xmin, xmax, ymin, ymax, z_near, z_far );
}

MSHVMDEF inline msh_mat4_t 
msh_ortho( float left,   float right, 
           float bottom, float top, 
           float z_near, float z_far )
{
  float x_diff = right - left;
  float y_diff = top - bottom;
  float z_diff = z_far - z_near;
  float tx     = -( right + left ) / x_diff;
  float ty     = -( top + bottom ) / y_diff;
  float tz     = -( z_near + z_far ) / z_diff;

  msh_mat4_t o = {{ 2.0f / x_diff,          0.0f,           0.0f, 0.0f,
                             0.0f, 2.0f / y_diff,           0.0f, 0.0f,
                             0.0f,          0.0f, -2.0f / z_diff, 0.0f,
                               tx,            ty,             tz, 1.0f }};
  return o;
}

MSHVMDEF inline msh_vec3_t
msh_project ( msh_vec4_t obj,     msh_mat4_t modelview,
              msh_mat4_t project, msh_vec4_t viewport )
{
  msh_vec4_t tmp = msh_mat4_vec4_mul(msh_mat4_mul(project, modelview), obj);
  tmp = msh_vec4_scalar_div(tmp, tmp.w);

  msh_vec3_t win;
  win.x = viewport.x + (viewport.z * (tmp.x + 1.0f)) / 2.0f;
  win.y = viewport.y + (viewport.w * (tmp.y + 1.0f)) / 2.0f;
  win.z = (tmp.z + 1.0f) / 2.0f;

  return win;
}

MSHVMDEF inline msh_vec4_t
msh_unproject ( msh_vec3_t win,     msh_mat4_t modelview,
            msh_mat4_t project, msh_vec4_t viewport )
{
  msh_mat4_t inv_pm = msh_mat4_inverse( msh_mat4_mul(project, modelview));
  msh_vec4_t tmp;
  tmp.x = (2.0f * ( win.x - viewport.x )) / viewport.z - 1.0f;
  tmp.y = (2.0f * ( win.y - viewport.y )) / viewport.w - 1.0f;
  tmp.z = 2.0f * win.z - 1.0f;
  tmp.w = 1.0f;

  msh_vec4_t obj = msh_mat4_vec4_mul( inv_pm, tmp );
  obj = msh_vec4_scalar_div( obj, obj.w );
  return obj;
}

MSHVMDEF inline msh_mat4_t
msh_translate( msh_mat4_t m, msh_vec3_t t )
{
  msh_mat4_t result = m;
  result.col[3] = msh_vec4_add( 
                    msh_vec4_add( msh_vec4_scalar_mul( m.col[0], t.x ), 
                                  msh_vec4_scalar_mul( m.col[1], t.y )), 
                    msh_vec4_add( msh_vec4_scalar_mul( m.col[2], t.z ), 
                                  m.col[3]) );
  return result;
}

MSHVMDEF inline msh_mat4_t
msh_scale( msh_mat4_t m, msh_vec3_t s )
{
  msh_mat4_t result = m;
  result.col[0]=msh_vec4_scalar_mul( result.col[0], s.x );
  result.col[1]=msh_vec4_scalar_mul( result.col[1], s.y );
  result.col[2]=msh_vec4_scalar_mul( result.col[2], s.z );
  return result;
}

/* derivation : 
 * http://www.euclideanspace.com/matrixhs/geometry/rotations/conversions/angleToMatrix/ 
 */
MSHVMDEF inline msh_mat4_t
msh_rotate( msh_mat4_t m, float angle, msh_vec3_t v )
{
  float c = cosf( angle );
  float s = sinf( angle );
  float t = 1.0 - c;

  msh_vec3_t axis = msh_vec3_normalize( v );

  msh_mat4_t rotate = msh_mat4_zeros();
  rotate.data[ 0] = c + axis.x * axis.x * t;
  rotate.data[ 5] = c + axis.y * axis.y * t;
  rotate.data[10] = c + axis.z * axis.z * t;

  float tmp_1 = axis.x * axis.y * t;
  float tmp_2 = axis.z * s;

  rotate.data[1] = tmp_1 + tmp_2;
  rotate.data[4] = tmp_1 - tmp_2;

  tmp_1 = axis.x * axis.z * t;
  tmp_2 = axis.y * s;

  rotate.data[2] = tmp_1 - tmp_2;
  rotate.data[8] = tmp_1 + tmp_2;

  tmp_1 = axis.y * axis.z * t;
  tmp_2 = axis.x * s;

  rotate.data[6] = tmp_1 + tmp_2;
  rotate.data[9] = tmp_1 - tmp_2;

  msh_mat4_t result = m;
  result.col[0]=msh_vec4_add(msh_vec4_scalar_mul(m.col[0], rotate.data[0]), 
                 msh_vec4_add(msh_vec4_scalar_mul(m.col[1], rotate.data[1]), 
                              msh_vec4_scalar_mul(m.col[2], rotate.data[2])));
  
  result.col[1]=msh_vec4_add(msh_vec4_scalar_mul(m.col[0], rotate.data[4]), 
                 msh_vec4_add(msh_vec4_scalar_mul(m.col[1], rotate.data[5]), 
                              msh_vec4_scalar_mul(m.col[2], rotate.data[6])));
  
  result.col[2]=msh_vec4_add(msh_vec4_scalar_mul(m.col[0], rotate.data[8]), 
                 msh_vec4_add(msh_vec4_scalar_mul(m.col[1], rotate.data[9]), 
                              msh_vec4_scalar_mul(m.col[2], rotate.data[10])));
  result.col[3]=m.col[3];
  return result;
}

/*
 * =============================================================================
 *       QUATERNION IMPLEMENTATION
 * =============================================================================
 */

MSHVMDEF inline msh_quat_t
msh_quat_from_axis_angle( msh_vec3_t axis, float angle )
{
  float a = angle * 0.5;
  float s = sinf(a);
  return (msh_quat_t){{ axis.x * s, axis.y * s, axis.z * s, cosf(a)}};
}

MSHVMDEF inline msh_quat_t 
msh_quat_add( msh_quat_t a, msh_quat_t b )
{
  return (msh_quat_t){{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w }};
}

MSHVMDEF inline msh_quat_t 
msh_quat_scalar_add( msh_quat_t v, float s )
{
  msh_quat_t o = v;
  o.re += s;
  return o;
}

MSHVMDEF inline msh_quat_t 
msh_quat_sub( msh_quat_t a, msh_quat_t b )
{
  return (msh_quat_t){{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w }};
}

MSHVMDEF inline msh_quat_t 
msh_quat_scalar_sub( msh_quat_t v, float s )
{
  msh_quat_t o = v;
  o.re -= s;
  return o;
}

MSHVMDEF inline msh_quat_t 
msh_quat_mul( msh_quat_t a, msh_quat_t b )
{
  /*
  NOTE: This is implementation of mathematically easier to express formulation
  of multiplication, however it will be slower than explicit multiplications of
  the components 
  msh_quat_t o;
  o.re = a.re * b.re - msh_vec3_dot( a.im, b.im );
  o.im = msh_vec3_add( msh_vec3_cross( a.im, b.im ), 
         msh_vec3_add( msh_vec3_scalar_mul( b.im, a.re), 
                       msh_vec3_scalar_mul( a.im, b.re )));
  return o;
  */
  return (msh_quat_t){{ a.w*b.x + b.w*a.x + a.y*b.z - b.y*a.z,
                        a.w*b.y + b.w*a.y - a.x*b.z + b.x*a.z,
                        a.w*b.z + b.w*a.z + a.x*a.y - b.x*a.y,
                        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z }};
}

MSHVMDEF inline msh_quat_t 
msh_quat_scalar_mul( msh_quat_t v, float s )
{
  msh_quat_t o;
  o.im = msh_vec3_scalar_mul(v.im, s);
  o.re = v.re * s;
  return o;
}

MSHVMDEF inline msh_quat_t 
msh_quat_div( msh_quat_t a, msh_quat_t b )
{
  return msh_quat_mul( a, msh_quat_inverse(b) );
}

MSHVMDEF inline msh_quat_t 
msh_quat_scalar_div( msh_quat_t v, float s )
{
  float denom = 1.0f / s;
  return (msh_quat_t){{ v.x * denom, v.y * denom, v.z * denom, v.w * denom }};
}

float 
msh_quat_dot( msh_quat_t a,  msh_quat_t b )
{
  return ( a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w );
}

float 
msh_quat_norm( msh_quat_t q )
{
  return sqrtf( q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w );
}

float 
msh_quat_norm_sq( msh_quat_t q )
{
  return ( q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w );
}

MSHVMDEF inline msh_quat_t 
msh_quat_normalize( msh_quat_t q )
{
  float denom = 1.0f / msh_quat_norm( q );
  msh_quat_t o;
  o.im = msh_vec3_scalar_mul( q.im, denom );
  o.re = q.re * denom;
  return o;
}

MSHVMDEF inline msh_quat_t 
msh_quat_conjugate( msh_quat_t q )
{
  msh_quat_t o;
  o.im = (msh_vec3_t){{ -q.x, -q.y, -q.z }};
  o.re = q.re;
  return o;
}

MSHVMDEF inline msh_quat_t 
msh_quat_inverse( msh_quat_t q )
{
  msh_quat_t o;
  float denom = 1.0f / msh_quat_norm_sq( q );
  o.x = -q.x * denom;
  o.y = -q.y * denom;
  o.z = -q.z * denom;
  o.w *= denom;
  return o;
}

MSHVMDEF inline msh_quat_t 
msh_quat_lerp( msh_quat_t q, 
               msh_quat_t r, 
               float t )
{
  msh_quat_t o;
  o.x = q.x * (1.0f-t) + r.x * t;
  o.y = q.y * (1.0f-t) + r.y * t;
  o.z = q.z * (1.0f-t) + r.z * t;
  o.w = q.w * (1.0f-t) + r.w * t;
  return o;
}

MSHVMDEF inline msh_quat_t 
msh_quat_slerp( msh_quat_t q, 
                msh_quat_t r, 
                float t )
{
  float a = acosf( msh_quat_dot(q, r) );
  msh_quat_t o;
  if ( fabs( a ) > 1e-6 )
  {
    o = msh_quat_add(msh_quat_scalar_mul(q, sin(a * (1.0-t)) / sin(a) ), 
                     msh_quat_scalar_mul(r, sin(a * t) / sin(a) ) );
  }
  else
  {
    o = msh_quat_lerp( q, r, t );
  }
  return o;
}

MSHVMDEF inline msh_quat_t
msh_quat_from_angle_axis( msh_vec3_t axis, float angle )
{
    float a = angle * 0.5f;
    float s = sinf(a);
    return (msh_quat_t){{ axis.x * s, axis.y * s, axis.z * s, cosf(a) }};
}

MSHVMDEF inline msh_mat3_t 
msh_quat_to_mat3( msh_quat_t q )
{
  msh_mat3_t o;
  float xx = q.x * q.x ;
  float xy = q.x * q.y ;
  float xz = q.x * q.z ;
  float xw = q.x * q.w ;

  float yy = q.y * q.y ;
  float yz = q.y * q.z ;
  float yw = q.y * q.w ;

  float zz = q.z * q.z ;
  float zw = q.z * q.w ;

  o.data[0] = 1.0f - 2.0f * (yy +  zz);
  o.data[1] = 2.0f * (xy + zw);
  o.data[2] = 2.0f * (xz - yw);

  o.data[3] = 2.0f * (xy - zw);
  o.data[4] = 1.0f - 2.0f * (xx +  zz);
  o.data[5] = 2.0f * (yz + xw);

  o.data[6] = 2.0f * (xz + yw);
  o.data[7] = 2.0f * (yz - xw);
  o.data[8] = 1.0f - 2.0f * (xx +  yy);
  return o;
}

MSHVMDEF inline msh_mat4_t 
msh_quat_to_mat4( msh_quat_t q )
{
  return msh_mat3_to_mat4( msh_quat_to_mat3(q) );
}
 

MSHVMDEF inline msh_quat_t
msh_mat3_to_quat( msh_mat3_t m )
{
  float tr = m.data[0] + m.data[4] + m.data[8];
  msh_quat_t q;
  if ( fabs(tr) > MSH_FLT_EPSILON )
  {
    float s = sqrtf( tr + 1.0f ) * 2.0f;
    q.w = 0.25f * s;
    q.x = ( m.data[5] - m.data[7] ) / s;
    q.y = ( m.data[6] - m.data[2] ) / s;
    q.z = ( m.data[1] - m.data[3] ) / s;
  }
  else if ( ( m.data[0] > m.data[4]) && (m.data[0] > m.data[8] ) )
  {
    float s = sqrtf( 1.0f + m.data[0] - m.data[4] - m.data[8]) * 2.0f;
    q.w = ( m.data[5] - m.data[7] ) / s;
    q.x = 0.25f * s;
    q.y = ( m.data[3] + m.data[1] ) / s;
    q.z = ( m.data[6] + m.data[2] ) / s;
  }
  else if ( m.data[4] > m.data[8] )
  {
    float s = sqrtf( 1.0f + m.data[4] - m.data[0] - m.data[8]) * 2.0f;
    q.w = ( m.data[6] - m.data[2] ) / s;
    q.x = ( m.data[3] + m.data[1] ) / s;
    q.y = 0.25f * s;
    q.z = ( m.data[7] + m.data[5] ) / s;
  } 
  else
  {
    float s = sqrtf( 1.0f + m.data[8] - m.data[0] - m.data[4] ) * 2.0f;
    q.w = ( m.data[1] - m.data[3] ) / s;
    q.x = ( m.data[6] + m.data[2] ) / s;
    q.y = ( m.data[7] + m.data[5] ) / s;
    q.z = 0.25f * s;
  }
  return q;
}

MSHVMDEF inline msh_quat_t 
msh_mat4_to_quat( msh_mat4_t m )
{
  return msh_mat3_to_quat( msh_mat4_to_mat3( m ) );
}

/*
 * =============================================================================
 *       DEBUG IMPLEMENTATION
 * =============================================================================
 */

MSHVMDEF inline void
msh_vec2_fprint( msh_vec2_t v, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f\n", v.x, v.y );
}

MSHVMDEF inline void
msh_vec3_fprint( msh_vec3_t v, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f %10.5f\n", v.x, v.y, v.z );
}

MSHVMDEF inline void
msh_vec4_fprint( msh_vec4_t v, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f %10.5f %10.5f\n", 
                   v.x, v.y, v.z, v.w );
}

MSHVMDEF inline void
msh_quat_fprint( msh_quat_t v, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f %10.5f %10.5f\n", 
                   v.w, v.x, v.y, v.z );
}

MSHVMDEF inline void
msh_mat2_fprint( msh_mat2_t m, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f\n%10.5f %10.5f\n\n", 
                    m.data[0], m.data[2], 
                    m.data[1], m.data[3] );
}

MSHVMDEF inline void
msh_mat3_fprint( msh_mat3_t m, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f\n\n", 
                   m.data[0], m.data[3], m.data[6],
                   m.data[1], m.data[4], m.data[7],
                   m.data[2], m.data[5], m.data[8] );
}

MSHVMDEF inline void
msh_mat4_fprint( msh_mat4_t m, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f %10.5f\n\n",
                   m.data[0], m.data[4], m.data[8],  m.data[12],
                   m.data[1], m.data[5], m.data[9],  m.data[13],
                   m.data[2], m.data[6], m.data[10], m.data[14],
                   m.data[3], m.data[7], m.data[11], m.data[15] );
}

MSHVMDEF inline void
msh_vec2_print( msh_vec2_t v )
{
  msh_vec2_fprint( v, stdout );
}

MSHVMDEF inline void
msh_vec3_print( msh_vec3_t v )
{
  msh_vec3_fprint( v, stdout );
}

MSHVMDEF inline void
msh_vec4_print( msh_vec4_t v )
{
  msh_vec4_fprint( v, stdout );
}

MSHVMDEF inline void
msh_quat_print( msh_quat_t v )
{
  msh_quat_fprint( v, stdout );
}

MSHVMDEF inline void
msh_mat2_print( msh_mat2_t m )
{
  msh_mat2_fprint( m, stdout );
}

MSHVMDEF inline void
msh_mat3_print( msh_mat3_t m )
{
  msh_mat3_fprint( m, stdout );
}

MSHVMDEF inline void
msh_mat4_print( msh_mat4_t m )
{
  msh_mat4_fprint( m, stdout );
}

#endif /* MSH_VEC_MATH_IMPLEMENTATION */

