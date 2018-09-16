/************************
 TODOs
 [x] Fix issue when _init function cannot be used if no implementation is declared.
 [ ] Optimization - in both knn and radius I need a better way to determine whether I can early out
 [ ] Optimization - see if supplying structure which interleaves distances and indices help
 [ ] Optimization - spatial locality - sort linear data on  bin idx or morton curves
 [ ] Detect if msh_std was declared. If so, don't re-add the data structures
 [ ] Multithreading ?
 [ ] API for supplying more then a single point 
 [ ] Docs
**********************/

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

#ifndef MSH_HG_CALLOC
#define MSH_HG_CALLOC(x,y) calloc((x), (y))
#endif

#ifndef MSH_HG_REALLOC
#define MSH_HG_REALLOC(x,y) realloc((x), (y))
#endif

#ifndef MSH_HG_FREE
#define MSH_HG_FREE(x) free((x))
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct msh_hg_bin_data msh_hg__bin_data_t;
typedef struct msh_hg_bin_info msh_hg__bin_info_t;
typedef struct msh_hg_map msh_hg_map_t;
typedef struct msh_hg_v3 msh_hg_v3_t;
typedef struct msh_hg_v3i msh_hg_v3i_t;
typedef struct msh_hash_grid msh_hash_grid_t;

void msh_hash_grid_init( msh_hash_grid_t* hg, const float* pts, const int n_pts, const float radius );
void msh_hash_grid_term( msh_hash_grid_t* hg );
int msh_hash_grid_radius_search( const msh_hash_grid_t *hg, const float* query_pt, const float radius, 
                                 float* dists_sq, int32_t* indices, int max_n_results, int sort );
int msh_hash_grid_knn_search( const msh_hash_grid_t* hg, const float* query_pt, const int k,
                              float* dists_sq, int* indices, int sort );

typedef struct msh_hg_v3 
{ 
  float x, y, z; 
} msh_hg_v3_t;

typedef struct msh_hg_v3i
{
  float x, y, z;
  int32_t i;
} msh_hg_v3i_t;

typedef struct msh_hash_grid
{
  size_t width;
  size_t height;
  size_t depth;
  float cell_size;

  msh_hg_v3_t min_pt;
  msh_hg_v3_t max_pt;

  msh_hg_map_t* bin_table;
  msh_hg_v3i_t* linear_data;
  msh_hg__bin_info_t* offsets;

  int32_t   _slab_size;
  float _inv_cell_size;
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

void      msh_hg_map_init( msh_hg_map_t* map );
void      msh_hg_map_free( msh_hg_map_t* map );
uint64_t  msh_hg_hash_uint64(uint64_t x);
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

typedef struct msh_hg_bin_data
{ 
  int32_t n_pts;
  msh_hg_v3i_t* data;
} msh_hg__bin_data_t;

typedef struct msh_hg_bin_info
{ 
  int32_t offset;
  int32_t length;
} msh_hg__bin_info_t;

MSH_HG_INLINE uint64_t
msh_hash_grid__bin_pt( const msh_hash_grid_t* hg, uint64_t ix, uint64_t iy, uint64_t iz )
{
  uint64_t bin_idx = iz * hg->_slab_size + iy * hg->width + ix;
  return bin_idx;
}

void
msh_hash_grid_init( msh_hash_grid_t* hg, const float* pts, const int n_pts, const float radius )
{
  // Compute bbox
  hg->min_pt = (msh_hg_v3_t){ .x =  1e9, .y =  1e9, .z =  1e9 };
  hg->max_pt = (msh_hg_v3_t){ .x = -1e9, .y = -1e9, .z = -1e9 };

  for( int i = 0; i < n_pts; ++i )
  {
    const float* pt_ptr = &pts[3*i];
    msh_hg_v3_t pt = { .x = pt_ptr[0], .y = pt_ptr[1], .z = pt_ptr[2] };
    hg->min_pt.x = (hg->min_pt.x > pt.x) ? pt.x : hg->min_pt.x;
    hg->min_pt.y = (hg->min_pt.y > pt.y) ? pt.y : hg->min_pt.y;
    hg->min_pt.z = (hg->min_pt.z > pt.z) ? pt.z : hg->min_pt.z;

    hg->max_pt.x = (hg->max_pt.x < pt.x) ? pt.x : hg->max_pt.x;
    hg->max_pt.y = (hg->max_pt.y < pt.y) ? pt.y : hg->max_pt.y;
    hg->max_pt.z = (hg->max_pt.z < pt.z) ? pt.z : hg->max_pt.z;
  }

  // Calculate dimensions
  float dim_x = (hg->max_pt.x - hg->min_pt.x);
  float dim_y = (hg->max_pt.y - hg->min_pt.y);
  float dim_z = (hg->max_pt.z - hg->min_pt.z);
  float dim = MSH_HG_MAX3( dim_x, dim_y, dim_z );
  
  // Calculate cell size
  if( radius > 0.0 ) { hg->cell_size = 2.0f * radius; }
  else               { hg->cell_size = dim / (32 * sqrtf(3.0f)); }

  hg->width     = (int)(dim_x / hg->cell_size + 1.0);
  hg->height    = (int)(dim_y / hg->cell_size + 1.0) ;
  hg->depth     = (int)(dim_z / hg->cell_size + 1.0) ;
  hg->_inv_cell_size = 1.0f / hg->cell_size;
  hg->_slab_size = hg->height * hg->width;

  // Create hash table
  hg->bin_table = (msh_hg_map_t*)MSH_HG_MALLOC( sizeof(msh_hg_map_t) );
  msh_hg_map_init( hg->bin_table );
  msh_hg_array( msh_hg__bin_data_t ) bin_table_data = 0;
  uint64_t n_bins = 0;
  for( int i = 0 ; i < n_pts; ++i )
  {
    msh_hg_v3i_t pt_data = (msh_hg_v3i_t){ .x = pts[3*i+0], 
                                           .y = pts[3*i+1], 
                                           .z = pts[3*i+2], 
                                           .i = i };
 
    uint64_t ix = (uint64_t)((pt_data.x - hg->min_pt.x ) * hg->_inv_cell_size);
    uint64_t iy = (uint64_t)((pt_data.y - hg->min_pt.y ) * hg->_inv_cell_size);
    uint64_t iz = (uint64_t)((pt_data.z - hg->min_pt.z ) * hg->_inv_cell_size);

    uint64_t bin_idx = msh_hash_grid__bin_pt( hg, ix, iy, iz );

    // NOTE(maciej): In msh_map we can't have 0 as key
    uint64_t* bin_table_idx = msh_hg_map_get( hg->bin_table, bin_idx + 1 );
    
    if( bin_table_idx ) 
    { 
      bin_table_data[*bin_table_idx].n_pts += 1;
      msh_hg_array_push( bin_table_data[*bin_table_idx].data, pt_data );
    }
    else 
    {
      msh_hg_map_insert( hg->bin_table, bin_idx + 1, n_bins );
      
      msh_hg__bin_data_t new_bin = {0};
      new_bin.n_pts = 1;
      msh_hg_array_push( new_bin.data, pt_data );
      msh_hg_array_push( bin_table_data, new_bin );
      n_bins++;
    }
  }

  // Prepare storage for linear data
  hg->offsets     = (msh_hg__bin_info_t*)MSH_HG_MALLOC( n_bins * sizeof(msh_hg__bin_info_t) );
  hg->linear_data = (msh_hg_v3i_t*)MSH_HG_MALLOC( n_pts * sizeof(msh_hg_v3i_t) );
  memset( hg->offsets, 0, n_bins * sizeof(msh_hg__bin_info_t) );

  // Reorder the data from the hash-table into linear block
  // NOTE(maciej): There should be a sort here
  // Possible way to achieve this is to generate a bin index, check if
  // such index exists ( ) and then put data into linear array.
  // -> Get all non zero values from hashmap, sort them
  // and then go over them.
  int offset = 0;
  for( size_t i = 0; i < n_bins; ++i )
  {
    msh_hg__bin_data_t* bin = &bin_table_data[i];
    int n_bin_pts = bin->n_pts;
    if(!n_bin_pts) { continue; }
  
    for( int j = 0; j < n_bin_pts; ++j )
    {
      hg->linear_data[offset+j] = bin->data[j];
    }
    hg->offsets[i] = (msh_hg__bin_info_t) { .offset = offset, .length = n_bin_pts };
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

  MSH_HG_FREE( hg->linear_data ); hg->linear_data = NULL;
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
      if (c01 != c12) {
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
      for(;;) {
         // handling of equality is crucial here for sentinels & efficiency with duplicates
         db = dists[0];
         for (;;++i) {
            da = dists[i];
            c = da < db;
            if (!c) break;
         }
         da = dists[0];
         for (;;--j) {
            db = dists[j];
            c = da < db;
            if (!c) break;
         }
         // make sure we haven't crossed
         if (i >= j) break;
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

void msh_hash_grid__sort( float* dists, int32_t* indices, int n)
{
  msh_hash_grid__quick_sort(dists, indices, n);
  msh_hash_grid__ins_sort(dists, indices, n);
}


typedef struct msh_hash_grid_dist_storage
{
  int32_t cap;
  int32_t len;
  int32_t max_dist_idx;
  float   *dists;
  int32_t *indices;
} msh_hash_grid_dist_storage_t;

MSH_HG_INLINE void 
msh_hash_grid_dist_storage_init( msh_hash_grid_dist_storage_t* q,
                                 const int k, float* dists, int32_t* indices )
{
  q->cap          = k;
  q->len          = 0;
  q->max_dist_idx = -1;
  q->dists        = dists;
  q->indices      = indices;
}

MSH_HG_INLINE void
msh_hash_grid_dist_storage_push( msh_hash_grid_dist_storage_t* q, float dist, int32_t idx )
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
      if( q->dists[q->max_dist_idx] < dist ) { q->max_dist_idx = q->len; }
    }
    else
    {
      q->max_dist_idx = q->len;
    }
    q->len++;
  }
  // We are at capacity. Only add if new is smaller than maximal dist.
  else
  {
    if( q->dists[q->max_dist_idx] > dist )
    {
      q->dists[q->max_dist_idx] = dist;
      q->indices[q->max_dist_idx] = idx;

      // Make sure we are really looking at the highest element after replacement
      for( int32_t i = 0; i < q->len; ++i )
      {
        if (q->dists[i] > q->dists[q->max_dist_idx] ) { q->max_dist_idx = i; }
      }
    }
  }
}

MSH_HG_INLINE void
msh_hash_grid__find_neighbors_in_bin( const msh_hash_grid_t* hg, const uint64_t bin_idx, 
                                      const float radius_sq, const msh_hg_v3_t* pt, 
                                      msh_hash_grid_dist_storage_t* s )
{
  // issue this whole things stops working if we use doubles.
  uint64_t* bin_table_idx = msh_hg_map_get( hg->bin_table, (bin_idx + 1));
  if( !bin_table_idx ) { return; }

  msh_hg__bin_info_t bi = hg->offsets[ *bin_table_idx ];
  int n_pts = bi.length;
  const msh_hg_v3i_t* data = &hg->linear_data[bi.offset];

  for( int32_t i = 0; i < n_pts; ++i )
  {
    // TODO(maciej): Maybe sse?
    msh_hg_v3_t v = { data[i].x - pt->x,
                      data[i].y - pt->y,
                      data[i].z - pt->z };
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    if( dist_sq < radius_sq )
    {
      msh_hash_grid_dist_storage_push( s, dist_sq, data[i].i );
    }
  }
}

int
msh_hash_grid_radius_search( const msh_hash_grid_t* hg, const float* query_pt, 
                             const float radius, float* dists_sq, int* indices, 
                             int max_n_results, int sort )
{
  msh_hash_grid_dist_storage_t storage;
  msh_hash_grid_dist_storage_init( &storage, max_n_results, dists_sq, indices );

  msh_hg_v3_t* pt = (msh_hg_v3_t*)query_pt;

  // Get base bin idx for query pt
  uint64_t ix = (uint64_t)( (pt->x - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t iy = (uint64_t)( (pt->y - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t iz = (uint64_t)( (pt->z - hg->min_pt.z) * hg->_inv_cell_size );

  // Decide where to look
  float dist_px = pt->x + radius;
  float dist_nx = pt->x - radius;
  
  float dist_py = pt->y + radius;
  float dist_ny = pt->y - radius;
  
  float dist_pz = pt->z + radius;
  float dist_nz = pt->z - radius;

  uint64_t px = (uint64_t)( (dist_px - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t nx = (uint64_t)( (dist_nx - hg->min_pt.x) * hg->_inv_cell_size );
  int64_t opx = px - ix;
  int64_t onx = nx - ix;
  uint64_t py = (uint64_t)( (dist_py - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t ny = (uint64_t)( (dist_ny - hg->min_pt.y) * hg->_inv_cell_size );
  int64_t opy = py - iy;
  int64_t ony = ny - iy;
  uint64_t pz = (uint64_t)( (dist_pz - hg->min_pt.z) * hg->_inv_cell_size );
  uint64_t nz = (uint64_t)( (dist_nz - hg->min_pt.z) * hg->_inv_cell_size );
  int64_t opz = pz - iz;
  int64_t onz = nz - iz;

  int32_t neigh_idx = 0;
  float radius_sq = radius * radius;
  int max_dist_idx = -1;
  for( int64_t oz = onz; oz <= opz; ++oz )
  {
    int64_t cz = (int64_t)iz + oz;
    if( cz < 0 || cz >= (int64_t)hg->depth ) { continue; }
    uint64_t idx_z = cz * hg->_slab_size;
    for( int64_t oy = ony; oy <= opy; ++oy )
    {
      int64_t cy = (int64_t)iy + oy;
      if( cy < 0 || cy >= (int64_t)hg->height ) { continue; }
      uint64_t idx_y = cy * hg->width;
      for( int64_t ox = onx; ox <= opx; ++ox )
      {
        int64_t cx = (int64_t)ix + ox;
        if( cx < 0 || cx >= (int64_t)hg->width ) { continue; }
        uint64_t bin_idx = idx_z + idx_y + cx;
        msh_hash_grid__find_neighbors_in_bin( hg, bin_idx, radius_sq, pt,
                                              &storage );
      }
    }
  }
  if( sort ) { msh_hash_grid__sort( dists_sq, indices, storage.len ); }

  return storage.len;
}

// maybe write alternate function if radius is less or equal 0.5 cell size;
int
msh_hash_grid_radius_search2( const msh_hash_grid_t* hg, const float* query_pt, 
                             const float radius, float* dists_sq, int* indices, 
                             int max_n_results, int sort )
{
  msh_hash_grid_dist_storage_t storage;
  msh_hash_grid_dist_storage_init( &storage, max_n_results, dists_sq, indices );

  msh_hg_v3_t* pt = (msh_hg_v3_t*)query_pt;

  // Get base bin idx for query pt
  uint64_t ix = (uint64_t)( (pt->x - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t iy = (uint64_t)( (pt->y - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t iz = (uint64_t)( (pt->z - hg->min_pt.z) * hg->_inv_cell_size );

  // Decide where to look
  float dist_px = pt->x + radius;
  float dist_nx = pt->x - radius;
  
  float dist_py = pt->y + radius;
  float dist_ny = pt->y - radius;
  
  float dist_pz = pt->z + radius;
  float dist_nz = pt->z - radius;

  uint64_t px = (uint64_t)( (dist_px - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t nx = (uint64_t)( (dist_nx - hg->min_pt.x) * hg->_inv_cell_size );
  int64_t opx = px - ix;
  int64_t onx = nx - ix;

  uint64_t py = (uint64_t)( (dist_py - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t ny = (uint64_t)( (dist_ny - hg->min_pt.y) * hg->_inv_cell_size );
  int64_t opy = py - iy;
  int64_t ony = ny - iy;

  uint64_t pz = (uint64_t)( (dist_pz - hg->min_pt.z) * hg->_inv_cell_size );
  uint64_t nz = (uint64_t)( (dist_nz - hg->min_pt.z) * hg->_inv_cell_size );
  int64_t opz = pz - iz;
  int64_t onz = nz - iz;

  int32_t neigh_idx = 0;
  float radius_sq = radius * radius;
  uint64_t cur_bin_idx = msh_hash_grid__bin_pt(hg, ix, iy, iz );
  int32_t bin_indices[1024] = {0};
  float bin_dists[1024] = {0};
  int n_visited_bins = 0;
  
  for( int64_t oz = onz; oz <= opz; ++oz )
  {
    int64_t cz = (int64_t)iz + oz;
    if( cz < 0 || cz >= (int64_t)hg->depth ) { continue; }
    uint64_t idx_z = cz * hg->_slab_size;
    for( int64_t oy = ony; oy <= opy; ++oy )
    {
      int64_t cy = (int64_t)iy + oy;
      if( cy < 0 || cy >= (int64_t)hg->height ) { continue; }
      uint64_t idx_y = cy * hg->width;
      for( int64_t ox = onx; ox <= opx; ++ox )
      {
        int64_t cx = (int64_t)ix + ox;
        if( cx < 0 || cx >= (int64_t)hg->width ) { continue; }
        // I need tp take position of point p'
        uint64_t bin_idx = idx_z + idx_y + cx;
        bin_dists[n_visited_bins] = oz * hg->cell_size * oz * hg->cell_size +
                                    oy * hg->cell_size * oy * hg->cell_size +
                                    ox * hg->cell_size * ox * hg->cell_size;
        bin_indices[n_visited_bins++] = bin_idx;
      }
    }
  }

  msh_hash_grid__ins_sort(bin_dists, bin_indices, n_visited_bins);

  for( int i = 0; i < n_visited_bins; ++i )
  {
    printf("%d | %d %f\n", cur_bin_idx, bin_indices[i], bin_dists[i] );
    msh_hash_grid__find_neighbors_in_bin( hg, bin_indices[i], radius_sq, pt, &storage );
    if( storage.len >= max_n_results &&
        storage.max_dist_idx >= 0 &&
        dists_sq[storage.max_dist_idx] <= bin_dists[i] )
    {
      printf("      | %f \n", dists_sq[storage.max_dist_idx] );
      break;
    }
  }

  if( sort ) { msh_hash_grid__sort( dists_sq, indices, storage.len ); }
  
  return storage.len;
}



MSH_HG_INLINE void
msh_hash_grid__add_bin_contents( const msh_hash_grid_t* hg, const uint64_t bin_idx,   
                                 const msh_hg_v3_t* pt, msh_hash_grid_dist_storage_t* s )
{
  uint64_t* bin_table_idx = msh_hg_map_get( hg->bin_table, (bin_idx+1));

  if( !bin_table_idx ) { return; }
  
  msh_hg__bin_info_t bi = hg->offsets[*bin_table_idx];
  int n_pts = bi.length;
  const msh_hg_v3i_t* data = &hg->linear_data[bi.offset];

  for( int32_t i = 0; i < n_pts; ++i )
  {
    msh_hg_v3_t v = { data[i].x - pt->x, data[i].y - pt->y, data[i].z - pt->z };
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    msh_hash_grid_dist_storage_push( s, dist_sq, data[i].i );
  }
}

int
msh_hash_grid_knn_search( const msh_hash_grid_t* hg, const float* query_pt, const int k,
                          float* dists_sq, int* indices, int sort )
{
  msh_hash_grid_dist_storage_t storage;
  msh_hash_grid_dist_storage_init( &storage, k, dists_sq, indices );

  msh_hg_v3_t* pt = (msh_hg_v3_t*)query_pt;

  // get base bin for query
  uint64_t ix = (uint64_t)( (pt->x - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t iy = (uint64_t)( (pt->y - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t iz = (uint64_t)( (pt->z - hg->min_pt.z) * hg->_inv_cell_size );
  
  int32_t layer = 0;
  int8_t should_break = 0;
  while( true )
  {
    int32_t inc_x = 1;
    for( int32_t oz = -layer; oz <= layer; oz++ )
    {
      int64_t cz = (int64_t)iz + oz;
      if( cz < 0 || cz >= hg->depth ) continue;
      uint64_t idx_z = cz * hg->_slab_size;
      for( int32_t oy = -layer; oy <= layer; oy++ )
      {
        int64_t cy = (int64_t)iy + oy;
        if( cy < 0 || cy >= hg->height ) continue;
        uint64_t idx_y = cy * hg->width;
        if( abs(oy) != layer && abs(oz) != layer ) { inc_x = 2 * layer; }
        else                                       { inc_x = 1; }
        for( int32_t ox = -layer; ox <= layer; ox += inc_x )
        {
          int64_t cx = (int64_t)ix + ox;
          if( cx < 0 || cx >= hg->width ) continue;
          uint64_t bin_idx = idx_z + idx_y + cx;
          msh_hash_grid__add_bin_contents( hg, bin_idx, pt, &storage );
        }
      }
    }
    layer++;
    if( should_break ) { break; }
    if( storage.len >= k ) { should_break = 1;}
  }

  if( sort ) { msh_hash_grid__sort( dists_sq, indices, storage.len ); }

  return storage.len;
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

void 
msh_hg_map_init( msh_hg_map_t* map )
{
  map->keys = NULL;
  map->vals = NULL;
  map->_len = 0;
  map->_cap = 0;
}

void msh_hg_map__grow( msh_hg_map_t *map, size_t new_cap) {
  new_cap = msh_max( new_cap, 16 );
  msh_hg_map_t new_map;
  new_map.keys = (uint64_t*)MSH_HG_CALLOC( new_cap, sizeof(uint64_t) );
  new_map.vals = (uint64_t*)MSH_HG_MALLOC( new_cap * sizeof(uint64_t) );
  new_map._len = 0;
  new_map._cap = new_cap;

  for( size_t i = 0; i < map->_cap; i++ ) 
  {
    if (map->keys[i]) 
    {
      msh_hg_map_insert( &new_map, map->keys[i], map->vals[i] );
    }
  }
  MSH_HG_FREE( (void *)map->keys );
  MSH_HG_FREE( map->vals );
  *map = new_map;
}

size_t msh_hg_map_len( msh_hg_map_t* map )
{
  return map->_len;
}

size_t msh_hg_map_cap( msh_hg_map_t* map )
{
  return map->_cap;
}

void 
msh_hg_map_insert( msh_hg_map_t* map, uint64_t key, uint64_t val )
{
  assert( key );
  if( 2 * map->_len >= map->_cap) { msh_hg_map__grow(map, 2*map->_cap); }
  assert( 2 * map->_len < map->_cap );
  size_t i = (size_t)msh_hg_hash_uint64(key);
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

uint64_t* msh_hg_map_get( const msh_hg_map_t* map, uint64_t key )
{
  if (map->_len == 0) { return NULL; }
  size_t i = (size_t)msh_hg_hash_uint64(key);
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

void msh_hg_map_free( msh_hg_map_t* map )
{
  MSH_HG_FREE( map->keys );
  MSH_HG_FREE( map->vals );
  map->_cap = 0;
  map->_len = 0;
}

#endif /* MSH_HASH_GRID_IMPLEMENTATION */

// if( nk_button_label( nk_ctx, "Colorize" ) )
          // {
          //   msh_hash_grid_t* hg = msh_hash_grid_create( (float*)input_scenes[TIME_IDX]->positions[0], 
          //                                               input_scenes[TIME_IDX]->n_pts[0], 0.3 );
          //   int bin_idx = -1;
          //   uint64_t* bin_table_idx = NULL;
          //   while( bin_table_idx == NULL )
          //   {
          //     bin_idx++;
          //     bin_table_idx = msh_hg_map_get( hg->bin_table, (bin_idx + 1));
          //   }
          //   printf("%d %d\n", bin_idx, (int)(*bin_table_idx));
          //   msh_hg__bin_info_t bi = hg->offsets[ *bin_table_idx ];
          //   int n_pts = bi.length;
          //   const msh_hg_v3i_t* data = &hg->linear_data[bi.offset];
          //   printf(" || %d %d\n", bi.length, bi.offset );
          //   for( int i = 0; i < n_pts; ++i )
          //   {
          //     input_scenes[TIME_IDX]->colors[0][data[i].i] = msh_vec3(1.0f, 0.0f, 0.0f);
          //   }
          //   bin_idx++;
          //   bin_table_idx = msh_hg_map_get( hg->bin_table, (bin_idx + 1));
          //   while( bin_table_idx == NULL )
          //   {
          //     bin_idx++;
          //     bin_table_idx = msh_hg_map_get( hg->bin_table, (bin_idx + 1));
          //   }
          //   printf("%d %d\n", bin_idx, (int)(*bin_table_idx));
          //   bi = hg->offsets[ *bin_table_idx ];
          //   n_pts = bi.length;
          //   data = &hg->linear_data[bi.offset];
          //   printf(" || %d %d\n", bi.length, bi.offset );
          //   for( int i = 0; i < n_pts; ++i )
          //   {
          //     input_scenes[TIME_IDX]->colors[0][data[i].i] = msh_vec3(0.0f, 1.0f, 0.0f);
          //   }
          //   bin_idx++;
          //   bin_table_idx = msh_hg_map_get( hg->bin_table, (bin_idx + 1));
          //   while( bin_table_idx == NULL )
          //   {
          //     bin_idx++;
          //     bin_table_idx = msh_hg_map_get( hg->bin_table, (bin_idx + 1));
          //   }
          //   printf("%d %d\n", bin_idx, (int)(*bin_table_idx));
          //   bi = hg->offsets[ *bin_table_idx ];
          //   n_pts = bi.length;
          //   data = &hg->linear_data[bi.offset];
          //   printf(" || %d %d\n", bi.length, bi.offset );
          //   for( int i = 0; i < n_pts; ++i )
          //   {
          //     input_scenes[TIME_IDX]->colors[0][data[i].i] = msh_vec3(0.0f, 0.0f, 1.0f);
          //   }

          //   mshgfx_geometry_free( &pointclouds[0][n_objects+TIME_IDX] );
          //   pointclouds[0][n_objects+TIME_IDX] = convert_rs_pointcloud_to_gpu( input_scenes[TIME_IDX], 0 );

          // }