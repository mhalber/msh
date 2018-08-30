/************************
 TODOs
 [x] Finish sorting
 [x] Test SOA vs AOS.
 [x] Implement knn searches
 [x] Clean up the datastructure contents
 [x] Add inline resizeable array class.
 [x] Figure out if and why creation is slower if I were to use dense approach

 [x] Clean-up the code
 [x]   Check differet format for data & indices [line them up]
 [x]   Check out hash function by Teshner et al.
 [x]   Check how this would work with the MG hashtable.
 [x]   Remove binned_data, make bin_table_data temporary
 
 [ ] Detect if msh_std was declared. If so, don't re-add the data structures
 [ ] Look at Morton Codes
 [ ]  In general how to make sure that the bins our data falls into are contiguous in memory..
      I think this might be related to Perfect Spatial Hashing stuff. Overall it might be the case that
      After creating the hash-table we will reorder linear data, and make sure offsets array is correctly
      addressable.
 [ ] Multithreading?
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

#ifndef MSH_HG_REALLOC
#define MSH_HG_REALLOC(x,y) realloc((x), (y))
#endif

#ifndef MSH_HG_FREE
#define MSH_HG_FREE(x) free((x))
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct msh_hash_grid msh_hash_grid_t;

void msh_hash_grid_init( msh_hash_grid_t* hg, const float* pts, const int n_pts, const float radius );

void msh_hash_grid_term( msh_hash_grid_t* hg );

int msh_hash_grid_radius_search( const msh_hash_grid_t *hg, const float* query_pt, const float radius, 
                                 float* dists_sq, int32_t* indices, int max_n_results, int sort );

int msh_hash_grid_knn_search( const msh_hash_grid_t* hg, const float* query_pt, const int k,
                              float* dists_sq, int* indices, int sort );

#ifdef __cplusplus
}
#endif

#endif /* MSH_HASH_GRID_H */

////////////////////////////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MSH_HASH_GRID_IMPLEMENTATION

#if defined(_MSC_VER)
#define MSH_HG_INLINE __forceinline
#else
#define MSH_HG_INLINE __attribute__((always_inline, unused)) inline
#endif

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

uint64_t  msh_hg_hash_uint64(uint64_t x);

size_t    msh_hg_map_len( msh_hg_map_t* map );
size_t    msh_hg_map_cap( msh_hg_map_t* map ); 
void      msh_hg_map_insert( msh_hg_map_t* map, uint64_t key, uint64_t val );
uint64_t* msh_hg_map_get( const msh_hg_map_t* map, uint64_t key );
void      msh_hg_map_free( msh_hg_map_t* map );

typedef struct msh_hg_v3 
{ 
  float x, y, z; 
} msh_hg_v3_t;

typedef struct msh_hg_v3i
{
  float x, y, z;
  int32_t i;
} msh_hg_v3i_t;

typedef struct msh_hg_bin_data
{ 
  // int32_t idx;
  int32_t n_pts;
  msh_hg_array(msh_hg_v3i_t) data; 
  
} msh_hg__bin_data_t;

typedef struct msh_hg_bin_info
{ 
  int32_t offset;
  int32_t length;
} msh_hg__bin_info_t;

typedef struct msh_hash_grid
{
  size_t width;
  size_t height;
  size_t depth;
  float cell_size;

  msh_hg_v3_t min_pt;
  msh_hg_v3_t max_pt;

  msh_hg_map_t bin_table;

  msh_hg_v3i_t* linear_data;
  msh_hg__bin_info_t* offsets;

  int32_t   _slab_size;
  float _inv_cell_size;
} msh_hash_grid_t;


MSH_HG_INLINE uint64_t
msh_hash_grid__bin_pt( const msh_hash_grid_t* hg, uint64_t ix, uint64_t iy, uint64_t iz )
{
  uint64_t bin_idx = iz * hg->_slab_size + iy * hg->width + ix;
  return bin_idx;
}

void
msh_hash_grid_init( msh_hash_grid_t* hg, const float* pts, const int n_pts, const float radius )
{
  // uint64_t t1, t2;

  // t1 = msh_time_now();
  // build a bbox
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
  // t2 = msh_time_now();
  // printf("Time A: %f\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  // t1 = msh_time_now();
  // calculate dimensions
  float dim_x = (hg->max_pt.x - hg->min_pt.x);
  float dim_y = (hg->max_pt.y - hg->min_pt.y);
  float dim_z = (hg->max_pt.z - hg->min_pt.z);
  float dim = MSH_HG_MAX3( dim_x, dim_y, dim_z );
  
  // get width, height and depth
  if( radius > 0.0 ) { hg->cell_size = (2.0 * radius) / sqrt(3.0); }
  else               { hg->cell_size = dim / (32 * sqrt(3.0)); }

  hg->width     = (int)(dim_x / hg->cell_size + 1.0);
  hg->height    = (int)(dim_y / hg->cell_size + 1.0) ;
  hg->depth     = (int)(dim_z / hg->cell_size + 1.0) ;
  hg->_inv_cell_size = 1.0f / hg->cell_size;
  hg->_slab_size = hg->height * hg->width;
  // t2 = msh_time_now();
  // printf("Time B: %f | %d %d\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1), hg->width * hg->height * hg->depth, INT_MAX);


  // create hash table
  // t1 = msh_time_now();
  hg->bin_table = (msh_hg_map_t){0};
  msh_hg_array( msh_hg__bin_data_t ) bin_table_data = 0;
  uint64_t n_bins = 0;
  // t2 = msh_time_now();
  // printf("Time C: %f\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  // t1 = msh_time_now();
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
    
    uint64_t* bin_table_idx = msh_hg_map_get( &hg->bin_table, bin_idx+1 );
    if( bin_table_idx ) 
    { 
      bin_table_data[*bin_table_idx].n_pts += 1;
      msh_hg_array_push( bin_table_data[*bin_table_idx].data, pt_data );
    }
    else 
    {
      msh_hg_map_insert( &hg->bin_table, bin_idx+1, n_bins );
      
      msh_hg__bin_data_t new_bin = {0};
      new_bin.n_pts = 1;
      // new_bin.idx = bin_idx;
      msh_hg_array_push( new_bin.data, pt_data );

      msh_hg_array_push( bin_table_data, new_bin );
      n_bins++;
    }
  }
  // t2 = msh_time_now();
  // printf("Time D: %f\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  // t1 = msh_time_now();
  hg->offsets     = (msh_hg__bin_info_t*)malloc( n_bins * sizeof(msh_hg__bin_info_t) );
  hg->linear_data = (msh_hg_v3i_t*)malloc( n_pts * sizeof(msh_hg_v3i_t) );
  memset( hg->offsets, 0, n_bins * sizeof(msh_hg__bin_info_t) );
  // t2 = msh_time_now();
  // printf("Time E: %f\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));
  // printf("A: %d %d %d\n", n_bins, msh_hg_map_len(&hg->bin_table), hg_size );

  // int all_pts = 0;
  // for( int i = 0; i < n_bins; ++i )
  // {
  //   msh_hg__bin_data_t* bin = &bin_table_data[i];
  //   all_pts += bin->n_pts;
  // }
  // printf("B: %d %d\n", all_pts, n_pts );
  // assert(all_pts == n_pts );

  // t1 = msh_time_now();
  // reorder the data from the hash-table into linear block
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
  // t2 = msh_time_now();
  // printf("Time F: %f\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  // t1 = msh_time_now();
  for( size_t i = 0; i < n_bins; ++i )
  {
    msh_hg_array_free( bin_table_data[i].data );
  }
  msh_hg_array_free( bin_table_data );
  // t2 = msh_time_now();
  // printf("Time F: %f\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));
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

  free( hg->linear_data ); hg->linear_data = NULL;
  free( hg->offsets );     hg->offsets = NULL;
}


// NOTE(maciej): This implementation is a special case modification of a templated
// sort by Sean T. Barret from stb.h. We simply want to allow sorting both the indices
// and distances if user requested returning sorted results.
void 
msh_hash_grid__ins_sort( float *dists, int32_t* indices, int n )
{
   int i = 0;
   int j = 0;
   for( i=1; i < n; ++i )
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
   /* threshhold for transitioning to insertion sort */
   while (n > 12) 
   {
      float da, db, dt;
      int32_t it = 0;
      int32_t c01, c12, c, m, i, j;

      /* compute median of three */
      m = n >> 1;
      da = dists[0];
      db = dists[m];
      c = da < db;
      c01 = c;
      da = dists[m];
      db = dists[n-1];
      c = da < db;
      c12 = c;
      /* if 0 >= mid >= end, or 0 < mid < end, then use mid */
      if (c01 != c12) {
         /* otherwise, we'll need to swap something else to middle */
         int32_t z;
         da = dists[0];
         db = dists[n-1];
         c = da < db;
         /* 0>mid && mid<n:  0>n => n; 0<n => 0 */
         /* 0<mid && mid>n:  0>n => 0; 0<n => n */
         z = (c == c12) ? 0 : n-1;
         dt = dists[z];
         dists[z] = dists[m];
         dists[m] = dt;
         it = indices[z];
         indices[z] = indices[m];
         indices[m] = it;
      }
      /* now dists[m] is the median-of-three */
      /* swap it to the beginning so it won't move around */
      dt = dists[0];
      dists[0] = dists[m];
      dists[m] = dt;
      it = indices[0];
      indices[0] = indices[m];
      indices[m] = it; 

      /* partition loop */
      i=1;
      j=n-1;
      for(;;) {
         /* handling of equality is crucial here */
         /* for sentinels & efficiency with duplicates */
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
         /* make sure we haven't crossed */
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
      /* recurse on smaller side, iterate on larger */
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

// static double timers[4] = {0.0f, 0.0f, 0.0f, 0.0f};

MSH_HG_INLINE void
msh_hash_grid__find_neighbors_in_bin( const msh_hash_grid_t* hg, const uint64_t bin_idx, const double radius_sq,
                                      const msh_hg_v3_t pt, float* dists_sq, int* indices, 
                                      int* n_neigh, const int max_n_neigh )
{
  if( (*n_neigh) >= max_n_neigh ) { return; }
  
  // uint64_t t1, t2;
  // t1 = msh_time_now();
  uint64_t* bin_table_idx = msh_hg_map_get( &hg->bin_table, (bin_idx+1));
  // t2 = msh_time_now();
  // timers[0] += msh_time_diff(MSHT_MILLISECONDS, t2, t1);

  if( !bin_table_idx ) { return; }
  
  // t1 = msh_time_now();
  msh_hg__bin_info_t bi = hg->offsets[*bin_table_idx];
  int n_pts = bi.length;
  const msh_hg_v3i_t* data = &hg->linear_data[bi.offset];
  // t2 = msh_time_now();
  // timers[1] += msh_time_diff(MSHT_MILLISECONDS, t2, t1);
  
  // t1 = msh_time_now();
  for( int32_t i = 0; i < n_pts; ++i )
  {
    msh_hg_v3_t v = { data[i].x - pt.x, data[i].y - pt.y, data[i].z - pt.z };
    double dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    if( dist_sq < radius_sq )
    {
      dists_sq[(*n_neigh)] = dist_sq;
      indices[(*n_neigh)]  = data[i].i;

      ++(*n_neigh);
      if( (*n_neigh) >= max_n_neigh ) { return; }
    }
  }
  // t2 = msh_time_now();
  // timers[2] += msh_time_diff(MSHT_MILLISECONDS, t2, t1);
}

// static float offsets[6] = {0, 0, 0, 0, 0, 0};
// static float offset_count = 0;

int
msh_hash_grid_radius_search( const msh_hash_grid_t* hg, const float* query_pt, const float radius,
                             float* dists_sq, int* indices, int max_n_results, int sort )
{
  msh_hg_v3_t* pt = (msh_hg_v3_t*)query_pt;

  // get base bin idx for query pt
  uint64_t ix = (uint64_t)( (pt->x - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t iy = (uint64_t)( (pt->y - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t iz = (uint64_t)( (pt->z - hg->min_pt.z) * hg->_inv_cell_size );

  uint64_t px = (uint64_t)( ((pt->x + radius) - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t nx = (uint64_t)( ((pt->x - radius) - hg->min_pt.x) * hg->_inv_cell_size );
  int64_t opx = px - ix;
  int64_t onx = nx - ix;
  uint64_t py = (uint64_t)( ((pt->y + radius) - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t ny = (uint64_t)( ((pt->y - radius) - hg->min_pt.y) * hg->_inv_cell_size );
  int64_t opy = py - iy;
  int64_t ony = ny - iy;
  uint64_t pz = (uint64_t)( ((pt->z + radius) - hg->min_pt.z) * hg->_inv_cell_size );
  uint64_t nz = (uint64_t)( ((pt->z - radius) - hg->min_pt.z) * hg->_inv_cell_size );
  int64_t opz = pz - iz;
  int64_t onz = nz - iz;

  // offsets[0] += opx;
  // offsets[1] += onx;
  // offsets[2] += opy;
  // offsets[3] += ony;
  // offsets[4] += opz;
  // offsets[5] += onz;
  // offset_count++;

  int32_t neigh_idx = 0;
  float radius_sq = radius * radius;

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
        msh_hash_grid__find_neighbors_in_bin( hg, bin_idx, radius_sq, *pt,
                                              dists_sq, indices, &neigh_idx,
                                              max_n_results );
        if( neigh_idx >= max_n_results ) { break; }
      }
      if( neigh_idx >= max_n_results ) { break; }
    }
    if( neigh_idx >= max_n_results ) { break; }
  }
  int32_t n_neigh = neigh_idx;

  if( sort ) { msh_hash_grid__sort( dists_sq, indices, n_neigh ); }

  return n_neigh;
}

typedef struct msh_hash_grid_dist_storage
{
  int32_t cap;
  int32_t len;
  int32_t max_dist_idx;
  float   *dists;
  int32_t *indices;
} msh_hash_grid_dist_storage_t;

void 
msh_hash_grid_dist_storage_init( msh_hash_grid_dist_storage_t* q, int k, float* dists, int32_t* indices )
{
  q->cap          = k;
  q->len          = 0;
  q->max_dist_idx = -1;
  q->dists        = dists;
  q->indices      = indices;
}

void
msh_hash_grid_dist_storage_push( msh_hash_grid_dist_storage_t* q, float dist, int32_t idx )
{
  // We have storage left
  if( q->len < q->cap )
  {
    // Push new element
    q->dists[q->len]   = dist;
    q->indices[q->len] = idx;
    // bookkeep the index of max dist
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
  // we are at capacity. Only add if new is smaller than maximal dist.
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
msh_hash_grid__add_bin_contents( const msh_hash_grid_t* hg, const uint64_t bin_idx, const msh_hg_v3_t pt, msh_hash_grid_dist_storage_t* s )
{
  uint64_t* bin_table_idx = msh_hg_map_get( &hg->bin_table, (bin_idx+1));

  if( !bin_table_idx ) { return; }
  
  msh_hg__bin_info_t bi = hg->offsets[*bin_table_idx];
  int n_pts = bi.length;
  const msh_hg_v3i_t* data = &hg->linear_data[bi.offset];

  for( int32_t i = 0; i < n_pts; ++i )
  {
    msh_hg_v3_t v = { data[i].x - pt.x, data[i].y - pt.y, data[i].z - pt.z };
    double dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    msh_hash_grid_dist_storage_push( s, dist_sq, data[i].i );
  }
}

int
msh_hash_grid_knn_search( const msh_hash_grid_t* hg, const float* query_pt, const int k,
                   float* dists_sq, int* indices, int sort )
{
  msh_hash_grid_dist_storage_t storage;
  msh_hash_grid_dist_storage_init( &storage, k, dists_sq, indices );

  msh_hg_v3_t pt = (msh_hg_v3_t){ .x = query_pt[0], .y = query_pt[1], .z = query_pt[2] };

  // get base bin for query
  uint64_t ix = (uint64_t)( (pt.x - hg->min_pt.x) * hg->_inv_cell_size );
  uint64_t iy = (uint64_t)( (pt.y - hg->min_pt.y) * hg->_inv_cell_size );
  uint64_t iz = (uint64_t)( (pt.z - hg->min_pt.z) * hg->_inv_cell_size );
  
  int32_t layer = 0;
  int8_t should_break = 0;
  while( true )
  {
    int32_t inc_x = 1;
    for( int32_t oz = -layer; oz <= layer; oz++ )
    {
      if( (int64_t)iz + oz < 0) continue;
      if( iz + oz >= hg->depth ) continue;
      uint64_t idx_z = (iz + oz) * hg->_slab_size;
      for( int32_t oy = -layer; oy <= layer; oy++ )
      {
        if( (int64_t)iy + oy < 0) continue;
        if( iy + oy >= hg->height ) continue;
        uint64_t idx_y = (iy + oy) * hg->width;
        if( abs(oy) != layer && abs(oz) != layer ) { inc_x = 2*layer; }
        else                                       { inc_x = 1; }
        for( int32_t ox = -layer; ox <= layer; ox += inc_x )
        {
          if( (int64_t)ix + ox < 0) continue;
          if( ix + ox >= hg->width ) continue;
          uint64_t bin_idx = idx_z + idx_y + (ix + ox);
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

void msh_hg_map__grow( msh_hg_map_t *map, size_t new_cap) {
  new_cap = msh_max( new_cap, 16 );
  msh_hg_map_t new_map;
  new_map.keys = (uint64_t*)calloc( new_cap, sizeof(uint64_t) );
  new_map.vals = (uint64_t*)malloc( new_cap * sizeof(uint64_t) );
  new_map._len = 0;
  new_map._cap = new_cap;

  for( size_t i = 0; i < map->_cap; i++ ) 
  {
    if (map->keys[i]) 
    {
      msh_hg_map_insert( &new_map, map->keys[i], map->vals[i] );
    }
  }
  free( (void *)map->keys );
  free( map->vals );
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
  free( map->keys );
  free( map->vals );
  map->_cap = 0;
  map->_len = 0;
}


#endif /* MSH_HASH_GRID_IMPLEMENTATION */