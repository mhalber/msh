/*
  ==============================================================================
  
  MSH_HASH_GRID.H v0.1
  
  A single header library for low-dimensional(2d and 3d) range and nearest neighbor 
  queries.

  To use the library you simply following line once in your project:
  
  #define MSH_HASH_GRID_IMPLEMENTATION
  #include "msh_hash_grid.h"

  Additionally, since this library does do memory allocation, you have an option
  to provide alternate memory allocatio functions, by defining following macros
  prior to inclusion of this file:
  - MSH_HG_MALLOC
  - MSH_HG_MEMSET
  - MSH_HG_CALLOC
  - MSH_HG_REALLOC
  - MSH_HG_FREE

  ==============================================================================
  USAGE:
  This library focuses on the radius search, which is done by first creating
  hashgrid from your input pointset, and then calling search procedure. Both 
  initialization functions 'msh_hash_grid_init_2d' and 'msh_hash_grid_init_3d'
  take in contiguous point buffer and radius as input.

  The search functions take the pointer to previously created 'msh_hash_grid_t'
  object and search descriptor structure, that allows user to specify all search
  related options. The members of 'msh_hash_grid_search_desc':

    float* query_pts     - INPUT: array of query points. Provided and owned by the user
    size_t n_query_pts   - INPUT: size of query points array. Provided by the user.

    float radius         - OPTION: radius within which we wish to find neighbors for each query
    int sort             - OPTION: should the results be sorted from closest to farthest
    size_t max_n_neigh/k - OPTION: maximum number of neighbors allowed for each query.

    float* distances_sq  - OUTPUT: max_n_neigh * n_query_pts matrix of squared distances to neighbors 
                                   of query pts that are within radius. Each row contains up
                                   to max_n_neigh neighbors for i-th query pts. This array is allocated
                                   internally by library, but ownership is then passed to the user.
    int32_t* indices     - OUTPUT: max_n_neigh * n_query_pts array of indices to neighbors of query 
                                   pts that are within radius. Each row contains up
                                   to max_n_neigh neighbors for i-th query pts. This array is allocated
                                   internally by library, but ownership is then passed to the user.
    size_t* n_neighbors  - OUTPUT: n_query_pts array of number of number neighbors found for 
                                   each of query pts. Note that for i-th points we could find less
                                   than max_n_neighbors. This array should be used when iterating over
                                   indices and distances_sq matrices.

    This libray also has knn function implemenation. 
    Depending on how large k is, these queries might not be very fast.

  ==============================================================================
  DEPENDENCIES

    This file requires following c stdlib headers:
      - <stdlib.h>
      - <stdint.h>
      - <string.h>
      - <stdio.h>
      - <stdbool.h>
      - <stddef.h>
    Note that this file will not pull them in automatically to prevent pulling same
    files multiple time. If you do not like this behaviour and want this file to
    pull in c headers, simply define following before including the library:

    #define MSH_HASH_GRID_INCLUDE_HEADERS

  ==============================================================================
  AUTHORS:
    Maciej Halber

  CREDITS:
    Inspiration for single header ply reader:   tinyply    by ddiakopoulos
    Dynamic array based on                      stb.h      by Sean T. Barrett

  ==============================================================================
  TODOs:
  [x] Fix issue when _init function cannot be used if no implementation is declared.
  [x] Optimization - in both knn and radius I need a better way to determine whether I can early out
  [x] Optimization - see if I can simplify the radius search function for small search radii.
         --> Very small gains given the increase in complexity.
  [x] Optimization - spatial locality - sort linear data on bin idx or morton curves
         --> Does not seem to produce improvement. Something else must be dominating the times
         --> Maybe morton curves will be better
  [x] Fix knn search
      [x] Multithread knn
  [ ] Options for creating search tree
      [ ] User specification of number of threads
  [ ] Heap implementation for knn radius
      [ ] Use <algorithm> first
      [ ] Implement own version and compare
  [x] Multithreading
      [x] API for supplying more then a single point
      [x] OpenMP optional support ( if -fopenmp was supplied, sequential otherwise)
      [ ] Replace openMP with portable implementation (subset of c11 API?)
  [x] Params struct for searching
     [ ] Compatibility function?
  [x] Add 2d support on API level
  [ ] Docs
      NOTE(maciej): Remember to tell the user about sensitivity to max_n_neigh, as it is essentially
                    spreading out the memory. Should some functions to query point density be added?
  [ ] Assert proof
  ==============================================================================
*/

#ifndef MSH_HASH_GRID_H
#define MSH_HASH_GRID_H

#if defined(MSH_HASH_GRID_INCLUDE_HEADERS)
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#endif

#ifndef MSH_HG_MALLOC
#define MSH_HG_MALLOC(x) malloc((x))
#endif

#ifndef MSH_HG_MEMSET
#define MSH_HG_MEMSET(x,y,z) memset((x), (y), (z))
#endif

#ifndef MSH_HG_CALLOC
#define MSH_HG_CALLOC(x,y) calloc((x), (y))
#endif

#ifndef MSH_HG_REALLOC
#define MSH_HG_REALLOC(x,y) realloc((x), (y))
#endif

#ifndef MSH_HG_FREE
#define MSH_HG_FREE(x) free((x))
#endif

#if defined(_OPENMP)
#include <omp.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct msh_hash_grid msh_hash_grid_t;

typedef struct msh_hash_grid_search_desc
{
  float* query_pts;
  size_t n_query_pts;

  float* distances_sq;
  int32_t* indices;
  size_t* n_neighbors;

  float radius;
  union
  {
    size_t k;
    size_t max_n_neigh;
  };

  int sort;
} msh_hash_grid_search_desc_t;

void   msh_hash_grid_init_2d( msh_hash_grid_t* hg,
                              const float* pts, const int32_t n_pts, const float radius );

void   msh_hash_grid_init_3d( msh_hash_grid_t* hg,
                              const float* pts, const int32_t n_pts, const float radius );

void   msh_hash_grid_term( msh_hash_grid_t* hg );

size_t msh_hash_grid_radius_search( const msh_hash_grid_t* hg,
                                    msh_hash_grid_search_desc_t* search_desc );

size_t msh_hash_grid_knn_search( const msh_hash_grid_t* hg,
                                 msh_hash_grid_search_desc_t* search_desc );


typedef struct msh_hg_v3
{
  float x, y, z;
} msh_hg_v3_t;

typedef struct msh_hg_v3i
{
  float x, y, z;
  int32_t i;
} msh_hg_v3i_t;

typedef struct msh_hg_bin_data msh_hg__bin_data_t;
typedef struct msh_hg_bin_info msh_hg__bin_info_t;
typedef struct msh_hg_map msh_hg_map_t;

typedef struct msh_hash_grid
{
  size_t width;
  size_t height;
  size_t depth;
  float cell_size;

  msh_hg_v3_t min_pt;
  msh_hg_v3_t max_pt;

  msh_hg_map_t* bin_table;
  msh_hg_v3i_t* data_buffer;
  msh_hg__bin_info_t* offsets;

  int32_t   _slab_size;
  float _inv_cell_size;
  uint8_t _pts_dim;
  uint16_t _num_threads;
  uint32_t max_n_pts_in_bin;
} msh_hash_grid_t;


#ifdef __cplusplus
}
#endif

#endif /* MSH_HASH_GRID_H */





#ifdef MSH_HASH_GRID_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Copy of msh_array
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct msh_hg_array_header
{
  size_t len;
  size_t cap;
} msh_hg_array_hdr_t;

#define msh_hg_array(T) T*

void* msh_hg__array_grow(const void *array, size_t new_len, size_t elem_size);

#define msh_hg_array__grow_formula(x)    ((2*(x)+5))
#define msh_hg_array__hdr(a)             ((msh_hg_array_hdr_t *)((char *)(a) - sizeof(msh_hg_array_hdr_t)))

#define msh_hg_array_len(a)              ((a) ? (msh_hg_array__hdr((a))->len) : 0)
#define msh_hg_array_cap(a)              ((a) ? (msh_hg_array__hdr((a))->cap) : 0)
#define msh_hg_array_front(a)            ((a) ? (a) : NULL)
#define msh_hg_array_back(a)             (msh_hg_array_len((a)) ? ((a) + msh_hg_array_len((a)) - 1 ) : NULL)

#define msh_hg_array_free(a)             ((a) ? (MSH_HG_FREE(msh_hg_array__hdr(a)), (a) = NULL) : 0 )
#define msh_hg_array_fit(a, n)           ((n) <= msh_hg_array_cap(a) ? (0) : ( *(void**)&(a) = msh_hg__array_grow((a), (n), sizeof(*(a))) ))
#define msh_hg_array_push(a, ...)        (msh_hg_array_fit((a), 1 + msh_hg_array_len((a))), (a)[msh_hg_array__hdr(a)->len++] = (__VA_ARGS__))

#define MSH_HG_MAX(a, b) ((a) > (b) ? (a) : (b))
#define MSH_HG_MIN(a, b) ((a) <= (b) ? (a) : (b))
#define MSH_HG_MAX3(a, b, c) MSH_HG_MAX(MSH_HG_MAX(a,b), MSH_HG_MAX(b,c))

////////////////////////////////////////////////////////////////////////////////////////////////////
// Copy of msh_map
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct msh_hg_map
{
  uint64_t* keys;
  uint64_t* vals;
  size_t _len;
  size_t _cap;
} msh_hg_map_t;

uint64_t  msh_hg_hash_uint64( uint64_t x );
void      msh_hg_map_init( msh_hg_map_t* map, uint32_t cap );
void      msh_hg_map_free( msh_hg_map_t* map );
size_t    msh_hg_map_len( msh_hg_map_t* map );
size_t    msh_hg_map_cap( msh_hg_map_t* map );
void      msh_hg_map_insert( msh_hg_map_t* map, uint64_t key, uint64_t val );
uint64_t* msh_hg_map_get( const msh_hg_map_t* map, uint64_t key );

////////////////////////////////////////////////////////////////////////////////////////////////////
// Actual start of implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
#define MSH_HG_INLINE __forceinline
#else
#define MSH_HG_INLINE __attribute__((always_inline, unused)) inline
#endif


MSH_HG_INLINE msh_hg_v3_t
msh_hg__vec3_add( msh_hg_v3_t a, msh_hg_v3_t b )
{
  return (msh_hg_v3_t) { a.x + b.x, a.y + b.y, a.z + b.z };
}

MSH_HG_INLINE msh_hg_v3_t
msh_hg__vec3_sub( msh_hg_v3_t a, msh_hg_v3_t b )
{
  return (msh_hg_v3_t) { a.x - b.x, a.y - b.y, a.z - b.z };
}

typedef struct msh_hg_bin_data
{
  int32_t n_pts;
  msh_hg_v3i_t* data;
} msh_hg__bin_data_t;

typedef struct msh_hg_bin_info
{
  uint32_t offset;
  uint32_t length;
} msh_hg__bin_info_t;

MSH_HG_INLINE uint64_t
msh_hash_grid__bin_pt( const msh_hash_grid_t* hg, uint64_t ix, uint64_t iy, uint64_t iz )
{
  uint64_t bin_idx = iz * hg->_slab_size + iy * hg->width + ix;
  return bin_idx;
}

int uint64_compare( const void * a, const void * b )
{
  return ( *(uint64_t*)a - *(uint64_t*)b );
}



void
msh_hash_grid__init( msh_hash_grid_t* hg,
                     const float* pts, const int32_t n_pts, const int32_t dim,
                     const float radius )
{
  assert(dim == 2 || dim == 3);

  if( hg->_num_threads == 0 )
  {
    #if defined(_OPENMP)
      #pragma omp parallel
      {
        hg->_num_threads = omp_get_num_threads();
      }
    #else
      hg->_num_threads = 1;
    #endif
  }

  hg->_pts_dim = dim;

  // Compute bbox
  hg->min_pt = (msh_hg_v3_t){ .x =  1e9, .y =  1e9, .z =  1e9 };
  hg->max_pt = (msh_hg_v3_t){ .x = -1e9, .y = -1e9, .z = -1e9 };

  for( int i = 0; i < n_pts; ++i )
  {
    const float* pt_ptr = &pts[ dim * i ];

    msh_hg_v3_t pt;
    if( dim == 2 ) { pt = (msh_hg_v3_t){ .x = pt_ptr[0], .y = pt_ptr[1], .z = 0 }; }
    else           { pt = (msh_hg_v3_t){ .x = pt_ptr[0], .y = pt_ptr[1], .z = pt_ptr[2] }; };

    hg->min_pt.x = (hg->min_pt.x > pt.x) ? pt.x : hg->min_pt.x;
    hg->min_pt.y = (hg->min_pt.y > pt.y) ? pt.y : hg->min_pt.y;
    hg->min_pt.z = (hg->min_pt.z > pt.z) ? pt.z : hg->min_pt.z;

    hg->max_pt.x = (hg->max_pt.x < pt.x) ? pt.x : hg->max_pt.x;
    hg->max_pt.y = (hg->max_pt.y < pt.y) ? pt.y : hg->max_pt.y;
    hg->max_pt.z = (hg->max_pt.z < pt.z) ? pt.z : hg->max_pt.z;

  }

  // Calculate dimensions
  float dim_x   = (hg->max_pt.x - hg->min_pt.x);
  float dim_y   = (hg->max_pt.y - hg->min_pt.y);
  float dim_z   = (hg->max_pt.z - hg->min_pt.z);
  float max_dim = MSH_HG_MAX3( dim_x, dim_y, dim_z );

  // Calculate cell size
  if( radius > 0.0 ) { hg->cell_size = 2.0f * radius; }
  else               { hg->cell_size = max_dim / (32 * sqrtf(3.0f)); }

  hg->width     = (int)(dim_x / hg->cell_size + 1.0);
  hg->height    = (int)(dim_y / hg->cell_size + 1.0) ;
  hg->depth     = (int)(dim_z / hg->cell_size + 1.0) ;
  hg->_inv_cell_size = 1.0f / hg->cell_size;
  hg->_slab_size = hg->height * hg->width;

  // Create hash table
  hg->bin_table = (msh_hg_map_t*)MSH_HG_CALLOC( 1, sizeof(msh_hg_map_t) );
  msh_hg_map_init( hg->bin_table, 1024 );
  msh_hg_array( msh_hg__bin_data_t ) bin_table_data = 0;
  uint64_t n_bins = 0;
  for( int i = 0 ; i < n_pts; ++i )
  {
    const float* pt_ptr = &pts[ dim * i ];
    msh_hg_v3i_t pt_data;
    if( dim == 2 )
    {
      pt_data = (msh_hg_v3i_t){ .x = pt_ptr[0], .y = pt_ptr[1], .z = 0, .i = i };
    }
    else
    {
      pt_data = (msh_hg_v3i_t){ .x = pt_ptr[0], .y = pt_ptr[1], .z = pt_ptr[2], .i = i };
    }

    uint64_t ix = (uint64_t)( ( pt_data.x - hg->min_pt.x ) * hg->_inv_cell_size );
    uint64_t iy = (uint64_t)( ( pt_data.y - hg->min_pt.y ) * hg->_inv_cell_size );
    uint64_t iz = (uint64_t)( ( pt_data.z - hg->min_pt.z ) * hg->_inv_cell_size );

    uint64_t bin_idx = msh_hash_grid__bin_pt( hg, ix, iy, iz );

    // NOTE(maciej): In msh_map we can't have 0 as key
    uint64_t* bin_table_idx = msh_hg_map_get( hg->bin_table, bin_idx );

    if( bin_table_idx )
    {
      bin_table_data[*bin_table_idx].n_pts += 1;
      msh_hg_array_push( bin_table_data[*bin_table_idx].data, pt_data );
    }
    else
    {
      msh_hg_map_insert( hg->bin_table, bin_idx, n_bins );

      msh_hg__bin_data_t new_bin = {0};
      new_bin.n_pts = 1;
      msh_hg_array_push( new_bin.data, pt_data );
      msh_hg_array_push( bin_table_data, new_bin );
      n_bins++;
    }
  }

  // Prepare storage for linear data
  hg->offsets     = (msh_hg__bin_info_t*)MSH_HG_MALLOC( n_bins * sizeof(msh_hg__bin_info_t) );
  hg->data_buffer = (msh_hg_v3i_t*)MSH_HG_MALLOC( n_pts * sizeof( msh_hg_v3i_t ) );
  MSH_HG_MEMSET( hg->offsets, 0, n_bins * sizeof(msh_hg__bin_info_t) );

  // Gather indices of bins that have data in them from hash table
  msh_array( uint64_t ) filled_bin_indices = {0};
  for( size_t i = 0; i < msh_hg_map_cap(hg->bin_table); ++i )
  {
    // Remember that msh_hg_map internally increments the index, so we need to decrement it here.
    if( hg->bin_table->keys[i] )
    {
      msh_array_push( filled_bin_indices, hg->bin_table->keys[i] - 1);
    }
  }
  qsort( filled_bin_indices, msh_hg_array_len( filled_bin_indices ), sizeof(uint64_t), uint64_compare );

  // Now lay the data into an array based on the sorted keys (following fill order)
  // TODO(maciej): Morton ordering?
  hg->max_n_pts_in_bin = 0;
  uint32_t offset = 0;
  for( size_t i = 0; i < msh_hg_array_len(filled_bin_indices); ++i )
  {
    uint64_t* bin_index = msh_hg_map_get( hg->bin_table, filled_bin_indices[i] );
    assert( bin_index );
    msh_hg__bin_data_t* bin = &bin_table_data[ *bin_index ];
    assert( bin );
    uint32_t n_bin_pts = bin->n_pts;
    hg->max_n_pts_in_bin = MSH_HG_MAX( n_bin_pts, hg->max_n_pts_in_bin );
    for( uint32_t j = 0; j < n_bin_pts; ++j )
    {
      hg->data_buffer[ offset + j ] = bin->data[j] ;
    }
    hg->offsets[ *bin_index ] = (msh_hg__bin_info_t) { .offset = offset, .length = n_bin_pts };
    offset += n_bin_pts;
  }


  // Clean-up temporary data
  for( size_t i = 0; i < n_bins; ++i )
  {
    msh_hg_array_free( bin_table_data[i].data );
  }
  msh_hg_array_free( bin_table_data );
}

void
msh_hash_grid_init_2d( msh_hash_grid_t* hg,
                       const float* pts, const int32_t n_pts, const float radius)
{
  msh_hash_grid__init( hg, pts, n_pts, 2, radius );
}

void
msh_hash_grid_init_3d( msh_hash_grid_t* hg,
                       const float* pts, const int32_t n_pts, const float radius)
{
  msh_hash_grid__init( hg, pts, n_pts, 3, radius );
}


void
msh_hash_grid_term( msh_hash_grid_t* hg )
{
  hg->width          = 0;
  hg->height         = 0;
  hg->depth          = 0;
  hg->cell_size      = 0.0f;
  hg->min_pt         = (msh_hg_v3_t){ 0.0f, 0.0f, 0.0f };
  hg->max_pt         = (msh_hg_v3_t){ 0.0f, 0.0f, 0.0f };
  hg->_slab_size     = 0.0f;
  hg->_inv_cell_size = 0.0f;

  MSH_HG_FREE( hg->data_buffer ); hg->data_buffer = NULL;
  MSH_HG_FREE( hg->offsets );     hg->offsets = NULL;
  MSH_HG_FREE( hg->bin_table );   hg->bin_table = NULL;
}


// NOTE(maciej): This implementation is a special case modification of a templated
// sort by Sean T. Barret from stb.h. We simply want to allow sorting both the indices
// and distances if user requested returning sorted results.
void
msh_hash_grid__ins_sort( float *dists, int32_t* indices, int n )
{
   int i = 0;
   int j = 0;
   for( i = 1; i < n; ++i )
   {
      float da   = dists[i];
      int32_t ia = indices[i];
      j = i;
      while( j > 0 )
      {
        float db = dists[j-1];
        if( da >= db ) { break; }
        dists[j] = dists[j-1];
        indices[j] = indices[j-1];
        --j;
      }
      if (i != j)
      {
        dists[j] = da;
        indices[j] = ia;
      }
   }
}

void
msh_hash_grid__quick_sort( float *dists, int32_t* indices, int n )
{
   // threshold for transitioning to insertion sort
   while( n > 12 )
   {
      float da, db, dt;
      int32_t it = 0;
      int32_t c01, c12, c, m, i, j;

      // compute median of three
      m = n >> 1;
      da = dists[0];
      db = dists[m];
      c = da < db;
      c01 = c;
      da = dists[m];
      db = dists[n-1];
      c = da < db;
      c12 = c;
      // if 0 >= mid >= end, or 0 < mid < end, then use mid
      if( c01 != c12 )
      {
         // otherwise, we'll need to swap something else to middle
         int32_t z;
         da = dists[0];
         db = dists[n-1];
         c = da < db;
         // 0>mid && mid<n:  0>n => n; 0<n => 0
         // 0<mid && mid>n:  0>n => 0; 0<n => n
         z = (c == c12) ? 0 : n-1;
         dt = dists[z];
         dists[z] = dists[m];
         dists[m] = dt;
         it = indices[z];
         indices[z] = indices[m];
         indices[m] = it;
      }
      // now dists[m] is the median-of-three  swap it to the beginning so it won't move around
      dt = dists[0];
      dists[0] = dists[m];
      dists[m] = dt;
      it = indices[0];
      indices[0] = indices[m];
      indices[m] = it;

      // partition loop
      i=1;
      j=n-1;
      for(;;)
      {
         // handling of equality is crucial here for sentinels & efficiency with duplicates
         db = dists[0];
         for( ;;++i )
         {
            da = dists[i];
            c = da < db;
            if (!c) break;
         }
         da = dists[0];
         for( ;;--j ) {
            db = dists[j];
            c = da < db;
            if (!c) break;
         }
         // make sure we haven't crossed
         if( i >= j ) { break; }
         dt = dists[i];
         dists[i] = dists[j];
         dists[j] = dt;
         it = indices[i];
         indices[i] = indices[j];
         indices[j] = it;

         ++i;
         --j;
      }
      // recurse on smaller side, iterate on larger
      if( j < (n-i) )
      {
         msh_hash_grid__quick_sort( dists, indices, j );
         dists = dists + i;
         indices = indices + i;
         n = n - i;
      }
      else
      {
         msh_hash_grid__quick_sort( dists + i, indices + i, n - i );
         n = j;
      }
   }
}

void msh_hash_grid__sort( float* dists, int32_t* indices, int n )
{
  msh_hash_grid__quick_sort( dists, indices, n );
  msh_hash_grid__ins_sort( dists, indices, n );
}

typedef struct msh_hash_grid_dist_storage
{
  size_t    cap;
  size_t    len;
  int32_t   max_dist_idx;
  real32_t  max_dist;
  real32_t* dists;
  int32_t*  indices;
  int32_t   is_heap;
} msh_hash_grid_dist_storage_t;

void
msh_hash_grid_dist_storage_init( msh_hash_grid_dist_storage_t* q,
                                 const int k, float* dists, int32_t* indices )
{
  q->cap          = k;
  q->len          = 0;
  q->max_dist_idx = -1;
  q->max_dist     = MSH_F32_MAX;
  q->is_heap      = 0;
  q->dists        = dists;
  q->indices      = indices;
}

void
msh_hash_grid_dist_storage_push_heap( msh_hash_grid_dist_storage_t* q,
                                      const float dist, const int32_t idx )
{
  if( dist >= q->max_dist ) { return; }

  if( q->len >= q->cap ) 
  {
    // if result set is filled to capacity, remove farthest element
    std::pop_heap( q->dists, q->dists + q->len );
    // std::pop_heap( q->indices, q->indices + q->len );
    q->len--; // "remove the farthest"
  }

  // add new element
  q->dists[ q->len ] = dist;
  q->indices[ q->len ] = idx;

  // book keep the index of max dist
  // POSSIBLY REMOVE
  if( q->is_heap )
  {
    if( q->max_dist_idx != -1 )
    {
      if( q->dists[ q->max_dist_idx ] < dist ) { q->max_dist_idx = q->len; }
    }
    else
    {
      q->max_dist_idx = q->len;
    }
  }

  if( q->is_heap) 
  {
    std::push_heap( q->dists, q->dists + q->len + 1 );
    // std::push_heap( q->indices, q->indices + q->len + 1 );
  }

  q->len++;

  if( q->len >= q->cap ) 
  {
    // when got to full capacity, make it a heap
    if( !q->is_heap ) 
    {
      std::make_heap( q->dists, q->dists + q->len + 1 );
      // std::make_heap( q->indices, q->indices + q->len + 1 );
      q->is_heap = 1;
    }
    
    // we replaced the farthest element, update worst distance
    q->max_dist = q->dists[0];
    q->max_dist_idx = 0;
  }
}

void
msh_hash_grid_dist_storage_push_linear( msh_hash_grid_dist_storage_t* q,
                                 const float dist, const int32_t idx )
{
  // We have storage left
  if( q->len < q->cap )
  {
    // Push new element
    q->dists[q->len]   = dist;
    q->indices[q->len] = idx;

    // book keep the index of max dist
    if( q->max_dist_idx != -1 )
    {
      if( q->dists[ q->max_dist_idx ] < dist ) { q->max_dist_idx = q->len; }
    }
    else
    {
      q->max_dist_idx = q->len;
    }
    q->len++;
  }
  // We are at capacity.
  else
  {
    // Replace if new element hash smaller distance than current dist
    if( q->dists[q->max_dist_idx] > dist )
    {
      q->dists[q->max_dist_idx]   = dist;
      q->indices[q->max_dist_idx] = idx;

      // Make sure we are really looking at the highest element after replacement
      q->max_dist = q->dists[q->max_dist_idx];
      for( size_t i = 0; i < q->len; ++i )
      {
        if( q->dists[i] > q->max_dist ) 
        { 
          q->max_dist_idx = i; 
          q->max_dist = q->dists[q->max_dist_idx];
        }
      }
    }
  }
}

void
msh_hash_grid__find_neighbors_in_bin( const msh_hash_grid_t* hg, const uint64_t bin_idx,
                                      const float radius_sq, const float* pt,
                                      msh_hash_grid_dist_storage_t* s )
{
  
  // issue this whole things stops working if we use doubles.
  uint64_t* bin_table_idx = msh_hg_map_get( hg->bin_table, bin_idx );
  if( !bin_table_idx ) { return; }

  msh_hg__bin_info_t bi = hg->offsets[ *bin_table_idx ];
  uint32_t n_pts = bi.length;
  const msh_hg_v3i_t* data = &hg->data_buffer[bi.offset];

  float px = pt[0];
  float py = pt[1];
  float pz = (hg->_pts_dim == 2 ) ? 0.0 : pt[2];

  for( uint32_t i = 0; i < n_pts; ++i )
  {
    // TODO(maciej): Maybe SSE?
    float   dix = data[i].x;
    float   diy = data[i].y;
    float   diz = data[i].z;
    int32_t dii = data[i].i;

    float vx = dix - px;
    float vy = diy - py;
    float vz = diz - pz;
    float dist_sq = vx * vx + vy * vy + vz * vz;

    if( dist_sq < radius_sq )
    {
      msh_hash_grid_dist_storage_push_linear( s, dist_sq, dii );
    }
  }
}

size_t msh_hash_grid_radius_search( const msh_hash_grid_t* hg,
                                    msh_hash_grid_search_desc_t* hg_sd )
{
  assert( hg_sd->query_pts );
  assert( hg_sd->distances_sq );
  assert( hg_sd->indices );
  assert( hg_sd->radius > 0.0 );
  assert( hg_sd->n_query_pts > 0 );
  assert( hg_sd->max_n_neigh > 0 );

  // Unpack the some useful data from structs
  enum { MAX_BIN_COUNT = 128, MAX_THREAD_COUNT = 128 };
  uint32_t n_query_pts = hg_sd->n_query_pts;
  size_t row_size      = hg_sd->max_n_neigh;
  float radius         = hg_sd->radius;
  uint64_t slab_size   = hg->_slab_size;
  float cs             = hg->cell_size;
  float ics            = hg->_inv_cell_size;
  int64_t w            = hg->width;
  int64_t h            = hg->height;
  int64_t d            = hg->depth;
  float radius_sq      = radius * radius;

  uint32_t n_pts_per_thread = n_query_pts;
  uint32_t total_num_neighbors = 0;
  uint32_t num_neighbors_per_thread[MAX_THREAD_COUNT] = {0};
  uint32_t num_threads = hg->_num_threads;
  assert( num_threads <= MAX_THREAD_COUNT );

#if defined(_OPENMP)
  #pragma omp parallel num_threads( num_threads )
  {
    if( n_query_pts < num_threads ) { num_threads = n_query_pts; }
    n_pts_per_thread = ceilf((float)n_query_pts / num_threads);
    uint32_t thread_idx = omp_get_thread_num();
#else
  for( uint32_t thread_idx = 0; thread_idx < num_threads; ++thread_idx )
  {
#endif
    if( thread_idx < num_threads )
    {
      uint32_t low_lim      = thread_idx * n_pts_per_thread;
      uint32_t high_lim     = MSH_HG_MIN((thread_idx + 1) * n_pts_per_thread, n_query_pts);
      uint32_t cur_n_pts    = high_lim - low_lim;

      float* query_pt       = hg_sd->query_pts + low_lim * hg->_pts_dim;
      size_t* n_neighbors   = hg_sd->n_neighbors ? (hg_sd->n_neighbors + low_lim) : NULL;
      float* dists_sq       = hg_sd->distances_sq + (low_lim * row_size);
      int32_t* indices      = hg_sd->indices + (low_lim * row_size);

      int32_t bin_indices[ MAX_BIN_COUNT ];
      float bin_dists_sq[ MAX_BIN_COUNT ];
      msh_hash_grid_dist_storage_t storage;

      for( uint32_t pt_idx = 0; pt_idx < cur_n_pts; ++pt_idx )
      {
        // Prep the storage for the next point
        msh_hash_grid_dist_storage_init( &storage, row_size, dists_sq, indices );

        // Normalize query pt with respect to grid
        msh_hg_v3_t q;
        if( hg->_pts_dim == 2 )
        {
          q = (msh_hg_v3_t) { query_pt[0] - hg->min_pt.x,
                              query_pt[1] - hg->min_pt.y,
                              0.0 };
        }
        else
        {
          q = (msh_hg_v3_t) { query_pt[0] - hg->min_pt.x,
                              query_pt[1] - hg->min_pt.y,
                              query_pt[2] - hg->min_pt.z };
        }

        // Get base bin idx for query pt
        int64_t ix = (int64_t)( q.x * ics );
        int64_t iy = (int64_t)( q.y * ics );
        int64_t iz = (int64_t)( q.z * ics );

        // Decide where to look
        int64_t px  = (int64_t)( (q.x + radius) * ics );
        int64_t nx  = (int64_t)( (q.x - radius) * ics );
        int64_t opx = px - ix;
        int64_t onx = nx - ix;

        int64_t py  = (int64_t)( (q.y + radius) * ics );
        int64_t ny  = (int64_t)( (q.y - radius) * ics );
        int64_t opy = py - iy;
        int64_t ony = ny - iy;

        int64_t pz  = (int64_t)( (q.z + radius) * ics );
        int64_t nz  = (int64_t)( (q.z - radius) * ics );
        int64_t opz = pz - iz;
        int64_t onz = nz - iz;

        uint32_t n_visited_bins = 0;
        float dx, dy, dz;
        int64_t cx, cy, cz;
        for( int64_t oz = onz; oz <= opz; ++oz )
        {
          cz = (int64_t)iz + oz;
          if( cz < 0 || cz >= d ) { continue; }
          uint64_t idx_z = cz * slab_size;

          if( oz < 0 )      { dz = q.z - (cz + 1) * cs; }
          else if( oz > 0 ) { dz = cz * cs - q.z; }
          else              { dz = 0.0f; }

          for( int64_t oy = ony; oy <= opy; ++oy )
          {
            cy = iy + oy;
            if( cy < 0 || cy >= h ) { continue; }
            uint64_t idx_y = cy * w;

            if( oy < 0 )      { dy = q.y - (cy + 1) * cs; }
            else if( oy > 0 ) { dy = cy * cs - q.y; }
            else              { dy = 0.0f; }

            for( int64_t ox = onx; ox <= opx; ++ox )
            {
              cx = ix + ox;
              if( cx < 0 || cx >= w ) { continue; }

              assert( n_visited_bins < MAX_BIN_COUNT );

              bin_indices[n_visited_bins] = idx_z + idx_y + cx;

              if( ox < 0 )      { dx = q.x - (cx + 1) * cs; }
              else if( ox > 0 ) { dx = cx * cs - q.x; }
              else              { dx = 0.0f; }

              bin_dists_sq[n_visited_bins] = dz * dz + dy * dy + dx * dx;
              n_visited_bins++;
            }
          }
        }

        msh_hash_grid__sort( bin_dists_sq, bin_indices, n_visited_bins );

        for( uint32_t i = 0; i < n_visited_bins; ++i )
        {
          msh_hash_grid__find_neighbors_in_bin( hg, bin_indices[i], radius_sq, query_pt, &storage );
          if( storage.len >= row_size &&
              dists_sq[ storage.max_dist_idx ] <= bin_dists_sq[i] )
          {
            break;
          }
        }

        if( hg_sd->sort ) { msh_hash_grid__sort( dists_sq, indices, storage.len ); }

        if( n_neighbors ) { (*n_neighbors++) = storage.len; }
        num_neighbors_per_thread[thread_idx] += storage.len;

        // Advance pointers
        dists_sq += row_size;
        indices  += row_size;
        query_pt += hg->_pts_dim;
      }
    }
  }

  for( uint32_t i = 0 ; i < num_threads; ++i )
  {
    total_num_neighbors += num_neighbors_per_thread[i];
  }

  return total_num_neighbors;
}




MSH_HG_INLINE void
msh_hash_grid__add_bin_contents( const msh_hash_grid_t* hg, const uint64_t bin_idx,
                                 const float* pt, msh_hash_grid_dist_storage_t* s )
{
  uint64_t* bin_table_idx = msh_hg_map_get( hg->bin_table, bin_idx );

  if( !bin_table_idx ) { return; }
  msh_hg__bin_info_t bi = hg->offsets[*bin_table_idx];
  int n_pts = bi.length;
  const msh_hg_v3i_t* data = &hg->data_buffer[bi.offset];

  for( int32_t i = 0; i < n_pts; ++i )
  {

    msh_hg_v3_t v;
    if( hg->_pts_dim == 2 )
    {
      v = (msh_hg_v3_t){ data[i].x - pt[0], data[i].y - pt[1], 0.0 };
    }
    else
    {
      v = (msh_hg_v3_t){ data[i].x - pt[0], data[i].y - pt[1], data[i].z - pt[2] };
    }
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;

    msh_hash_grid_dist_storage_push_linear( s, dist_sq, data[i].i );

  }
  
}

size_t msh_hash_grid_knn_search( const msh_hash_grid_t* hg,
                                 msh_hash_grid_search_desc_t* hg_sd )
{
  assert( hg_sd->query_pts );
  assert( hg_sd->distances_sq );
  assert( hg_sd->indices );
  assert( hg_sd->n_query_pts > 0 );
  assert( hg_sd->max_n_neigh > 0 );
  assert( hg_sd->k > 0 );

  // Unpack the some useful data from structs
  enum { MAX_BIN_COUNT = 128, MAX_THREAD_COUNT = 128 };
  uint32_t n_query_pts = hg_sd->n_query_pts;
  uint32_t k           = hg_sd->k;
  uint64_t slab_size   = hg->_slab_size;
  int8_t sort          = hg_sd->sort;
  float cs             = hg->cell_size;
  int64_t w            = hg->width;
  int64_t h            = hg->height;
  int64_t d            = hg->depth;

  uint32_t n_pts_per_thread = n_query_pts;
  uint32_t total_num_neighbors = 0;
  uint32_t num_neighbors_per_thread[MAX_THREAD_COUNT] = {0};
  uint32_t num_threads = hg->_num_threads;
  assert( num_threads <= MAX_THREAD_COUNT );
#if defined(_OPENMP)
  #pragma omp parallel
  {
    if( n_query_pts < num_threads ) { num_threads = n_query_pts; }
    uint32_t thread_idx = omp_get_thread_num();
#else
  for( uint32_t thread_idx = 0; thread_idx < num_threads; ++thread_idx )
  {
#endif
    if( thread_idx < num_threads )
    {
      uint32_t low_lim      = thread_idx * n_pts_per_thread;
      uint32_t high_lim     = MSH_HG_MIN((thread_idx + 1) * n_pts_per_thread, n_query_pts);
      uint32_t cur_n_pts    = high_lim - low_lim;

      float *query_pt       = hg_sd->query_pts + low_lim * hg->_pts_dim;
      size_t* n_neighbors   = hg_sd->n_neighbors ? (hg_sd->n_neighbors + low_lim) : NULL;
      float* dists_sq       = hg_sd->distances_sq + (low_lim * k);
      int32_t* indices      = hg_sd->indices + (low_lim * k);

      int32_t bin_indices[ MAX_BIN_COUNT ];
      msh_hash_grid_dist_storage_t storage;
      
      for( uint32_t pt_idx = 0; pt_idx < cur_n_pts; ++pt_idx )
      {
        // Prep the storage for the next point
        msh_hash_grid_dist_storage_init( &storage, k, dists_sq, indices );

        // Normalize query pt with respect to grid
        float dx, dy, dz;
        int64_t cx, cy, cz;
        int32_t layer = 0;
        int8_t should_break = 0;

        msh_hg_v3_t pt_prime;
        if( hg->_pts_dim == 2 )
        {
          pt_prime = (msh_hg_v3_t) { query_pt[0] - hg->min_pt.x,
                                     query_pt[1] - hg->min_pt.y,
                                     0.0 };
        }
        else
        {
          pt_prime = (msh_hg_v3_t) { query_pt[0] - hg->min_pt.x,
                                     query_pt[1] - hg->min_pt.y,
                                     query_pt[2] - hg->min_pt.z };
        }
        // get base bin for query
        uint64_t ix = (uint64_t)( (pt_prime.x) * hg->_inv_cell_size );
        uint64_t iy = (uint64_t)( (pt_prime.y) * hg->_inv_cell_size );
        uint64_t iz = (uint64_t)( (pt_prime.z) * hg->_inv_cell_size );
        while( true )
        {
          int32_t inc_x = 1;
          uint32_t n_visited_bins = 0;
          for( int64_t oz = -layer; oz <= layer; oz++ )
          {
            cz = iz + oz;
            if( cz < 0 || cz >= d ) continue;
            uint64_t idx_z = cz * slab_size;

            if( oz < 0 )      { dz = pt_prime.z - (cz + 1) * cs; }
            else if( oz > 0 ) { dz = cz * cs - pt_prime.z; }
            else              { dz = 0.0f; }

            for( int64_t oy = -layer; oy <= layer; oy++ )
            {
              cy = iy + oy;
              if( cy < 0 || cy >= h ) continue;
              uint64_t idx_y = cy * w;

              if( oy < 0 )      { dy = pt_prime.y - (cy + 1) * cs; }
              else if( oy > 0 ) { dy = cy * cs - pt_prime.y; }
              else              { dy = 0.0f; }

              if( abs(oy) != layer && abs(oz) != layer ) { inc_x = 2 * layer; }
              else                                       { inc_x = 1; }

              for( int64_t ox = -layer; ox <= layer; ox += inc_x )
              {
                cx = ix + ox;
                if( cx < 0 || cx >= w ) continue;

                if( ox < 0 )      { dx = pt_prime.x - (cx + 1) * cs; }
                else if( ox > 0 ) { dx = cx * cs - pt_prime.x; }
                else              { dx = 0.0f; }

                float dist_sq = dz * dz + dy * dy + dx * dx;

                if( storage.len >= k &&
                    dist_sq > dists_sq[storage.max_dist_idx] ) { continue; }

                assert( n_visited_bins < MAX_BIN_COUNT );

                bin_indices[n_visited_bins]  = idx_z + idx_y + cx;

                n_visited_bins++;
              }
            }
          }

          for( uint32_t bin_idx = 0; bin_idx < n_visited_bins; ++bin_idx )
          {
            msh_hash_grid__add_bin_contents( hg, bin_indices[bin_idx], query_pt, &storage );
          }
          
          layer++;
          if( should_break ) { break; }
          if( storage.len >= k ) { should_break = true; }
        }
        if( n_neighbors ) { (*n_neighbors++) = storage.len; }
        num_neighbors_per_thread[thread_idx] += storage.len;

        if( sort ) { msh_hash_grid__sort( dists_sq, indices, storage.len ); }
        // Advance pointers
        dists_sq += k;
        indices  += k;
        query_pt += hg->_pts_dim;
      }
    }
  }

  for( uint32_t i = 0 ; i < num_threads; ++i )
  {
    total_num_neighbors += num_neighbors_per_thread[i];
  }

  return total_num_neighbors;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// msh_array / msh_hg_map implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

void*
msh_hg__array_grow(const void *array, size_t new_len, size_t elem_size) {
  size_t old_cap = msh_hg_array_cap( array );
  size_t new_cap = (size_t)msh_hg_array__grow_formula( old_cap );
  new_cap = (size_t)MSH_HG_MAX( new_cap, MSH_HG_MAX(new_len, 16) );
  size_t new_size = sizeof(msh_hg_array_hdr_t) + new_cap * elem_size;
  msh_hg_array_hdr_t *new_hdr;

  if( array ) {
    new_hdr = (msh_hg_array_hdr_t*)MSH_HG_REALLOC( msh_hg_array__hdr( array ), new_size );
  } else {
    new_hdr = (msh_hg_array_hdr_t*)MSH_HG_MALLOC( new_size );
    new_hdr->len = 0;
  }
  new_hdr->cap = new_cap;
  return (void*)((char*)new_hdr + sizeof(msh_hg_array_hdr_t));
}

MSH_HG_INLINE uint64_t
msh_hg_hash_uint64(uint64_t x)
{
  x *= 0xff51afd7ed558ccd;
  x ^= x >> 32;
  return x;
}


size_t
msh_hg_map__pow2ceil( uint32_t v )
{
  --v;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  ++v;
  v += ( v == 0 );
  return v;
}

void
msh_hg_map_init( msh_hg_map_t* map, uint32_t cap )
{
  assert( !map->keys && !map->vals );
  cap = msh_hg_map__pow2ceil( cap );
  map->keys = (uint64_t*)MSH_HG_CALLOC( cap, sizeof(uint64_t) );
  map->vals = (uint64_t*)MSH_HG_MALLOC( cap * sizeof(uint64_t) );
  map->_len = 0;
  map->_cap = cap;
}

void
msh_hg_map__grow( msh_hg_map_t *map, size_t new_cap) {
  new_cap = msh_max( new_cap, 16 );
  msh_hg_map_t new_map;
  new_map.keys = (uint64_t*)MSH_HG_CALLOC( new_cap, sizeof(uint64_t) );
  new_map.vals = (uint64_t*)MSH_HG_MALLOC( new_cap * sizeof(uint64_t) );
  new_map._len = 0;
  new_map._cap = new_cap;

  for( size_t i = 0; i < map->_cap; i++ )
  {
    if( map->keys[i] )
    {
      msh_hg_map_insert( &new_map, map->keys[i] - 1, map->vals[i] );
    }
  }
  MSH_HG_FREE( (void *)map->keys );
  MSH_HG_FREE( map->vals );
  *map = new_map;
}

size_t
msh_hg_map_len( msh_hg_map_t* map )
{
  return map->_len;
}

size_t
msh_hg_map_cap( msh_hg_map_t* map )
{
  return map->_cap;
}

void
msh_hg_map_insert( msh_hg_map_t* map, uint64_t key, uint64_t val )
{
  key += 1;
  if( 2 * map->_len >= map->_cap) { msh_hg_map__grow( map, 2 * map->_cap ); }
  assert( 2 * map->_len < map->_cap );
  size_t i = (size_t)key;
  for (;;)
  {
    i &= map->_cap - 1;
    if( !map->keys[i] )
    {
      map->_len++;
      map->keys[i] = key;
      map->vals[i] = val;
      return;
    }
    else if( map->keys[i] == key )
    {
      map->vals[i] = val;
      return;
    }
    i++;
  }
}

uint64_t*
msh_hg_map_get( const msh_hg_map_t* map, uint64_t key )
{
  if (map->_len == 0) { return NULL; }
  key += 1;
  size_t i = (size_t)key;
  assert(map->_len < map->_cap);
  for (;;) {
    i &= map->_cap - 1;
    if( map->keys[i] == key )
    {
      return &map->vals[i];
    }
    else if( !map->keys[i] )
    {
      return NULL;
    }
    i++;
  }
}

void
msh_hg_map_free( msh_hg_map_t* map )
{
  MSH_HG_FREE( map->keys );
  MSH_HG_FREE( map->vals );
  map->_cap = 0;
  map->_len = 0;
}

#endif /* MSH_HASH_GRID_IMPLEMENTATION */