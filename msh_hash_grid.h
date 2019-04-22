/*
  ==============================================================================
  
  MSH_HASH_GRID.H v0.5
  
  A single header library for low-dimensional(2d and 3d) range and nearest neighbor queries.

  To use the library you simply add:
    #define MSH_HASH_GRID_IMPLEMENTATION
    #include "msh_hash_grid.h"

  ==============================================================================
  API DOCUMENTATION

  This library focuses on the radius search, which is done by first creating
  hash grid from your input pointset, and then calling search procedure. 

  It is important to note that this library produces search structures that are initialization
  depended - the radius supplied during initialization should be close to the radius used in
  search queries

  Customization
  -------------
  'msh_hash_grid.h' performs dynamic memory allocation. You have an option to provide alternate memory
  allocation functions, by defining following macros prior to inclusion of this file:
    - MSH_HG_MALLOC
    - MSH_HG_MEMSET
    - MSH_HG_CALLOC
    - MSH_HG_REALLOC
    - MSH_HG_FREE

  msh_hash_grid_init_2d
  ---------------------
    void msh_hash_grid_init_2d( msh_hash_grid_t* hg,
                                const float* pts, const int32_t n_pts, const float radius );

  Initializes the 2d hash grid 'hg' using the data passed in 'pts' where the cell size is
  selected to best serve queries with 'radius' search distance. 'pts' is expected to
  be continuous array of 2d point corrdinates.

  msh_hash_grid_init_3d
  ---------------------
    void msh_hash_grid_init_3d( msh_hash_grid_t* hg,
                                const float* pts, const int32_t n_pts, const float radius );

  Initializes the 3d hash grid 'hg' using the data passed in 'pts' where the cell size is
  selected to best serve queries with 'radius' search distance. 'pts' is expected to
  be continuous array of 3d point corrdinates.

  msh_hash_grid_term
  ---------------------
    void msh_hash_grid_term( msh_hash_grid_t* hg );
  
  Terminates storage for grid 'hg'. 'hg' should not be used after this call.
  
  
  msh_hash_grid_radius_search
  ---------------------
    size_t msh_hash_grid_radius_search( const msh_hash_grid_t* hg,
                                    msh_hash_grid_search_desc_t* search_desc );

  Performs radius search using 'hg' as acceleration structure, with search queries described
  in 'search_desc'. Returns the total number of neighbors found. The members of 
  'msh_hash_grid_search_desc_t' are:

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
  
  Note that when doing searches, 'max_n_neigh' parameter is important - set it too low and
  you will not find all neighbors within the radius (the k returned will still be the k closest though).
  Set it too high, and there will be a decent amount of memory wasting and cache misses.

  msh_hash_grid_knn_search
  ---------------------
    size_t msh_hash_grid_knn_search( const msh_hash_grid_t* hg,
                                   msh_hash_grid_search_desc_t* search_desc );

  Exactly the same as 'msh_hash_grid_radius_search', except search will be performed until
  'k' (specified in 'search_desc') neighbors will be found.  Depending on how large 'k' is, 
  these queries might not be very fast.

  ==============================================================================
  DEPENDENCIES

    This file requires following c stdlib headers:
      - <stdlib.h>
      - <stdint.h>
      - <string.h>
      - <stdio.h>
      - <stdbool.h>
      - <stddef.h>

    Note that this file will not pull them in. This is to prevent pulling the same
    files multiple times, especially within single compilation unit. 
    To actually include the headers, simply define following before including the library:

    #define MSH_HASH_GRID_INCLUDE_HEADERS

  ==============================================================================
  AUTHORS:
    Maciej Halber

  CREDITS:
    Map implementation based on                 bitwise    by Per Vogsnen
    Dynamic array based on                      stb.h      by Sean T. Barrett

  Licensing information can be found at the end of the file.
  ==============================================================================
  TODOs:
  [ ] Replace openMP with a custom threading/scheduler implementation
  [ ] Compatibility function
    [ ] Allow user to specify compatibility function instead of just L2 norm
    [ ] Allow user to provide some extra user data like normals for computing the distances
    [ ] Write ICP example + visualization to test this. Question how well that helps icp converge
  [ ] Assert proof
  [ ] Docs
  [x] Fix issue when _init function cannot be used if no implementation is declared.
  [x] Optimization - in both knn and radius I need a better way to determine whether I can early out
  [x] Optimization - see if I can simplify the radius search function for small search radii.
         --> Very small gains given the increase in complexity.
  [x] Optimization - spatial locality - sort linear data on bin idx or morton curves
         --> Does not seem to produce improvement. Something else must be dominating the times
         --> Maybe morton curves will be better
  [x] Fix knn search
      [x] Multithread knn
  [x] Heap implementation for knn radius
      [x] Use <algorithm> first
      [x] Implement own version and compare
  [x] Multithreading
      [x] API for supplying more then a single point
      [x] OpenMP optional support ( if -fopenmp was supplied, sequential otherwise)
  [x] Params struct for searching
  [x] Add 2d support on API level
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
#ifdef MSH_JOBS
  msh_jobs_ctx_t* work_ctx;
#endif
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
  double cell_size;

  msh_hg_v3_t min_pt;
  msh_hg_v3_t max_pt;

  msh_hg_map_t* bin_table;
  msh_hg_v3i_t* data_buffer;
  msh_hg__bin_info_t* offsets;

  int32_t   _slab_size;
  double _inv_cell_size;
  uint8_t _pts_dim;
  uint16_t _num_threads;
  int32_t _dont_use_omp;
  uint32_t max_n_pts_in_bin;
  size_t _n_pts;
} msh_hash_grid_t;

typedef struct msh_hg_map
{
  uint64_t* keys;
  uint64_t* vals;
  size_t _len;
  size_t _cap;
} msh_hg_map_t;

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



#ifdef __cplusplus
}
#endif

#endif /* MSH_HASH_GRID_H */


#ifdef MSH_HASH_GRID_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// THIS IS A SIMPLIFIED VERSION OF MSH_STD_ARRAY

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// THIS IS A COPY OF MSH_MAP

uint64_t  msh_hg_hash_uint64( uint64_t x );
void      msh_hg_map_init( msh_hg_map_t* map, uint32_t cap );
void      msh_hg_map_free( msh_hg_map_t* map );
size_t    msh_hg_map_len( msh_hg_map_t* map );
size_t    msh_hg_map_cap( msh_hg_map_t* map );
void      msh_hg_map_insert( msh_hg_map_t* map, uint64_t key, uint64_t val );
uint64_t* msh_hg_map_get( const msh_hg_map_t* map, uint64_t key );

// NOTE(maciej): This is not very comprehensive as far as platform detection macros go. 
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
  assert( dim == 2 || dim == 3 );

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
  #if defined(_OPENMP)
  if( hg->_num_threads == 1 ) { hg->_dont_use_omp = 1; }
  #endif

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
  hg->max_pt.x += 0.0001f; hg->max_pt.y += 0.0001f; hg->max_pt.z += 0.0001f;
  hg->min_pt.x -= 0.0001f; hg->min_pt.y -= 0.0001f; hg->min_pt.z -= 0.0001f;

  // Calculate dimensions
  float dim_x   = (hg->max_pt.x - hg->min_pt.x);
  float dim_y   = (hg->max_pt.y - hg->min_pt.y);
  float dim_z   = (hg->max_pt.z - hg->min_pt.z);
  float max_dim = MSH_HG_MAX3( dim_x, dim_y, dim_z );
  
  // Calculate cell size
  if( radius > 0.0 ) { hg->cell_size = 2.0 * radius; }
  else               { hg->cell_size = max_dim / (32 * sqrtf(3.0f)); }

  hg->width     = (int)(dim_x / hg->cell_size + 1.0);
  hg->height    = (int)(dim_y / hg->cell_size + 1.0) ;
  hg->depth     = (int)(dim_z / hg->cell_size + 1.0) ;
  hg->_inv_cell_size = 1.0f/ hg->cell_size;
  hg->_slab_size = hg->height * hg->width;
  hg->_n_pts = 0;

  // Create hash table
  hg->bin_table = (msh_hg_map_t*)MSH_HG_CALLOC( 1, sizeof(msh_hg_map_t) );
  msh_hg_map_init( hg->bin_table, 128 );
  msh_hg_array( msh_hg__bin_data_t ) bin_table_data = {0};
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
    hg->_n_pts += n_bin_pts;
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

void 
msh_hash_grid__sort( float* dists, int32_t* indices, int n )
{
  msh_hash_grid__quick_sort( dists, indices, n );
  msh_hash_grid__ins_sort( dists, indices, n );
}

// Heap implementation with a twist that we swap array of indices based on the distance heap
void
msh_hash_grid__heapify( float *dists, int32_t* ind, size_t len, size_t cur )
{
  size_t max = cur;
  const size_t left  = (cur<<1) + 1;
  const size_t right = (cur<<1) + 2;

  if( (left < len) && (dists[left] > dists[cur]) )   { max = left; }
  if( (right < len) && (dists[right] > dists[max]) ) { max = right; }

  if( max != cur ) // need to swap
  {
    float tmp_dist = dists[cur];
    dists[cur] = dists[max];
    dists[max] = tmp_dist;

    int32_t tmp_idx = ind[cur];
    ind[cur] = ind[max];
    ind[max] = tmp_idx;
    
    msh_hash_grid__heapify( dists, ind, len, max );
  }
}

void msh_hash_grid__heap_make( real32_t* dists, int32_t* ind, size_t len )
{
  int64_t i = len >> 1;
  while ( i >= 0 ) { msh_hash_grid__heapify( dists, ind, len, i-- ); }
}

void msh_hash_grid__heap_pop( real32_t* dists, int32_t* ind, size_t len )
{
  float max_dist = dists[0];
  dists[0] = dists[len-1];
  dists[len-1] = max_dist;

  float max_idx = ind[0];
  ind[0] = ind[len-1];
  ind[len-1] = max_idx;

  len--;

  if( len > 0 ){ msh_hash_grid__heapify( dists, ind, len, 0 ); }
}

void msh_hash_grid__heap_push( real32_t* dists, int32_t* ind, size_t len )
{
  int64_t i = len - 1;
  float d = dists[i];
  int32_t idx = ind[i];

  while( i > 0 )
  {
    int64_t j = (i - 1) >> 1;
    if( dists[j] >= d ) break;
    dists[i] = dists[j];
    ind[i] = ind[j];
    i = j;
  }
  dists[i] = d;
  ind[i] = idx;
}


typedef struct msh_hash_grid_dist_storage
{
  size_t    cap;
  size_t    len;
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
  q->max_dist     = -MSH_F32_MAX;
  q->is_heap      = 0;
  q->dists        = dists;
  q->indices      = indices;
}

uint32_t query_counter = 0;
uint32_t skip_counter = 0;
uint32_t valid_counter = 0;

MSH_HG_INLINE void
msh_hash_grid_dist_storage_push( msh_hash_grid_dist_storage_t* q,
                                 const float dist, const int32_t idx )
{
  if( q->len >= q->cap && dist >= q->max_dist ) { return; }

  if( q->len >= q->cap ) 
  {
    // remove farthest if at capacity
    msh_hash_grid__heap_pop( q->dists, q->indices, q->len );
    q->len--;
  }

  // add new element
  q->dists[ q->len ] = dist;
  q->indices[ q->len ] = idx;
  q->len++;

  if( q->is_heap) { msh_hash_grid__heap_push( q->dists, q->indices, q->len ); }

  if( q->len >= q->cap && !q->is_heap ) 
  {
    msh_hash_grid__heap_make( q->dists, q->indices, q->len );
    q->is_heap = 1;
  }

  if( q->is_heap ) { q->max_dist = q->dists[0]; }
  else if ( q->max_dist <= dist ) { q->max_dist = dist; }
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
      msh_hash_grid_dist_storage_push( s, dist_sq, dii );
    }
  }
}

uint32_t
msh_hash_grid__radius_search( const msh_hash_grid_t* hg, 
                              msh_hash_grid_search_desc_t* hg_sd, 
                              uint32_t start_idx, uint32_t end_idx  )
{
  if( !hg || !hg_sd ) { return 0; }
  enum { MAX_BIN_COUNT = 256 };
  int32_t bin_indices[ MAX_BIN_COUNT ];
  float bin_dists_sq[ MAX_BIN_COUNT ];

  float radius          = hg_sd->radius;
  double radius_sq      = (double)radius * (double)radius;
  size_t row_size       = hg_sd->max_n_neigh;

  msh_hash_grid_dist_storage_t storage;

  uint32_t total_num_neighbors = 0;
  for( uint32_t pt_idx = start_idx; pt_idx < end_idx; ++pt_idx )
  {
    float* query_pt       = hg_sd->query_pts + (hg->_pts_dim * pt_idx);
    size_t* n_neighbors   = hg_sd->n_neighbors ? (hg_sd->n_neighbors + pt_idx) : NULL;
    float* dists_sq       = hg_sd->distances_sq + (pt_idx * row_size);
    int32_t* indices      = hg_sd->indices + (pt_idx * row_size);

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
    int64_t ix = (int64_t)( q.x * hg->_inv_cell_size );
    int64_t iy = (int64_t)( q.y * hg->_inv_cell_size );
    int64_t iz = (int64_t)( q.z * hg->_inv_cell_size );

    // Decide where to look
    int64_t px  = (int64_t)( (q.x + radius) * hg->_inv_cell_size );
    int64_t nx  = (int64_t)( (q.x - radius) * hg->_inv_cell_size );
    int64_t opx = px - ix;
    int64_t onx = nx - ix;

    int64_t py  = (int64_t)( (q.y + radius) * hg->_inv_cell_size );
    int64_t ny  = (int64_t)( (q.y - radius) * hg->_inv_cell_size );
    int64_t opy = py - iy;
    int64_t ony = ny - iy;

    int64_t pz  = (int64_t)( (q.z + radius) * hg->_inv_cell_size );
    int64_t nz  = (int64_t)( (q.z - radius) * hg->_inv_cell_size );
    int64_t opz = pz - iz;
    int64_t onz = nz - iz;


    uint32_t n_visited_bins = 0;
    float dx, dy, dz;
    int64_t cx, cy, cz;
    for( int64_t oz = onz; oz <= opz; ++oz )
    {
      cz = (int64_t)iz + oz;
      if( cz < 0 || cz >= (int64_t)hg->depth ) { continue; }
      uint64_t idx_z = cz * hg->_slab_size;

      if( oz < 0 )      { dz = q.z - (cz + 1) * hg->cell_size; }
      else if( oz > 0 ) { dz = cz * hg->cell_size - q.z; }
      else              { dz = 0.0f; }

      for( int64_t oy = ony; oy <= opy; ++oy )
      {
        cy = iy + oy;
        if( cy < 0 || cy >= (int64_t)hg->height ) { continue; }
        uint64_t idx_y = cy * hg->width;

        if( oy < 0 )      { dy = q.y - (cy + 1) * hg->cell_size; }
        else if( oy > 0 ) { dy = cy * hg->cell_size - q.y; }
        else              { dy = 0.0f; }

        for( int64_t ox = onx; ox <= opx; ++ox )
        {
          cx = ix + ox;
          if( cx < 0 || cx >= (int64_t)hg->width ) { continue; }

          // assert( n_visited_bins < MAX_BIN_COUNT );
          if( n_visited_bins >= MAX_BIN_COUNT ) { goto msh_hash_grid_lbl__find_neighbors2; }

          bin_indices[n_visited_bins] = idx_z + idx_y + cx;

          if( ox < 0 )      { dx = q.x - (cx + 1) * hg->cell_size; }
          else if( ox > 0 ) { dx = cx * hg->cell_size - q.x; }
          else              { dx = 0.0f; }

          bin_dists_sq[n_visited_bins] = dz * dz + dy * dy + dx * dx;
          n_visited_bins++;
        }
      }
    }

msh_hash_grid_lbl__find_neighbors2:
    msh_hash_grid__sort( bin_dists_sq, bin_indices, n_visited_bins );

    for( uint32_t i = 0; i < n_visited_bins; ++i )
    {
      msh_hash_grid__find_neighbors_in_bin( hg, bin_indices[i], radius_sq, query_pt, &storage );
      if( storage.len >= row_size &&
          storage.max_dist <= bin_dists_sq[i] )
      {
        break;
      }
    }

    if( hg_sd->sort ) { msh_hash_grid__sort( dists_sq, indices, storage.len ); }

    if( n_neighbors ) { (*n_neighbors++) = storage.len; }

    total_num_neighbors += storage.len;
  }
  return total_num_neighbors;;
}

#ifdef MSH_JOBS
typedef struct msh_hash_grid__work_opts
{
  const msh_hash_grid_t* hg;
  msh_hash_grid_search_desc_t* hg_sd;
  uint32_t start_idx;
  uint32_t end_idx;
  uint32_t volatile *total_num_neighbors;
} msh_hash_grid__work_opts_t;

MSH_JOBS_JOB_SIGNATURE(msh_hash_grid__run_radius_search)
{
  msh_hash_grid__work_opts_t opts = *((msh_hash_grid__work_opts_t*)params);
  uint32_t cur_num_neighbors = msh_hash_grid__radius_search( opts.hg, opts.hg_sd, opts.start_idx, opts.end_idx );
  msh_jobs_atomic_add( opts.total_num_neighbors, cur_num_neighbors );
  return 0;
}
#endif

size_t 
msh_hash_grid_radius_search2( const msh_hash_grid_t* hg,
                             msh_hash_grid_search_desc_t* hg_sd )
{
  assert( hg_sd->query_pts );
  assert( hg_sd->distances_sq );
  assert( hg_sd->indices );
  assert( hg_sd->radius > 0.0 );
  assert( hg_sd->n_query_pts > 0 );
  assert( hg_sd->max_n_neigh > 0 );

#ifdef MSH_JOBS
  uint32_t single_thread_limit = 64;
  if( !hg_sd->work_ctx || hg_sd->n_query_pts < single_thread_limit )
  {
    return msh_hash_grid__radius_search( hg, hg_sd, 0, hg_sd->n_query_pts );
  }
  else
  {
#if 1
    uint32_t volatile total_num_neighbors = 0;
    enum{ MSH_HASH_GRID__N_TASKS = 128 };
    msh_hash_grid__work_opts_t work_array[MSH_HASH_GRID__N_TASKS];
    uint32_t n_pts_per_task = hg_sd->n_query_pts / MSH_HASH_GRID__N_TASKS + 1;
    n_pts_per_task = n_pts_per_task < single_thread_limit ? single_thread_limit : n_pts_per_task;
    for( uint32_t work_idx = 0; work_idx < MSH_HASH_GRID__N_TASKS; ++work_idx )
    {
      uint32_t start_idx = work_idx * n_pts_per_task;
      uint32_t end_idx = msh_min( (work_idx + 1) * n_pts_per_task, hg_sd->n_query_pts );
      if( start_idx > hg_sd->n_query_pts ) { break; }
      msh_hash_grid__work_opts_t* work_entry = work_array + work_idx;
      work_entry->hg = hg;
      work_entry->hg_sd = hg_sd;
      work_entry->total_num_neighbors = &total_num_neighbors;
      work_entry->start_idx = start_idx;
      work_entry->end_idx = end_idx;
      msh_jobs_push_work( hg_sd->work_ctx, msh_hash_grid__run_radius_search, work_entry );
    }
    msh_jobs_complete_all_work( hg_sd->work_ctx );

    return total_num_neighbors;
#else
    // NOTE(maciej): This does not work completely, as it sometimes overflows the queue
    uint32_t volatile total_num_neighbors = 0;
    uint32_t start_idx = 0;
    int64_t pts_left = hg_sd->n_query_pts;

    // NOTE(maciej): We could always just generate 16 jobs or smth like that to avoid malloc. Will need to test.
    uint32_t work_idx = 0;
    uint32_t n_tasks = pts_left / single_thread_limit + 1;
    msh_hash_grid__work_opts_t* work_array = malloc( n_tasks * sizeof(msh_hash_grid__work_opts_t) );
    while( pts_left > 0 )
    {
      uint32_t count = (pts_left >= single_thread_limit) ? single_thread_limit : pts_left;
      msh_hash_grid__work_opts_t* work_entry = work_array + work_idx++;
      work_entry->hg = hg;
      work_entry->hg_sd = hg_sd;
      work_entry->total_num_neighbors = &total_num_neighbors;
      work_entry->start_idx = start_idx;
      work_entry->end_idx = start_idx + count;
      msh_jobs_push_work( hg_sd->work_ctx, msh_hash_grid__run_radius_search, work_entry );
      
      pts_left -= single_thread_limit;
      start_idx += single_thread_limit;
    }
    msh_jobs_complete_all_work( hg_sd->work_ctx );
    free( work_array );

    return total_num_neighbors;
#endif
  }
#else
  return msh_hash_grid__radius_search( hg, hg_sd, 0, hg_sd->n_query_pts );
#endif
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
  enum { MAX_BIN_COUNT = 512, MAX_THREAD_COUNT = 512 };
  uint32_t n_query_pts = hg_sd->n_query_pts;
  size_t row_size      = hg_sd->max_n_neigh;
  double radius        = hg_sd->radius;
  uint64_t slab_size   = hg->_slab_size;
  double cs            = hg->cell_size;
  double ics           = hg->_inv_cell_size;
  int64_t w            = hg->width;
  int64_t h            = hg->height;
  int64_t d            = hg->depth;
  double radius_sq     = radius * radius;

  uint32_t n_pts_per_thread = n_query_pts;
  uint32_t total_num_neighbors = 0;
  uint32_t num_neighbors_per_thread[MAX_THREAD_COUNT] = {0};
  uint32_t num_threads = hg->_num_threads;
  assert( num_threads <= MAX_THREAD_COUNT );

#if defined(_OPENMP)
  #pragma omp parallel if (!hg->_dont_use_omp)
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

              // assert( n_visited_bins < MAX_BIN_COUNT );
              if( n_visited_bins >= MAX_BIN_COUNT ) { goto msh_hash_grid_lbl__find_neighbors; }

              bin_indices[n_visited_bins] = idx_z + idx_y + cx;

              if( ox < 0 )      { dx = q.x - (cx + 1) * cs; }
              else if( ox > 0 ) { dx = cx * cs - q.x; }
              else              { dx = 0.0f; }

              bin_dists_sq[n_visited_bins] = dz * dz + dy * dy + dx * dx;
              n_visited_bins++;
            }
          }
        }
msh_hash_grid_lbl__find_neighbors:
        msh_hash_grid__sort( bin_dists_sq, bin_indices, n_visited_bins );

        for( uint32_t i = 0; i < n_visited_bins; ++i )
        {
          msh_hash_grid__find_neighbors_in_bin( hg, bin_indices[i], radius_sq, query_pt, &storage );
          if( storage.len >= row_size &&
              storage.max_dist <= bin_dists_sq[i] )
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

    msh_hash_grid_dist_storage_push( s, dist_sq, data[i].i );
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
  #pragma omp parallel if (!hg->_dont_use_omp)
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
                    dist_sq > storage.max_dist ) { continue; }

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
  msh_hg_array_hdr_t *new_hdr = NULL;

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


/*
------------------------------------------------------------------------------

This software is available under 2 licenses - you may choose the one you like.

------------------------------------------------------------------------------

ALTERNATIVE A - MIT License

Copyright (c) 2018 Maciej Halber

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