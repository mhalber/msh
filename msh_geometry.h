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

typedef struct mshgeo_bbox
{
  msh_vec3_t min_p;
  msh_vec3_t max_p;
} msh_bbox_t;


/* SIMPLE BBOX */
MSHGEODEF msh_bbox_t mshgeo_bbox_init();
MSHGEODEF void mshgeo_bbox_reset(msh_bbox_t* bbox);
MSHGEODEF void mshgeo_bbox_union(msh_bbox_t* bb, msh_vec3_t p);
MSHGEODEF msh_vec3_t mshgeo_bbox_centroid(msh_bbox_t *bb);
MSHGEODEF float mshgeo_bbox_width(msh_bbox_t *bb);
MSHGEODEF float mshgeo_bbox_height(msh_bbox_t *bb);
MSHGEODEF float mshgeo_bbox_depth(msh_bbox_t *bb);
MSHGEODEF msh_vec3_t mshgeo_bbox_diagonal(msh_bbox_t* bb);
MSHGEODEF float mshgeo_bbox_volume(msh_bbox_t* bb);

#ifdef __cplusplus
}
#endif

#endif /*MSH_GEOMETRY_H*/

#ifdef MSH_GEOMETRY_IMPLEMENTATION

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

#endif