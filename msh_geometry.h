/*
  ==============================================================================
  
  MSH_GEOMETRY.H - v0.01
  
  A single header library for simple geometrical objects and their interactions.
  Think bounding boxes and ray intersections. 

  To use the library you simply following once in your source
  
  #define MSH_GEOMETRY_IMPLEMENTATION
  #include "msh_geometry.h"

  ==============================================================================
  DEPENDENCIES
    msh.h
    msh_vec_mat.h
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
  [ ] Test u,v,w in triangle intersection
  [ ] Move the ray through pixel to msh_cam.h
  [ ] Read more about separating axis test ( Ericsen )
  [ ] Test and improve robustness
  [ ] Improve aabb tree build time - change build iterative, non recursive function maybe?
  [ ] Dynamic aabb tree rebuild
  [ ] For aabb tree building - make handle to tree an opaque type (int) and use descriptor to build 
      it. See sokol for how that is achieved.
  [ ] Occlusion culling for ray-aabb traversal 
      (Dont test agains boxes that are completly behind others)
  [ ] AABB traversal could be made generic with callbacks..
  [ ] Error handling
  ==============================================================================
  REFERENCES:
 */


#ifndef MSH_GEOMETRY_H
#define MSH_GEOMETRY_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_GEOMETY_STATIC
#define MSHGEODEF static
#else
#define MSHGEODEF extern
#endif

#define MSHGEO_EPSILON 1e-6

typedef struct msh_geo_ray
{
  msh_vec3_t origin;
  msh_vec3_t direction;
} msh_ray_t;

typedef struct msh_geo_aabb
{
  msh_vec3_t min_pt;
  msh_vec3_t max_pt;
} msh_aabb_t;

typedef struct msh_geo_obb
{
  msh_vec3_t center;
  msh_mat3_t orientation;
  msh_vec3_t extends;
} msh_obb_t;

typedef struct msh_geo_plane
{
  msh_vec3_t center;
  msh_vec3_t normal;
} msh_plane_t;

typedef struct msh_geo_sphere
{
  msh_vec3_t center;
  float radius;
} msh_sphere_t;

typedef struct msh_geo_triangle
{
  msh_vec3_t v0;
  msh_vec3_t v1;
  msh_vec3_t v2;
} msh_triangle_t;

/* Based on https://github.com/rmitton/rjm/blob/master/rjm_raytrace.h 
 * It is a little slow to build - Will need to look into fixing that */
typedef struct msh_geo_aabb_tree
{
  /* User filled */
  size_t n_tris;
  uint32_t* tris;
  msh_vec3_t* verts;
  uint32_t max_tris_per_leaf;

  uint32_t *leaf_tris;             /* Indices of tris in a leaf -> indices to original mesh */
  struct msh_geo_aabb *nodes;
  struct msh_geo_aabb_leaf *leafs; /* Stores indices to leaf_tris */
  uint32_t first_leaf;
  uint32_t n_nodes;
  uint32_t n_leafs;
  uint32_t n_lvls;

} msh_aabb_tree_t;

MSHGEODEF void       mshgeo_aabb_init( msh_aabb_t* a );
MSHGEODEF void       mshgeo_aabb_expand( msh_aabb_t* a, const msh_vec3_t* p );

MSHGEODEF msh_vec2_t mshgeo_pts2d_center( msh_vec2_t* points, size_t n_points );
MSHGEODEF msh_mat2_t mshgeo_pts2d_covariance( msh_vec2_t center, msh_vec2_t* points, size_t n_points );
MSHGEODEF void       mshgeo_pts2d_pca( msh_vec2_t center, msh_vec2_t* points, size_t n_points, 
                                       msh_mat2_t *eig_vec, msh_vec2_t* eig_val );

MSHGEODEF msh_vec3_t mshgeo_pts3d_center( msh_vec3_t* points, size_t n_points );
MSHGEODEF msh_mat3_t mshgeo_pts3d_covariance( msh_vec3_t center, msh_vec3_t* points, size_t n_points );
MSHGEODEF void       mshgeo_pts3d_pca( msh_vec3_t center, msh_vec3_t* points, size_t n_points, 
                                       msh_mat3_t *eig_vec, msh_vec3_t* eig_val );


/* deprecated */
typedef struct mshgeo_bbox
{
  msh_vec3_t min_p;
  msh_vec3_t max_p;
} msh_bbox_t;

MSHGEODEF msh_bbox_t mshgeo_bbox_init();
MSHGEODEF void       mshgeo_bbox_reset(msh_bbox_t* bbox);
MSHGEODEF void       mshgeo_bbox_union(msh_bbox_t* bb, msh_vec3_t p);
MSHGEODEF msh_vec3_t mshgeo_bbox_centroid(msh_bbox_t *bb);
MSHGEODEF float      mshgeo_bbox_width(msh_bbox_t *bb);
MSHGEODEF float      mshgeo_bbox_height(msh_bbox_t *bb);
MSHGEODEF float      mshgeo_bbox_depth(msh_bbox_t *bb);
MSHGEODEF msh_vec3_t mshgeo_bbox_diagonal(msh_bbox_t* bb);
MSHGEODEF float      mshgeo_bbox_volume(msh_bbox_t* bb);
MSHGEODEF bool       mshgeo_bbox_intersect(msh_bbox_t* ba, msh_bbox_t* bb);


#ifdef __cplusplus
}
#endif

#endif /*MSH_GEOMETRY_H*/

#ifdef MSH_GEOMETRY_IMPLEMENTATION

#define MSHGEO_SWAP(T, X, Y) { T _tmp = (X); (X) = (Y); (Y) = _tmp; }

MSHGEODEF void
mshgeo_aabb_init( msh_aabb_t* a )
{
  a->min_pt.x = MSH_F32_MAX;
  a->min_pt.y = MSH_F32_MAX;
  a->min_pt.z = MSH_F32_MAX;

  a->max_pt.x = -MSH_F32_MAX;
  a->max_pt.y = -MSH_F32_MAX;
  a->max_pt.z = -MSH_F32_MAX;
}

MSHGEODEF void
mshgeo_aabb_expand( msh_aabb_t* a, const msh_vec3_t* p )
{
  a->min_pt.x = msh_min( a->min_pt.x, p->x );
  a->min_pt.y = msh_min( a->min_pt.y, p->y );
  a->min_pt.z = msh_min( a->min_pt.z, p->z );

  a->max_pt.x = msh_max( a->max_pt.x, p->x );
  a->max_pt.y = msh_max( a->max_pt.y, p->y );
  a->max_pt.z = msh_max( a->max_pt.z, p->z );
}

// NOTE(maciej): Look at rygorous notes on accumulation and see if they might apply here
// NOTE(maciej): This code seems like a good candidate for SIMD

MSHGEODEF msh_vec2_t
mshgeo_pts2d_centroid( msh_vec2_t* pts, size_t n_pts )
{
  msh_vec2_t c = msh_vec2_zeros();
  for( size_t i = 0; i < n_pts; ++i )
  {
    c = msh_vec2_add( c, pts[i] );
  }
  c = msh_vec2_scalar_div( c, (real32_t)n_pts );
  return c;
}

MSHGEODEF msh_mat2_t
mshgeo_pts2d_covariance( msh_vec2_t c, msh_vec2_t* pts, size_t n_pts )
{
  msh_mat2_t cov = msh_mat2_zeros();
  for( size_t i = 0; i < n_pts; ++i )
  {
    msh_vec2_t b = msh_vec2_sub( pts[i], c );
    msh_mat2_t m = msh_vec2_outer_product( b, b );
    cov = msh_mat2_add( cov, m );
  }
  cov = msh_mat2_scalar_div( cov, (float)n_pts );
  return cov;
}


void eigen_decomposition2( double* A, double* V, double* d );

MSHGEODEF void
mshgeo_pts2d_pca( msh_vec2_t c, msh_vec2_t* pts, size_t n_pts, 
                  msh_mat2_t* eig_vec, msh_vec2_t* eig_val )
{
  msh_mat2_t cov = mshgeo_pts2d_covariance( c, pts, n_pts );
  
  double A[4] = {0};
  double V[4] = {0};
  double d[2] = {0};
  A[0] = cov.data[0];
  A[1] = cov.data[1];
  A[2] = cov.data[2];
  A[3] = cov.data[3];

  eigen_decomposition2( A, V, d );

  eig_vec->data[0] = V[0];
  eig_vec->data[1] = V[1];
  eig_vec->data[2] = V[2];
  eig_vec->data[3] = V[3];
  eig_val->data[0] = d[0];
  eig_val->data[1] = d[1];
}


MSHGEODEF msh_vec3_t
mshgeo_pts3d_centroid( msh_vec3_t* pts, size_t n_pts )
{
  msh_vec3_t c = msh_vec3_zeros();
  for( size_t i = 0; i < n_pts; ++i )
  {
    c = msh_vec3_add( c, pts[i] );
  }
  c = msh_vec3_scalar_div( c, (real32_t)n_pts );
  return c;
}

MSHGEODEF msh_mat3_t
mshgeo_pts3d_covariance( msh_vec3_t c, msh_vec3_t* pts, size_t n_pts )
{
  // huhu -> this is accumulation.. should be done with doubles.
  msh_mat3_t cov = msh_mat3_zeros();
  for( size_t i = 0; i < n_pts; ++i )
  {
    msh_vec3_t b = msh_vec3_sub( pts[i], c );
    msh_mat3_t m = msh_vec3_outer_product( b, b );
    cov = msh_mat3_add( cov, m );
  }
  cov = msh_mat3_scalar_div( cov, (float)n_pts );
  return cov;
}

void eigen_decomposition3( double* A, double* V, double* d );

MSHGEODEF void
mshgeo_pts3d_pca( msh_vec3_t c, msh_vec3_t* pts, size_t n_pts, 
                  msh_mat3_t* eig_vec, msh_vec3_t* eig_val )
{
  msh_mat3_t cov = mshgeo_pts3d_covariance( c, pts, n_pts );

  double A[9] = {0};
  double V[9] = {0};
  double d[3] = {0};
  for( int i = 0; i < 3; ++i )
  {
    for( int j = 0; j < 3; ++j )
    {
      A[3*j + i] = cov.data[3*i + j];
    }
  }

  eigen_decomposition3( A, V, d );

  for( int i = 0; i < 3; ++i )
  {
    for( int j = 0; j < 3; ++j )
    {
      eig_vec->data[3*i + j] = V[3*j+i];
    }
    eig_val->data[i] = d[i];
  }

}


/* From Realtime colision detection by Ericson
 * It is hard to find (at least for me) a good geometrical intuition for this algorithm, but
 * algebra is pretty clear.
 * Define a triangle as T(v,w) = A + v(B-A) + w(C-A) (follows from barycentric coordinates)
 * Define a ray as R(t) = P + t(Q-P)
 * We wish to find T(v,w) = R(t)
 * 
 * A + v(B-A) + w(C-A) = P + t(Q-P)
 * v(B-A) + w(C-A) +t(P-Q) = P - A < -- this is a 3x3 linear system of equations
 * 
 * Can be solverd with Cramer's rule, but we can notice that det([a b c]) = a.(b x c), giving us
 * solution:
 * t =  (P-A).n / d
 * v =  (C-A).e / d
 * w = -(B-A).e / d
 * with
 * n = (B-A)x(C-A)
 * d = (P-Q).n
 * e = (P-Q)x(P-A)
 * 
 * There is also an implementation / method described in RTR, which in turns references:
 * https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
 * Possibly Moller implementation is faster, will need to revisit if this ever becomes an issue
 * 
 * This file provides a simd implementation
 * https://github.com/rmitton/rjm/blob/master/rjm_raytrace.h
 * 
 * Given how time critical this is, I might consider doing this with simd.
 * */

//NOTE(maciej): Somethings real wrong here.
MSHGEODEF int
msh_ray_plane_intersect( const msh_ray_t* ray, msh_vec3_t a, msh_vec3_t n, float* t )
{
  msh_vec3_t o = ray->origin;
  msh_vec3_t d = ray->direction;
  float denom = msh_vec3_dot( d, n );
  if( fabs(denom) > 1e-6 ) {
    msh_vec3_t v = msh_vec3_sub( a, o );
    (*t) = msh_vec3_dot(v, n) / denom;
    return ((*t) >= 0);
  }
  (*t) = MSH_F32_MAX;
  return 0;
}

MSHGEODEF int
msh_ray_triangle_intersect( const msh_ray_t* ray, msh_vec3_t a, msh_vec3_t b, msh_vec3_t c,
                            float *u, float *v, float *w, float* t )
{
  msh_vec3_t ab = msh_vec3_sub( b, a );
  msh_vec3_t ac = msh_vec3_sub( c, a );
  msh_vec3_t p = ray->origin;
  // msh_vec3_t q = msh_vec3_add( ray->origin, msh_vec3_scalar_mul( ray->direction, 1000 ) );
  msh_vec3_t q = msh_vec3_add( ray->origin, ray->direction );// thats stupid, no?
  msh_vec3_t qp = msh_vec3_sub( p, q );

  msh_vec3_t n = msh_vec3_cross( ab, ac );

  float d = msh_vec3_dot( qp, n );
  if( d <= 0.0f ) { return  0; }

  msh_vec3_t ap = msh_vec3_sub( p, a );
  (*t) = msh_vec3_dot(ap, n);
  if( (*t) < 0.0f ) { return 0; }

  msh_vec3_t e = msh_vec3_cross( qp, ap );
  (*v) = msh_vec3_dot( ac, e );
  if ( (*v) < 0.0f || (*v) > d ) { return  0; }
  (*w) = -msh_vec3_dot( ab, e );
  if ( (*w) < 0.0f || (*v) + (*w) > d ) { return  0; }

  float ood = 1.0f / d;
  (*t) *= ood;
  (*v) *= ood;
  (*w) *= ood;
  (*u) = 1.0 - (*v) - (*w);

  return 1;
}

/* From "Real-Time Collision Detection" by Ericsen
 *  Uses SAT - separating axis test
 * */
MSHGEODEF int
mshgeo_ray_aabb_test( const msh_ray_t* ray, const msh_aabb_t* box )
{
  msh_vec3_t p0 = ray->origin;
  msh_vec3_t p1 = msh_vec3_add( ray->origin, msh_vec3_scalar_mul( ray->direction, 100.0f ) );

  msh_vec3_t c = msh_vec3_scalar_mul( msh_vec3_add( box->min_pt, box->max_pt ), 0.5f );
  msh_vec3_t e = msh_vec3_sub( box->max_pt, c );
  msh_vec3_t m = msh_vec3_scalar_mul( msh_vec3_add( p0, p1 ), 0.5f );
  msh_vec3_t d = msh_vec3_sub( p1, m );
  m =  msh_vec3_sub( m, c );

  real32_t adx = fabs( d.x );
  if( abs(m.x) > e.x + adx ) { return 0; }
  real32_t ady = fabs( d.y );
  if( abs(m.y) > e.y + ady ) { return 0; }
  real32_t adz = fabs( d.z );
  if( abs(m.z) > e.z + adz ) { return 0; }

  adx += MSHGEO_EPSILON; ady += MSHGEO_EPSILON; adz += MSHGEO_EPSILON;

  if( abs( m.y * d.z - m.z * d.y) > e.y * adz + e.z * ady ) { return 0; }
  if( abs( m.z * d.x - m.x * d.z) > e.x * adz + e.z * adx ) { return 0; }
  if( abs( m.x * d.y - m.y * d.x) > e.x * ady + e.y * adx ) { return 0; }

  return 1;
}

/* From "Real-Time Collision Detection" by Ericsen 
 * Seems very fast - can be further speeded up when doing intersections against a lot of boxes.
 * */
MSHGEODEF int
mshgeo_ray_aabb_intersects( const msh_ray_t* ray, 
                            const msh_aabb_t* box,
                            float *tmin, msh_vec3_t* q )
{
  (*tmin) = -MSH_F32_MAX;
  float tmax = MSH_F32_MAX;

  for( int i = 0; i < 3; ++i )
  {
    if( fabs( ray->direction.data[i] ) < MSHGEO_EPSILON )
    {
      if( ray->origin.data[i] < box->min_pt.data[i] ||
          ray->origin.data[i] > box->max_pt.data[i]) return 0;
    }
    else
    {
      float ood = 1.0f / ray->direction.data[i];
      float t1 = (box->min_pt.data[i] - ray->origin.data[i]) * ood;
      float t2 = (box->max_pt.data[i] - ray->origin.data[i]) * ood;
      if( t1 > t2 ) { float tmp = t1; t1 = t2; t2 = tmp; }
      if( t1 > (*tmin) ) (*tmin) = t1;
      if( t2 < tmax ) tmax = t2;
      if( (*tmin) > tmax ) return 0;
    }
  }
  if( q ) *q = msh_vec3_add(ray->origin, msh_vec3_scalar_mul(ray->direction, *tmin));
  return 1;
}

MSHGEODEF real32_t
mshgeo_point_aabb_dist_sq( const msh_vec3_t* p, const msh_aabb_t* b )
{
  real32_t dist_sq = 0.0f;

  // For each axis count any excess distance outside box extents 
  if( p->x < b->min_pt.x) { dist_sq += (b->min_pt.x - p->x) * (b->min_pt.x - p->x); }
  if( p->x > b->max_pt.x) { dist_sq += (p->x - b->max_pt.x) * (p->x - b->max_pt.x); }

  if( p->y < b->min_pt.y) { dist_sq += (b->min_pt.y - p->y) * (b->min_pt.y - p->y); }
  if( p->y > b->max_pt.y) { dist_sq += (p->y - b->max_pt.y) * (p->y - b->max_pt.y); }
 
  if( p->z < b->min_pt.z) { dist_sq += (b->min_pt.z - p->z) * (b->min_pt.z - p->z); }
  if( p->z > b->max_pt.z) { dist_sq += (p->z - b->max_pt.z) * (p->z - b->max_pt.z); }

  return dist_sq;
}

MSHGEODEF uint8_t
mshgeo_sphere_aabb_test( const msh_sphere_t* sphere, const msh_aabb_t* box )
{
  real32_t dist_sq = mshgeo_point_aabb_dist_sq( &sphere->center, box );
  return ( dist_sq <= sphere->radius * sphere->radius );
}

MSHGEODEF int
mshgeo_sphere_triangle_test_approx( const msh_sphere_t* sphere, 
                                    const msh_vec3_t* a, const msh_vec3_t* b, const msh_vec3_t* c )
{
  real32_t rsq = sphere->radius * sphere->radius;
  if( msh_vec3_norm_sq( msh_vec3_sub( sphere->center, *a ) ) < rsq ) { return 1; }
  if( msh_vec3_norm_sq( msh_vec3_sub( sphere->center, *b ) ) < rsq ) { return 1; }
  if( msh_vec3_norm_sq( msh_vec3_sub( sphere->center, *c ) ) < rsq ) { return 1; }
  return 0;
}

/* Again from Real-time collision detection by Ericsen */
MSHGEODEF msh_vec3_t
mshgeo_point_triangle_closest_point( const msh_vec3_t* p,
                                     const msh_vec3_t* a, const msh_vec3_t* b, const msh_vec3_t* c )
{
  msh_vec3_t ab = msh_vec3_sub( *b, *a );
  msh_vec3_t ac = msh_vec3_sub( *c, *a );
  msh_vec3_t ap = msh_vec3_sub( *p, *a );
  real32_t d1 = msh_vec3_dot( ab, ap );
  real32_t d2 = msh_vec3_dot( ac, ap );
  if( d1 <= 0.0f && d2 <= 0.0f ) { return *a; }

  msh_vec3_t bp = msh_vec3_sub( *p, *b );
  real32_t d3 = msh_vec3_dot( ab, bp );
  real32_t d4 = msh_vec3_dot( ac, bp );
  if( d3 >= 0.0f && d4 <= d3 ) { return *b; }

  real32_t vc = d1*d4 - d3*d2;
  if( vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f )
  {
    real32_t v = d1 / (d1 - d3);
    return msh_vec3_add( *a, msh_vec3_scalar_mul( ab, v ) );
  }

  msh_vec3_t cp = msh_vec3_sub( *p, *c );
  real32_t d5 = msh_vec3_dot( ab, cp );
  real32_t d6 = msh_vec3_dot( ab, cp );
  if( d6 >= 0.0f && d5 <= d6 ) { return *c; }

  real32_t vb = d5*d2 - d1*d6;
  if( vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f )
  {
    real32_t w = d2 / (d2 - d6 );
    return msh_vec3_add( *a, msh_vec3_scalar_mul( ac, w ) );
  }

  real32_t va = d3*d6 - d5*d4;
  if( va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f )
  {
    real32_t w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
    msh_vec3_t bc =  msh_vec3_sub( *c, *b );
    return msh_vec3_add( *b , msh_vec3_scalar_mul( bc, w ) );
  }

  real32_t denom = 1.0f / ( va + vb + vc );
  real32_t v = vb * denom;
  real32_t w = vc* denom;
  return msh_vec3_add(*a, msh_vec3_add( msh_vec3_scalar_mul( ab, v ),
                                        msh_vec3_scalar_mul( ac, w ) ) );
}

MSHGEODEF int32_t
mshgeo_sphere_triangle_test( const msh_sphere_t* sphere, 
                             const msh_vec3_t* a, const msh_vec3_t* b, const msh_vec3_t* c )
{
  msh_vec3_t p = mshgeo_point_triangle_closest_point( &sphere->center, a, b, c );

  msh_vec3_t v      = msh_vec3_sub( p, sphere->center );
  real64_t v_len_sq = msh_vec3_norm_sq( v );
  real64_t r_sq     = sphere->radius * sphere->radius;

  return v_len_sq < r_sq;
}


typedef struct msh_geo_aabb msh_aabb_node_t;

typedef struct msh_geo_aabb_leaf
{
  uint32_t tri_idx;   /* Index to intermediate array storing indices to input mesh */
  uint32_t n_tris;   /* Number of tris in the leaf */
} msh_aabb_leaf_t;

static uint32_t *
mshgeo__aabb_tree_partition( msh_aabb_tree_t *tree,
                             uint32_t *left, uint32_t *right, uint8_t dim )
{
  /* Split test look at approperiate dimension of the first vertex in a triangle 
    pointed by the pivot */
  uint32_t pivot = right[0];
  uint32_t *ind = &tree->tris[ pivot * 3 ];
  real32_t split = tree->verts[ ind[0] ].data[ dim ];
  split += tree->verts[ ind[1] ].data[ dim ];
  split += tree->verts[ ind[2] ].data[ dim ];

  uint32_t *dest = left;
  for( uint32_t *i = left; i < right; i++ )
  {
    /* Similarly look at all triangles in the current extends (left-right) and figure out how
       position of their first vertex along approperiate dimension compares to split */
    ind = &tree->tris[ (*i) * 3 ];
    real32_t test = tree->verts[ ind[0] ].data[ dim ];
    test += tree->verts[ ind[1] ].data[ dim ];
    test += tree->verts[ ind[2] ].data[ dim ];

    /* If less then split, swap so that we partition mesh correctly */
    if( test < split )
    {
      MSHGEO_SWAP( uint32_t, *dest, *i );
      dest++;
    }
  }
  MSHGEO_SWAP( uint32_t, *dest, *right );

  return dest;
}

/* debug this on some simple example and understand how this works */
static void 
mshgeo__aabb_tree_quickselect( msh_aabb_tree_t *tree, 
                               uint32_t *left, uint32_t *right, uint32_t *mid, uint8_t dim )
{
  for(;;)
  {
    uint32_t *pivot = mshgeo__aabb_tree_partition( tree, left, right, dim );
    if ( mid < pivot )      { right = pivot - 1; }
    else if ( mid > pivot ) { left = pivot + 1; }
    else                    { break; }
  }
}

static void 
msh__aabb_tree_build_nodes( msh_aabb_tree_t *tree,
                            uint32_t node_idx, uint32_t tri_idx, uint32_t n_tris )
{
  if( node_idx >= tree->first_leaf  ) 
  {
    assert( n_tris <= tree->max_tris_per_leaf );
    msh_aabb_leaf_t *leaf = tree->leafs + (node_idx - tree->first_leaf);
    leaf->tri_idx         = tri_idx;
    leaf->n_tris          = n_tris;
    tree->n_leafs += 1;
    return;
  }

  /* Simple object-median split algorithm. Performs reasonably
   * well, gives us a balanced, implicit tree, and is guaranteed
   * to always split */

  /* Calculate bounds */
  msh_aabb_t *node = tree->nodes + node_idx;
  for( uint32_t n = 0; n < n_tris; n++ )
  {
    uint32_t idx = *(tree->tris + 3 * tree->leaf_tris[ tri_idx + n ]);
    msh_vec3_t* v = tree->verts + idx;
    mshgeo_aabb_expand( node, v );
  }
  tree->n_nodes++;

  /* Select dimension with the largest extend */
  msh_vec3_t extends = msh_vec3_sub( node->max_pt, node->min_pt );
  uint8_t dim = 0;
  if( extends.data[1] > extends.data[dim] ) { dim = 1; }
  if( extends.data[2] > extends.data[dim] ) { dim = 2; }

  // Partition
  assert( n_tris > 0 );
  uint32_t left_count = n_tris >> 1;
  uint32_t *tris = tree->leaf_tris + tri_idx;
  mshgeo__aabb_tree_quickselect( tree, tris, tris + n_tris - 1, tris + left_count, dim );

  // Recurse
  msh__aabb_tree_build_nodes( tree, node_idx * 2 + 1, tri_idx, left_count );
  msh__aabb_tree_build_nodes( tree, node_idx * 2 + 2, tri_idx + left_count, n_tris - left_count );
}

MSHGEODEF int
mshgeo_aabb_tree_build( msh_aabb_tree_t* tree )
{
  assert( tree->tris );
  assert( tree->verts );
  assert( tree->n_tris > 0 );

  if( tree->max_tris_per_leaf <= 0 ) { tree->max_tris_per_leaf = 64; }

  uint32_t leaf_count = 1;
  tree->n_lvls = 0;
  while( leaf_count * tree->max_tris_per_leaf < tree->n_tris ) 
  { 
    leaf_count <<= 1;
    tree->n_lvls++;
  };

  tree->first_leaf = leaf_count - 1;
  tree->nodes      = (msh_aabb_node_t*)malloc( tree->first_leaf * sizeof( msh_aabb_node_t ) );
  tree->leafs      = (msh_aabb_leaf_t*)malloc( leaf_count * sizeof( msh_aabb_leaf_t ) );
  tree->leaf_tris  = (uint32_t*)malloc( tree->n_tris * sizeof( uint32_t ) );
  tree->n_leafs    = 0;
  tree->n_nodes    = 0;
  
  // Initialize aabbs;
  for( size_t i = 0; i < tree->first_leaf; ++i )
  {
    mshgeo_aabb_init( &(tree->nodes[i]) );
  }

  for( uint32_t n = 0; n < tree->n_tris; ++n )
  {
    tree->leaf_tris[ n ] = n;
  }

  msh__aabb_tree_build_nodes( tree, 0, 0, tree->n_tris );
  return 1;
}

MSHGEODEF void
mshgeo_aabb_tree_free( msh_aabb_tree_t* tree )
{
  free( tree->nodes );
  free( tree->leafs );
  free( tree->leaf_tris );
  tree->first_leaf = 0;
  tree->n_nodes = 0;
  tree->n_leafs = 0;
  tree->n_lvls = 0;
}

// NOTE(maciej): Both intersection traversals are extremly similar - I should look into how
//               to ensure that the code is more compressed ( as in semantic compression by casey muratori )
typedef struct msh_ray_bvh_isect_info
{
  uint32_t intersects;
  int32_t tri_idx;
  real32_t u, v, w, t; /* TODO(maciej): Remove w */

  /* Debug info -> Maybe disable when NDEBUG flag is available? */
  int32_t n_nodes_visited;
  int32_t n_leafs_visited;
  int32_t n_tris_tested;

  /* Optional, memory provided by user */
  uint8_t* intersects_node;
} msh_ray_bvh_isect_info_t;

MSHGEODEF int32_t
mshgeo_ray_aabb_tree_intersect( const msh_ray_t* ray, const msh_aabb_tree_t* tree,
                                msh_ray_bvh_isect_info_t* isect_info )
{
  int32_t stack[256];
  int32_t sp = 0;
  int32_t node_idx = 0;

  // Fill initial intersection info
  isect_info->intersects      = 0;
  isect_info->tri_idx         = -1;
  isect_info->t               = MSH_F32_MAX;
  isect_info->u               = MSH_F32_MAX;
  isect_info->v               = MSH_F32_MAX;
  isect_info->w               = MSH_F32_MAX;
  isect_info->n_nodes_visited = 0;
  isect_info->n_leafs_visited = 0;
  isect_info->n_tris_tested   = 0;
  if( isect_info->intersects_node )
  {
    size_t byte_size = tree->n_nodes * sizeof(isect_info->intersects_node[0]);
    memset( isect_info->intersects_node, 0, byte_size );
  }

  // Trace the tree
  stack[sp] = 0;
  do {
    int32_t leaf_idx = node_idx - tree->first_leaf;
    if (leaf_idx >= 0)
    {
      // Leaf, test each triangle
      msh_aabb_leaf_t *leaf = tree->leafs + leaf_idx;
      uint32_t *inds = tree->leaf_tris + leaf->tri_idx;
      int32_t n_tris = leaf->n_tris;

      while( n_tris-- )
      {
        // Read triangle data
        uint32_t tri_idx = *inds++;
        uint32_t *tri    = tree->tris + (tri_idx * 3);
        msh_vec3_t *v0   = tree->verts + tri[0];
        msh_vec3_t *v1   = tree->verts + tri[1];
        msh_vec3_t *v2   = tree->verts + tri[2];

        // Ray-triangle intersection
        real32_t t = MSH_F32_MAX;
        real32_t u, v, w;
        uint8_t intersects = msh_ray_triangle_intersect( ray, *v0, *v1, *v2, &u, &v, &w, &t );
        if( intersects && t < isect_info->t )
        {
          isect_info->tri_idx    = tri_idx;
          isect_info->t          = t;
          isect_info->intersects = 1;
          isect_info->u          = u;
          isect_info->v          = v;
          isect_info->w          = w;
        }
      }

      // Gather stats
      isect_info->n_tris_tested += leaf->n_tris;
      isect_info->n_leafs_visited++;
    }
    else
    {
      // Check if ray hits the box
      uint8_t intersects = mshgeo_ray_aabb_test( ray, &tree->nodes[node_idx] );

      // If hit, recurse down the trees
      if( intersects )
      {
        // Gather stats
        if( isect_info->intersects_node ) { isect_info->intersects_node[node_idx] = 1; }
        isect_info->n_nodes_visited++;

        // Push nodes onto stack
        stack[sp++] = node_idx * 2 + 2;
        node_idx = node_idx * 2 + 1;
        continue;
      }
    }

    // Pull a new node off the stack
    node_idx = stack[--sp];
  } while( sp >= 0 );

  return isect_info->intersects;
}

typedef struct msh_sphere_bvh_isect_info
{
  uint32_t intersects;
  uint32_t* tri_list;
  uint32_t tri_list_len;
  uint32_t tri_list_cap;

  /* Debug info -> Maybe disable when NDEBUG flag is available? */
  int32_t n_nodes_visited;
  int32_t n_leafs_visited;
  int32_t n_tris_tested;

  /* Optional, memory provided by user */
  uint8_t* intersects_node;

} msh_sphere_bvh_isect_info_t;

MSHGEODEF int32_t
mshgeo_sphere_aabb_tree_intersect( const msh_sphere_t* sphere, const msh_aabb_tree_t* tree,
                                   msh_sphere_bvh_isect_info_t* isect_info )
{
  int32_t stack[256];
  int32_t sp = 0;
  int32_t node_idx = 0;

  // Fill initial intersection info
  if( !isect_info->tri_list  ) 
  { 
    isect_info->tri_list_cap = 1024;
    isect_info->tri_list = (uint32_t*)malloc( isect_info->tri_list_cap * sizeof( isect_info->tri_list[0] ) );
  }
  isect_info->tri_list_len = 0;
  isect_info->intersects = 0;
  isect_info->n_nodes_visited = 0;
  isect_info->n_leafs_visited = 0;
  isect_info->n_tris_tested   = 0;
  if( isect_info->intersects_node )
  {
    size_t byte_size = tree->n_nodes * sizeof(isect_info->intersects_node[0]);
    memset( isect_info->intersects_node, 0, byte_size );
  }

  // Trace the tree
  stack[sp] = 0;
  do {
    int32_t leaf_idx = node_idx - tree->first_leaf;
    if (leaf_idx >= 0)
    {
      // Leaf, test each triangle
      msh_aabb_leaf_t *leaf = tree->leafs + leaf_idx;
      uint32_t *inds = tree->leaf_tris + leaf->tri_idx;
      int32_t n_tris = leaf->n_tris;

      while( n_tris-- )
      {
        // Read triangle data
        uint32_t tri_idx = *inds++;
        uint32_t *tri    = tree->tris + (tri_idx * 3);
        msh_vec3_t *v0   = tree->verts + tri[0];
        msh_vec3_t *v1   = tree->verts + tri[1];
        msh_vec3_t *v2   = tree->verts + tri[2];

        // Ray-sphere intersection
        uint8_t intersectiton_found = mshgeo_sphere_triangle_test_approx( sphere, v0, v1, v2 );
        if( intersectiton_found )
        {
          // No more space, realloc
          if( isect_info->tri_list_len >= isect_info->tri_list_cap )
          {
            isect_info->tri_list_cap *= 2;
            size_t byte_size = isect_info->tri_list_cap * sizeof(isect_info->tri_list[0]);
            isect_info->tri_list = (uint32_t*)realloc( isect_info->tri_list, byte_size );
          }

          isect_info->tri_list[isect_info->tri_list_len] = tri_idx;
          isect_info->tri_list_len++;
          isect_info->intersects = 1;
        }
      }
      // Gather stats
      isect_info->n_tris_tested += leaf->n_tris;
      isect_info->n_leafs_visited++;
    }
    else
    {
      // Check if sphere intersects the box
      uint8_t intersects = mshgeo_sphere_aabb_test( sphere, &tree->nodes[node_idx] );

      // If hit, recurse down the trees
      if( intersects )
      {
        // Gather stats
        if( isect_info->intersects_node ) { isect_info->intersects_node[node_idx] = 1; }
        isect_info->n_nodes_visited++;

        // Push nodes onto stack
        stack[sp++] = node_idx * 2 + 2;
        node_idx = node_idx * 2 + 1;
        continue;
      }
    }

    // Pull a new node off the stack
    node_idx = stack[--sp];
  } while( sp >= 0 );

  return isect_info->intersects;
}




/* deprecated */

MSHGEODEF msh_bbox_t
mshgeo_bbox_init()
{
  msh_bbox_t bb;
  bb.min_p = msh_vec3(1e9, 1e9, 1e9);
  bb.max_p = msh_vec3(-1e9, -1e9, -1e9);
  return bb;
}

MSHGEODEF void
mshgeo_bbox_reset(msh_bbox_t* bb)
{
  bb->min_p = msh_vec3(1e9, 1e9, 1e9);
  bb->max_p = msh_vec3(-1e9, -1e9, -1e9);
}

MSHGEODEF void
mshgeo_bbox_union(msh_bbox_t* bb, msh_vec3_t p)
{
  bb->min_p.x = msh_min(bb->min_p.x, p.x);
  bb->min_p.y = msh_min(bb->min_p.y, p.y);
  bb->min_p.z = msh_min(bb->min_p.z, p.z);
  bb->max_p.x = msh_max(bb->max_p.x, p.x);
  bb->max_p.y = msh_max(bb->max_p.y, p.y);
  bb->max_p.z = msh_max(bb->max_p.z, p.z);
}

MSHGEODEF msh_vec3_t
mshgeo_bbox_centroid(msh_bbox_t* bb)
{
  return msh_vec3_scalar_mul(msh_vec3_add(bb->min_p, bb->max_p), 0.5);
}

MSHGEODEF msh_vec3_t
mshgeo_bbox_diagonal(msh_bbox_t* bb)
{
  return msh_vec3_sub(bb->max_p, bb->min_p);
}

MSHGEODEF float
mshgeo_bbox_width(msh_bbox_t* bb)
{
  return bb->max_p.x - bb->min_p.x;
}

MSHGEODEF float
mshgeo_bbox_height(msh_bbox_t* bb)
{
  return bb->max_p.y - bb->min_p.y;
}

MSHGEODEF float
mshgeo_bbox_depth(msh_bbox_t* bb)
{
  return bb->max_p.z - bb->min_p.z;
}

MSHGEODEF float
mshgeo_bbox_volume(msh_bbox_t* bb)
{
  return (bb->max_p.x - bb->min_p.x)*
         (bb->max_p.y - bb->min_p.y)*
         (bb->max_p.z - bb->min_p.z);
}

MSHGEODEF bool
mshgeo_bbox_intersect( msh_bbox_t* ba, msh_bbox_t* bb )
{
  return ((ba->max_p.x >= bb->min_p.x && bb->max_p.x >= ba->min_p.x) && 
          (ba->max_p.y >= bb->min_p.y && bb->max_p.y >= ba->min_p.y) &&
          (ba->max_p.z >= bb->min_p.z && bb->max_p.z >= ba->min_p.z) );
}

// Eigen decomposition

static void tred2n(double* V, double* d, double* e, int32_t n) {

//  This is derived from the Algol procedures tred2 by
//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//  Fortran subroutine in EISPACK.

  for (int j = 0; j < n; j++) {
    d[j] = V[n*(n-1) + j];
  }

  // Householder reduction to tridiagonal form.

  for (int i = n-1; i > 0; i--) {

    // Scale to avoid under/overflow.

    double scale = 0.0;
    double h = 0.0;
    for (int k = 0; k < i; k++) {
      scale = scale + fabs(d[k]);
    }
    if (scale == 0.0) {
      e[i] = d[i-1];
      for (int j = 0; j < i; j++) {
        d[j] = V[n*(i-1) + j];
        V[n*i + j] = 0.0;
        V[n*j + i] = 0.0;
      }
    } else {

      // Generate Householder vector.

      for (int k = 0; k < i; k++) {
        d[k] /= scale;
        h += d[k] * d[k];
      }
      double f = d[i-1];
      double g = sqrt(h);
      if (f > 0) {
        g = -g;
      }
      e[i] = scale * g;
      h = h - f * g;
      d[i-1] = f - g;
      for (int j = 0; j < i; j++) {
        e[j] = 0.0;
      }

      // Apply similarity transformation to remaining columns.

      for (int j = 0; j < i; j++) {
        f = d[j];
        V[n*j+i] = f;
        g = e[j] + V[n*j+j] * f;
        for (int k = j+1; k <= i-1; k++) {
          g += V[n*k+j] * d[k];
          e[k] += V[n*k+j] * f;
        }
        e[j] = g;
      }
      f = 0.0;
      for (int j = 0; j < i; j++) {
        e[j] /= h;
        f += e[j] * d[j];
      }
      double hh = f / (h + h);
      for (int j = 0; j < i; j++) {
        e[j] -= hh * d[j];
      }
      for (int j = 0; j < i; j++) {
        f = d[j];
        g = e[j];
        for (int k = j; k <= i-1; k++) {
          V[n*k+j] -= (f * e[k] + g * d[k]);
        }
        d[j] = V[n*(i-1)+j];
        V[n*i+j] = 0.0;
      }
    }
    d[i] = h;
  }

  // Accumulate transformations.

  for (int i = 0; i < n-1; i++) {
    V[n*(n-1)+i] = V[n*i + i];
    V[n*i + i] = 1.0;
    double h = d[i+1];
    if (h != 0.0) {
      for (int k = 0; k <= i; k++) {
        d[k] = V[n*k + (i+1)] / h;
      }
      for (int j = 0; j <= i; j++) {
        double g = 0.0;
        for (int k = 0; k <= i; k++) {
          g += V[n*k + (i+1)] * V[n*k + j];
        }
        for (int k = 0; k <= i; k++) {
          V[n*k + j] -= g * d[k];
        }
      }
    }
    for (int k = 0; k <= i; k++) {
      V[n*k +(i+1)] = 0.0;
    }
  }
  for (int j = 0; j < n; j++) {
    d[j] = V[n*(n-1) + j];
    V[n*(n-1) + j] = 0.0;
  }
  V[n*(n-1) + (n-1)] = 1.0;
  e[0] = 0.0;
} 

// Symmetric tridiagonal QL algorithm.

static void tql2n(double* V, double* d, double* e, int32_t n) {

//  This is derived from the Algol procedures tql2, by
//  Bowdler, Martin, Reinsch, and Wilkinson, Handbook for
//  Auto. Comp., Vol.ii-Linear Algebra, and the corresponding
//  Fortran subroutine in EISPACK.

  for (int i = 1; i < n; i++) {
    e[i-1] = e[i];
  }
  e[n-1] = 0.0;

  double f = 0.0;
  double tst1 = 0.0;
  double eps = pow(2.0,-52.0);
  for (int l = 0; l < n; l++) {

    // Find small subdiagonal element

    tst1 = msh_max(tst1,fabs(d[l]) + fabs(e[l]));
    int m = l;
    while (m < n) {
      if (fabs(e[m]) <= eps*tst1) {
        break;
      }
      m++;
    }

    // If m == l, d[l] is an eigenvalue,
    // otherwise, iterate.

    if (m > l) {
      int iter = 0;
      do {
        iter = iter + 1;  // (Could check iteration count here.)

        // Compute implicit shift

        double g = d[l];
        double p = (d[l+1] - g) / (2.0 * e[l]);
        double r = hypot(p,1.0);
        if (p < 0) {
          r = -r;
        }
        d[l] = e[l] / (p + r);
        d[l+1] = e[l] * (p + r);
        double dl1 = d[l+1];
        double h = g - d[l];
        for (int i = l+2; i < n; i++) {
          d[i] -= h;
        }
        f = f + h;

        // Implicit QL transformation.

        p = d[m];
        double c = 1.0;
        double c2 = c;
        double c3 = c;
        double el1 = e[l+1];
        double s = 0.0;
        double s2 = 0.0;
        for (int i = m-1; i >= l; i--) {
          c3 = c2;
          c2 = c;
          s2 = s;
          g = c * e[i];
          h = c * p;
          r = hypot( p, e[i] );
          e[i+1] = s * r;
          s = e[i] / r;
          c = p / r;
          p = c * d[i] - s * g;
          d[i+1] = h + s * (c * g + s * d[i]);

          // Accumulate transformation.

          for (int k = 0; k < n; k++) {
            h = V[k*3 + i+1];
            V[k*3 + i+1] = s * V[k*3 + i] + c * h;
            V[k*3 + i] = c * V[k*3 + i] - s * h;
          }
        }
        p = -s * s2 * c3 * el1 * e[l] / dl1;
        e[l] = s * p;
        d[l] = c * p;

        // Check for convergence.

      } while (fabs(e[l]) > eps*tst1);
    }
    d[l] = d[l] + f;
    e[l] = 0.0;
  }
  
  // Sort eigenvalues and corresponding vectors.

  for (int i = 0; i < n-1; i++) {
    int k = i;
    double p = d[i];
    for (int j = i+1; j < n; j++) {
      if (d[j] < p) {
        k = j;
        p = d[j];
      }
    }
    if (k != i) {
      d[k] = d[i];
      d[i] = p;
      for (int j = 0; j < n; j++) {
        p = V[n*j+i];
        V[n*j+i] = V[n*j+k];
        V[n*j+k] = p;
      }
    }
  }
}
/* Symmetric matrix A => eigenvectors in columns of V, corresponding
   eigenvalues in d. */
void eigen_decomposition3( double* A, double* V, double* d )
{
  double e[3] = {0};
  for( int i = 0; i < 3; i++ ) {
    for( int j = 0; j < 3; j++ ) {
      V[3*i + j] = A[3*i + j];
    }
  }
  tred2n(V, d, e, 3);
  tql2n(V, d, e, 3);
}

void eigen_decomposition2( double* A, double* V, double* d )
{
  double e[2] = {0};
  for( int i = 0; i < 2; i++ ) {
    for( int j = 0; j < 2; j++ ) {
      V[2*i + j] = A[2*i + j];
    }
  }
  tred2n(V, d, e, 2);
  tql2n(V, d, e, 2);
}


#endif
