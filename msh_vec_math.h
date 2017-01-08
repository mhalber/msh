#ifndef MSH_VEC_MATH
#define MSH_VEC_MATH

/*
 * =============================================================================
 *       VECTORS
 * =============================================================================
 */

typedef union vec2
{
  float data[2];
  struct { float x; float y; };
  struct { float r; float g; };
  struct { float p; float q; };
} msh_vec2_t;

typedef union vec3
{
  float data[3];
  struct { float x; float y; float z; };
  struct { float r; float g; float b; };
  struct { float p; float q; float s; };
} msh_vec3_t;

typedef union vec4
{
  float data[4];
  struct { float x; float y; float z; float w; };
  struct { float r; float g; float b; float a; };
  struct { float p; float q; float s; float t; };
} msh_vec4_t;

typedef union quaternion
{
  struct { float x, y, z, w; };
  struct { msh_vec3_t im; float re; };
} msh_quat_t;

typedef msh_vec2_t msh_point2_t;
typedef msh_vec3_t msh_point3_t;

#define MSH_VEC2_ZEROS {{0, 0}}
#define MSH_VEC3_ZEROS {{0, 0, 0}}
#define MSH_VEC4_ZEROS {{0, 0, 0, 0}}

// NOTE: Probably will need to benchmark this... It really hangs on RVO
// NOTE: Handmade math -> passes and returns by value...
msh_vec2_t msh_vec2_add( const msh_vec2_t *a, const msh_vec2_t *b );
msh_vec3_t msh_vec3_add( const msh_vec3_t *a, const msh_vec3_t *b );
msh_vec4_t msh_vec4_add( const msh_vec4_t *a, const msh_vec4_t *b );

msh_vec2_t msh_vec2_sub( const msh_vec2_t *a, const msh_vec2_t *b );
msh_vec3_t msh_vec3_sub( const msh_vec3_t *a, const msh_vec3_t *b );
msh_vec4_t msh_vec4_sub( const msh_vec4_t *a, const msh_vec4_t *b );

msh_vec2_t msh_vec2_mul( const msh_vec2_t *a, const msh_vec2_t *b );
msh_vec3_t msh_vec3_mul( const msh_vec3_t *a, const msh_vec3_t *b );
msh_vec4_t msh_vec4_mul( const msh_vec4_t *a, const msh_vec4_t *b );

msh_vec2_t msh_vec2_div( const msh_vec2_t *a, const msh_vec2_t *b );
msh_vec3_t msh_vec3_div( const msh_vec3_t *a, const msh_vec3_t *b );
msh_vec4_t msh_vec4_div( const msh_vec4_t *a, const msh_vec4_t *b );

/*
 * =============================================================================
 *       MATRICES
 * =============================================================================
 */

typedef union mat2
{
  float data[4];
  msh_vec2_t cols[2];
} msh_mat2_t;

typedef union mat3
{
  float data[9];
  msh_vec3_t cols[3];
} msh_mat3_t;

typedef union mat4
{
  float data[16];
  msh_vec4_t cols[4];
} msh_mat4_t;

#define MSH_MAT2_ZEROS {{0, 0, 0, 0}}
#define MSH_MAT3_ZEROS {{0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define MSH_MAT4_ZEROS {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
#define MSH_MAT2_IDENTITY {{1, 0, 0, 1}}
#define MSH_MAT3_IDENTITY {{1, 0, 0, 0, 1, 0, 0, 0, 1}}
#define MSH_MAT4_IDENTITY {{1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}}


/*
 * =============================================================================
 *       VECTOR IMPLEMENTATION
 * =============================================================================
 */
#ifdef MSH_VEC_MATH_IMPLEMENTATION

#include <stdio.h>
inline void
msh_fprint_vec2( const msh_vec2_t *vec, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f\n", vec->x, vec->y );
}

inline void
msh_fprint_vec3( const msh_vec3_t *vec, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f %10.5f\n", vec->x, vec->y, vec->z );
}

inline void
msh_fprint_vec4( const msh_vec4_t *vec, FILE *stream )
{
  fprintf( stream, "%10.5f %10.5f %10.5f %10.5f\n", 
                   vec->x, vec->y, vec->z, vec->w );
}

inline void
msh_fprint_mat2( const msh_mat2_t *mat, FILE *stream )
{
  const float *mat_ptr[2] = {&mat->data[0], &mat->data[2]};
  fprintf( stream, "%10.5f %10.5f\n%10.5f %10.5f\n\n", 
                    mat_ptr[0][0], mat_ptr[1][0], 
                    mat_ptr[0][1], mat_ptr[1][1] );
}

inline void
msh_fprint_mat3( const msh_mat3_t *mat, FILE *stream )
{
  const float *mat_ptr[3] = {&mat->data[0], &mat->data[3], &mat->data[6]};
  fprintf( stream, "%10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f\n\n", 
                   mat_ptr[0][0], mat_ptr[1][0], mat_ptr[2][0],
                   mat_ptr[0][1], mat_ptr[1][1], mat_ptr[2][1],
                   mat_ptr[0][2], mat_ptr[1][2], mat_ptr[2][2] );
}

inline void
msh_fprint_mat4( const msh_mat4_t *mat, FILE *stream )
{
  const float *mat_ptr[4] = { &mat->data[0], &mat->data[4], 
                              &mat->data[8], &mat->data[12] };
  fprintf( stream, "%10.5f %10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f %10.5f\n"
                   "%10.5f %10.5f %10.5f %10.5f\n\n",
                   mat_ptr[0][0], mat_ptr[1][0], mat_ptr[2][0], mat_ptr[3][0],
                   mat_ptr[0][1], mat_ptr[1][1], mat_ptr[2][1], mat_ptr[3][1], 
                   mat_ptr[0][2], mat_ptr[1][2], mat_ptr[2][2], mat_ptr[3][2],
                   mat_ptr[0][3], mat_ptr[1][3], mat_ptr[2][3], mat_ptr[3][3] );
}

inline void
msh_print_vec2( const msh_vec2_t *vec )
{
  msh_fprint_vec2( vec, stdout );
}

inline void
msh_print_vec3( const msh_vec3_t *vec )
{
  msh_fprint_vec3( vec, stdout );
}

inline void
msh_print_vec4( const msh_vec4_t *vec )
{
  msh_fprint_vec4( vec, stdout );
}

inline void
msh_print_mat2( const msh_mat2_t *vec )
{
  msh_fprint_mat2( vec, stdout );
}

inline void
msh_print_mat3( const msh_mat3_t *vec )
{
  msh_fprint_mat3( vec, stdout );
}

inline void
msh_print_mat4( const msh_mat4_t *vec )
{
  msh_fprint_mat4( vec, stdout );
}

inline msh_vec2_t 
msh_vec2_add( const msh_vec2_t *vec_a, const msh_vec2_t *vec_b )
{
  msh_vec2_t vec_c = {{ vec_a->x + vec_b->x, vec_a->y + vec_b->y }};
  return vec_c;
}

#endif /* MSH_VEC_MATH_IMPLEMENTATION */

#endif /* MSH_VEC_MATH */

