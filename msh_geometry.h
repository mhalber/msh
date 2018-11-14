/*
  ==============================================================================
  
  MSH_GEOMETRY.H - WIP!
  
  A single header library for simple geometrical objects and their interactions.
  Think bounding boxes and ray intersections. 

  To use the library you simply following once in your source
  
  #define MSH_GEOMETRY_IMPLEMENTATION
  #include "msh_gizon.h"

  All functions can be made static by definining:

  #ifdef MSH_GIZMO_STATIC

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
  [ ] Simplify the ray through pixel function
  [ ] Move the ray through pixel to msh_cam.h
  [ ] Read more about separating axis test ( Ericsen 195 )
  [ ] Test and improve robustness
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
  /* TODO */
} msh_obb_t;

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

#ifdef MSH_CAM

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

MSHGEODEF void
mshgeo_ray_through_pixel( msh_ray_t* ray, msh_vec2_t p, msh_vec4_t* viewport, msh_camera_t* camera )
{
  ray->origin = camera->position;
  msh_mat4_t inv_v = msh_mat4_inverse( camera->view );
  msh_mat4_t inv_p = msh_mat4_inverse( camera->proj );
  
  float clip_x = (2.0f * p.x) / viewport->z - 1.0f;
  float clip_y = 1.0f - (2.0f * p.y) / viewport->w;
  msh_vec4_t clip_coords = msh_vec4( clip_x, clip_y, 0.0f, 1.0f );

  msh_vec4_t eye_ray_dir = msh_mat4_vec4_mul( inv_p, clip_coords );
  eye_ray_dir.z = -1.0f;
  eye_ray_dir.w = 0.0f;
  msh_vec3_t world_ray_dir = msh_vec4_to_vec3( msh_mat4_vec4_mul( inv_v, eye_ray_dir ) );
  ray->direction = msh_vec3_normalize( world_ray_dir );
}

#endif

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
 * */

MSHGEODEF int
msh_ray_triangle_intersect( msh_ray_t* ray, msh_vec3_t a, msh_vec3_t b, msh_vec3_t c,
                            float *u, float *v, float *w, float* t )
{
  msh_vec3_t ab = msh_vec3_sub( b, a );
  msh_vec3_t ac = msh_vec3_sub( c, a );
  msh_vec3_t p = ray->origin;
  msh_vec3_t q = msh_vec3_add( ray->origin, msh_vec3_scalar_mul( ray->direction, 1000 ) );
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
 * Seems very fast - can be further sped up when doing intersections against a lot of boxes.
 * */
MSHGEODEF int
mshgeo_ray_aabb_intersects( const msh_ray_t* ray, const msh_aabb_t* box,
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
 
  if( p->z < b->min_pt.x) { dist_sq += (b->min_pt.z - p->z) * (b->min_pt.z - p->z); }
  if( p->z > b->max_pt.x) { dist_sq += (p->z - b->max_pt.z) * (p->z - b->max_pt.z); }

  return dist_sq;
}

MSHGEODEF int
mshgeo_sphere_aabb_test( const msh_sphere_t* sphere, const msh_aabb_t* box )
{
  real32_t dist_sq = mshgeo_point_aabb_dist_sq( &sphere->center, box );
  return ( dist_sq <= sphere->radius * sphere->radius );
}




/* Based on https://github.com/rmitton/rjm/blob/master/rjm_raytrace.h */
/* it is a little slow to build */
#define MSH_AABB_TREE_MAX_LEAF_TRIS 256
typedef struct msh_geo_aabb_tree
{
  size_t n_tris;
  uint32_t* tris;
  msh_vec3_t* verts;

  uint32_t first_leaf;
  uint32_t *leaf_tris;             /* Indices of tris in a leaf -> indices to original mesh */
  struct msh_geo_aabb *nodes;
  struct msh_geo_aabb_leaf *leafs; /* Stores indices to leaf_tris */
  uint32_t n_nodes;
  uint32_t n_leafs;

} msh_aabb_tree_t;

typedef struct msh_geo_aabb msh_aabb_node_t;

typedef struct msh_geo_aabb_leaf
{
  uint32_t tri_idx;   /* Index to intermediate array storing indices to input mesh */
  uint32_t n_tris;   /* Number of tris in the leaf */
} msh_aabb_leaf_t;

#define MSHGEO_SWAP(T, X, Y) { T _tmp = (X); (X) = (Y); (Y) = _tmp; }

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
  for (;;)
  {
    uint32_t *pivot = mshgeo__aabb_tree_partition( tree, left, right, dim );
    if ( mid < pivot )      { right = pivot - 1; }
    else if ( mid > pivot ) { left = pivot + 1; }
    else                    { break; }
  }
}

static void 
msh__aabb_tree_build_nodes( msh_aabb_tree_t *tree, uint32_t node_idx, uint32_t tri_idx, uint32_t n_tris )
{
  if( node_idx >= tree->first_leaf  ) 
  {
    assert( n_tris <= MSH_AABB_TREE_MAX_LEAF_TRIS );
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
  if( extends.data[1] > extends.data[dim] ) dim = 1;
  if( extends.data[2] > extends.data[dim] ) dim = 2;

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

  uint32_t leaf_count = 1;
  while( leaf_count * MSH_AABB_TREE_MAX_LEAF_TRIS < tree->n_tris ) leaf_count <<= 1;

  tree->first_leaf = leaf_count - 1;
  tree->nodes = (msh_aabb_node_t*)malloc( tree->first_leaf * sizeof( msh_aabb_node_t ) );
  tree->leafs = (msh_aabb_leaf_t*)malloc( leaf_count * sizeof( msh_aabb_leaf_t ) );
  tree->leaf_tris = (uint32_t*)malloc( tree->n_tris * sizeof( uint32_t ) );
  for( size_t i = 0; i < tree->first_leaf; ++i )
  {
    mshgeo_aabb_init( &(tree->nodes[i]) );
  }
  tree->n_leafs = 0;
  tree->n_nodes = 0;

  for( uint32_t n = 0; n < tree->n_tris; ++n )
  {
    tree->leaf_tris[ n ] = n;
  }

  msh__aabb_tree_build_nodes( tree, 0, 0, tree->n_tris );
  return 1;

}

// MSHGEODEF int
// mshgeo_ray_aabb_tree_intersect( const msh_ray_t* ray, const msh_aabb_tree_t* tree )
// {

// }



/* deprecated
MSHGEODEF inline msh_bbox_t
mshgeo_bbox_init()
{
  msh_bbox_t bb;
  bb.min_p = msh_vec3(1e9, 1e9, 1e9);
  bb.max_p = msh_vec3(-1e9, -1e9, -1e9);
  return bb;
}

MSHGEODEF inline void
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
*/
#endif