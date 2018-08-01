#define MSH_STD_IMPLEMENTATION
#define MSH_PLY_IMPLEMENTATION
#include <cmath>
#include "flann/flann.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "msh/msh_std.h"
#include "msh/msh_ply.h"

// TODOs
// [x] Finish sorting
// [x] Test SOA vs AOS.
// [ ] Implement knn searches
//    [ ] Implement list traversal
//    [ ] Figure out how to keep a list of k-closest for knn search
// [ ] Clean up the datastructure contents
// [ ] Work on the api


typedef flann::Index<flann::L2_Simple<float>> flann_index_t;
typedef struct v3 { float x, y, z; } v3_t;
typedef struct bin_data { int n_pts; v3_t* pts; int* ind; } bin_data_t;
typedef struct bin_info{ int offset; int length; } bin_info_t;
typedef struct pt_idx{ v3_t pt; int idx; } pt_idx_t;
typedef struct dist_idx{ float dist; int idx; } dist_idx_t;

typedef struct msh_spatial_table
{
  int width;
  int height;
  int depth;
  float cell_size;

  v3_t min_pt;
  v3_t max_pt;

  v3_t* data;
  int* indices;
  bin_info_t* offsets;

  int   _slab_size;
  float _inv_cell_size;
} msh_st_t;

void msh_st_init( msh_st_t* st, const float* pts, const int n_pts, const int density );
void msh_st_term( msh_st_t* st );

int
msh_st__bin_pt( const msh_st_t* st, v3_t pt )
{
  int ix = floor( (pt.x - st->min_pt.x ) / st->cell_size );
  
  int iy = floor( (pt.y - st->min_pt.y ) / st->cell_size );
  int iz = floor( (pt.z - st->min_pt.z ) / st->cell_size );
  int bin_idx = iz * st->_slab_size + iy * st->width + ix;
  return bin_idx;
}

void
msh_st_init( msh_st_t* st, const float* pts, const int n_pts, const int density )
{
  // build a bbox
  st->min_pt = (v3_t){ .x = 1e9, .y = 1e9, .z = 1e9 };
  st->max_pt = (v3_t){ .x = -1e9, .y = -1e9, .z = -1e9 };
  for( int i = 0; i < n_pts; ++i )
  {
    const float* pt_ptr = &pts[3*i];
    v3_t pt = { .x = pt_ptr[0], .y = pt_ptr[1], .z = pt_ptr[2] };
    st->min_pt.x = (st->min_pt.x > pt.x) ? pt.x : st->min_pt.x;
    st->min_pt.y = (st->min_pt.y > pt.y) ? pt.y : st->min_pt.y;
    st->min_pt.z = (st->min_pt.z > pt.z) ? pt.z : st->min_pt.z;
    st->max_pt.x = (st->max_pt.x < pt.x) ? pt.x : st->max_pt.x;
    st->max_pt.y = (st->max_pt.y < pt.y) ? pt.y : st->max_pt.y;
    st->max_pt.z = (st->max_pt.z < pt.z) ? pt.z : st->max_pt.z;
  }

  float dim_x = (st->max_pt.x - st->min_pt.x);
  float dim_y = (st->max_pt.y - st->min_pt.y);
  float dim_z = (st->max_pt.x - st->min_pt.z);
  float dim = msh_max3( dim_x, dim_y, dim_z );

  // get width, height and depth
  st->cell_size = dim / density;
  st->width     = (int)(dim_x / st->cell_size + 1.0);
  st->height    = (int)(dim_y / st->cell_size + 1.0) ;
  st->depth     = (int)(dim_z / st->cell_size + 1.0) ;

  st->_inv_cell_size = 1.0f / st->cell_size;
  st->_slab_size = st->height * st->width;

  // create tmp binned storage
  int n_bins = st->width * st->height * st->depth;
  bin_data_t* binned_data = (bin_data_t*)malloc( n_bins * sizeof(bin_data_t) );
  for( int i = 0; i < n_bins; ++i )
  {
    binned_data[i].pts   = {0};
    binned_data[i].ind   = {0};
    binned_data[i].n_pts = 0;
  }

  for( int i = 0 ; i < n_pts; ++i )
  {
    v3_t pt = (v3_t){ .x = pts[3*i+0], .y = pts[3*i+1], .z = pts[3*i+2] };
    int bin_idx = msh_st__bin_pt( st, pt );
    binned_data[bin_idx].n_pts += 1;
    msh_array_push( binned_data[bin_idx].pts, pt );
    msh_array_push( binned_data[bin_idx].ind, i );
  }

  st->offsets    = (bin_info_t*)malloc( n_bins * sizeof(bin_info_t) );
  st->data       = (v3_t*)malloc( n_pts * sizeof(v3_t) );
  st->indices    = (int*)malloc( n_pts * sizeof(int) );
  for( int i = 0; i < n_bins; ++i ) { st->offsets[i] = (bin_info_t){.offset = -1, .length = 0}; }

  // reorder the binned data into linear array for better cache friendliness
  int offset = 0;
  int avg_n_pts = 0;
  int valid_bins = 0;
  for( int i = 0; i < n_bins; ++i )
  {
    int n_bin_pts = binned_data[i].n_pts;
    if( n_bin_pts ) { valid_bins++; avg_n_pts+=n_bin_pts; }
    for( int j = 0; j < n_bin_pts; ++j )
    {
      st->data[offset+j] = binned_data[i].pts[j];
      st->indices[offset+j] = binned_data[i].ind[j];
    }
    if( n_bin_pts )
    {
      st->offsets[i] = (bin_info_t) { .offset = offset, .length = n_bin_pts };
      offset += n_bin_pts;
    }
  }

  for( int i = 0 ; i < n_bins; ++i ) 
  { 
    msh_array_free( binned_data[i].pts ); 
    msh_array_free( binned_data[i].ind ); 
  }
  free( binned_data );
}

void
msh_st_term( msh_st_t* st )
{
  st->width          = 0;
  st->height         = 0;
  st->depth          = 0;
  st->cell_size      = 0.0f;
  st->min_pt         = { 0.0f, 0.0f, 0.0f };
  st->max_pt         = { 0.0f, 0.0f, 0.0f };
  st->_slab_size     = 0.0f;
  st->_inv_cell_size = 0.0f;

  free( st->data );    st->data = NULL;
  free( st->indices ); st->indices = NULL;
  free( st->offsets ); st->offsets = NULL;
}


// NOTE(maciej): This implementation is a special case modification of a templated
// sort by Sean T. Barret from stb.h. We simply want to allow sorting both the indices
// and distances if user requested returning sorted results.
void 
msh_st__ins_sort( float *dists, int32_t* indices, int n )
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
msh_st__quick_sort( float *dists, int32_t* indices, int n )
{
   /* threshhold for transitioning to insertion sort */
   while (n > 12) 
   {
      float da, db, dt;
      int32_t ia, ib, it;
      int c01, c12, c, m, i, j;

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
         int z;
         da = dists[0];
         db = dists[n-1];
         ia = indices[0];
         ib = indices[n-1];
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
         msh_st__quick_sort( dists, indices, j );
         dists = dists + i;
         indices = indices + i;
         n = n - i;
      } 
      else 
      {
         msh_st__quick_sort( dists + i, indices + i, n - i );
         n = j;
      }
   }
}

void msh_st__sort( float* dists, int32_t* indices, int n)
{
  msh_st__quick_sort(dists, indices, n);
  msh_st__ins_sort(dists, indices, n);
}



inline void
msh_st__find_neighbors_in_bin( const msh_st_t* st, const uint32_t bin_idx, const double radius_sq,
                               const v3_t pt, float* dists_sq, int* indices, 
                               int* n_neigh, const int max_n_neigh )
{
  bin_info_t bi = st->offsets[bin_idx];
  if( !bi.length ) { return; }
  if( (*n_neigh) >= max_n_neigh ) { return; }
  const v3_t* data = &st->data[bi.offset];
  const int* ind = &st->indices[bi.offset];

  for( int32_t i = 0; i < bi.length; ++i )
  {
    v3_t v = { data[i].x - pt.x, data[i].y - pt.y, data[i].z - pt.z };
    double dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    if( dist_sq < radius_sq )
    {
      dists_sq[(*n_neigh)] = dist_sq;
      indices[(*n_neigh)]  = ind[i];

      ++(*n_neigh);
      if( (*n_neigh) >= max_n_neigh ) { return; }
    }
  }
}

int
msh_st_radius_search( const msh_st_t* st, const float* query_pt, const float radius,
                      float* dists_sq, int* indices, int max_n_results, int sort )
{

  v3_t pt = (v3_t){ .x = query_pt[0], .y = query_pt[1], .z = query_pt[2] };

  // get base bin idx for query pt
  int ix = (int)( (pt.x - st->min_pt.x) * st->_inv_cell_size );
  int iy = (int)( (pt.y - st->min_pt.y) * st->_inv_cell_size );
  int iz = (int)( (pt.z - st->min_pt.z) * st->_inv_cell_size );

  int px = (int)( ((pt.x + radius) - st->min_pt.x) * st->_inv_cell_size );
  int nx = (int)( ((pt.x - radius) - st->min_pt.x) * st->_inv_cell_size );
  int8_t opx = px - ix;
  int8_t onx = nx - ix;
  int py = (int)( ((pt.y + radius) - st->min_pt.y) * st->_inv_cell_size );
  int ny = (int)( ((pt.y - radius) - st->min_pt.y) * st->_inv_cell_size );
  int8_t opy = py - iy;
  int8_t ony = ny - iy;
  int pz = (int)( ((pt.z + radius) - st->min_pt.z) * st->_inv_cell_size );
  int nz = (int)( ((pt.z - radius) - st->min_pt.z) * st->_inv_cell_size );
  int8_t opz = pz - iz;
  int8_t onz = nz - iz;

  int neigh_idx = 0;
  float radius_sq = radius * radius;

  for( int8_t oz = onz; oz <= opz; ++oz )
  {
    uint32_t idx_z = (iz + oz) * st->_slab_size;
    if( iz + oz < 0) continue;
    if( iz + oz >= st->depth ) continue;
    for( int8_t oy = ony; oy <= opy; ++oy )
    {
      uint32_t idx_y = (iy + oy) * st->width;
      if( iy + oy < 0) continue;
      if( iy + oy >= st->height ) continue;
      for( int8_t ox = onx; ox <= opx; ++ox )
      {
        if( ix + ox < 0) continue;
        if( ix + ox >= st->width ) continue;
        uint32_t bin_idx = idx_z + idx_y + (ix + ox);
        msh_st__find_neighbors_in_bin( st, bin_idx, radius_sq, pt,
                                       dists_sq, indices, &neigh_idx,
                                       max_n_results );
        if( neigh_idx >= max_n_results ) { break; }
      }
      if( neigh_idx >= max_n_results ) { break; }
    }
    if( neigh_idx >= max_n_results ) { break; }
  }

  int32_t n_neigh = neigh_idx;

  if( sort ) { msh_st__sort( dists_sq, indices, n_neigh ); }

  return n_neigh;
}

typedef struct msh_st_dist_storage
{
  int32_t cap;
  int32_t len;
  int32_t max_dist_idx;
  float   *dists;
  int32_t *indices;
} msh_st_dist_storage_t;

void msh_st_dist_storage_init( msh_st_dist_storage_t* q, int k, float* dists, int32_t* indices )
{
  q->cap          = k;
  q->len          = 0;
  q->max_dist_idx = -1;
  q->dists        = dists;
  q->indices      = indices;
}

void msh_st_dist_storage_push( msh_st_dist_storage_t* q, float dist, int32_t idx )
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


inline void
msh_st__add_bin_contents( const msh_st_t* st, const uint32_t bin_idx, const v3_t pt, msh_st_dist_storage_t* s )
{
  bin_info_t bi = st->offsets[bin_idx];
  if( !bi.length ) { return; }
  const v3_t* data = &st->data[bi.offset];
  const int* ind = &st->indices[bi.offset];

  for( int32_t i = 0; i < bi.length; ++i )
  {
    v3_t v = { data[i].x - pt.x, data[i].y - pt.y, data[i].z - pt.z };
    double dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    msh_st_dist_storage_push( s, dist_sq, ind[i] );
  }
}

int
msh_st_knn_search( const msh_st_t* st, const float* query_pt, const int k,
                   float* dists_sq, int* indices, int sort )
{
  msh_st_dist_storage_t storage;
  msh_st_dist_storage_init( &storage, k, dists_sq, indices );

  v3_t pt = (v3_t){ .x = query_pt[0], .y = query_pt[1], .z = query_pt[2] };

  // get base bin for query
  int32_t ix = (int)( (pt.x - st->min_pt.x) * st->_inv_cell_size );
  int32_t iy = (int)( (pt.y - st->min_pt.y) * st->_inv_cell_size );
  int32_t iz = (int)( (pt.z - st->min_pt.z) * st->_inv_cell_size );
  
  int32_t layer = 0;
  int8_t should_break = 0;
  while( true )
  {
    int32_t inc_x = 1;
    for( int32_t oz = -layer; oz <= layer; oz++ )
    {
      uint32_t idx_z = (iz + oz) * st->_slab_size;
      if( iz + oz < 0) continue;
      if( iz + oz >= st->depth ) continue;
      for( int32_t oy = -layer; oy <= layer; oy++ )
      {
        uint32_t idx_y = (iy + oy) * st->width;
        if( iy + oy < 0) continue;
        if( iy + oy >= st->height ) continue;
        if( abs(oy) != layer && abs(oz) != layer ) { inc_x = 2*layer; }
        else                                       { inc_x = 1; }
        for( int32_t ox = -layer; ox <= layer; ox += inc_x )
        {
          if( ix + ox < 0) continue;
          if( ix + ox >= st->width ) continue;
          uint32_t bin_idx = idx_z + idx_y + (ix + ox);
          msh_st__add_bin_contents( st, bin_idx, pt, &storage );
        }
      }
    }
    layer++;
    if( should_break ) { break; }
    if( storage.len >= k ) { should_break = 1;}
  }

  if( sort ) { msh_st__sort( dists_sq, indices, storage.len ); }

  return storage.len;
}





typedef struct pointcloud
{
  int n_pts;
  v3_t* pts;
} PointCloud;

void read_ply( const char* filename, PointCloud* pc )
{
  msh_ply_t* pf = msh_ply_open( filename, "rb");
  if( pf )
  {
    const char* positions_names[] = { "x", "y", "z" };
    msh_ply_desc_t vertex_desc = { .element_name = (char*)"vertex",
                                   .property_names = positions_names,
                                   .num_properties = 3,
                                   .data_type = MSH_PLY_FLOAT,
                                   .list_type = MSH_PLY_INVALID,
                                   .data = &pc->pts,
                                   .list_data = 0,
                                   .data_count = &pc->n_pts,
                                   .list_size_hint = 0 };
    msh_ply_add_descriptor( pf, &vertex_desc );
    int test = msh_ply_read(pf);
  }
  msh_ply_close(pf);
}

int main( int argc, char** argv )
{
  printf("This program is an example of a spatial hashtable use\n");
  uint64_t t2, t1;
  t1 = msh_time_now();
  PointCloud pc = {};
  if( argc <= 1 ) { printf("Please priovide path to a .ply file to read\n"); }
  read_ply( argv[argc-1], &pc );
  t2 = msh_time_now();
  printf("Reading ply file with %d pts took %fms.\n", pc.n_pts, msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  msh_st_t spatial_table;
  t1 = msh_time_now();
  msh_st_init( &spatial_table, (float*)&pc.pts[0], pc.n_pts, 64 );
  t2 = msh_time_now();
  printf("Table creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  t1 = msh_time_now();
  flann::Matrix<float> pts_mat((float*)&pc.pts[0], pc.n_pts, 3);
  flann_index_t* kdtree = new flann_index_t( pts_mat, flann::KDTreeSingleIndexParams(16));
  kdtree->buildIndex();
  t2 = msh_time_now();
  printf("KD-Tree creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  int n_query_pts = 10;
  float* query_pts = (float*)malloc( n_query_pts * sizeof(float) * 3 );
  for( int i = 0; i < n_query_pts; ++i )
  {
    int idx = rand() % pc.n_pts;
    query_pts[3*i+0] = pc.pts[idx].x;
    query_pts[3*i+1] = pc.pts[idx].y;
    query_pts[3*i+2] = pc.pts[idx].z;
  }

  float radius = 0.05f;
  float radius_sq = radius*radius;
  t1 = msh_time_now();
  int total_neigh_count_a = 0;
  int total_neigh_count_b = 0;
  int* neigh_count_a = (int*)malloc(n_query_pts * sizeof(int));
  int* neigh_count_b = (int*)malloc(n_query_pts * sizeof(int));
  #define MAX_N_NEIGH 2000
  int nn_inds[MAX_N_NEIGH] = {-1}; float nn_dists[MAX_N_NEIGH] = {1e9};
  int flann_inds[MAX_N_NEIGH] = {-1}; float flann_dists[MAX_N_NEIGH] = {1e9};\

  flann::SearchParams params = flann::SearchParams(128, 0, 1);
  params.max_neighbors = MAX_N_NEIGH;

  double timers[2] = {0.0, 0.0};

  for( int i = 0 ; i < n_query_pts; ++i )
  {
    uint64_t ct1, ct2;
    ct1 = msh_time_now();
    float* pt = &query_pts[3*i];
    neigh_count_a[i] = msh_st_radius_search( &spatial_table, pt, radius, nn_dists, nn_inds, MAX_N_NEIGH, 1 );
    total_neigh_count_a += neigh_count_a[i];
    ct2 = msh_time_now();
    timers[0] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );

    ct1 = msh_time_now();
    float *query = pt;
    flann::Matrix<float> query_mat(&query[0], 1, 3);
    flann::Matrix<int> indices_mat(&flann_inds[0], 1, MAX_N_NEIGH);
    flann::Matrix<float> dists_mat(&flann_dists[0], 1, MAX_N_NEIGH);
    neigh_count_b[i] = kdtree->radiusSearch( query_mat, indices_mat,
                                             dists_mat, radius_sq,
                                             params );
    total_neigh_count_b += neigh_count_b[i];
    ct2 = msh_time_now();
    timers[1] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );
  }
  t2 = msh_time_now();
  printf("RADIUS: Finding %d points using hash_grid.: %fms.\n", total_neigh_count_a, timers[0]);
  printf("RADIUS: Finding %d points using flann.: %fms.\n", total_neigh_count_b, timers[1]);

  total_neigh_count_a = 0;
  total_neigh_count_b = 0;
  int knn = 32;
  timers[0] = timers[1] = 0;
  for( int i = 0 ; i < n_query_pts; ++i )
  {
    uint64_t ct1, ct2;
    ct1 = msh_time_now();
    float* pt = &query_pts[3*i];
    neigh_count_a[i] = msh_st_knn_search( &spatial_table, pt, knn, nn_dists, nn_inds, 1 );
    total_neigh_count_a += neigh_count_a[i];
    ct2 = msh_time_now();
    timers[0] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );

    float *query = pt;
    flann::Matrix<float> query_mat(&query[0], 1, 3);
    flann::Matrix<int> indices_mat(&flann_inds[0], 1, MAX_N_NEIGH);
    flann::Matrix<float> dists_mat(&flann_dists[0], 1, MAX_N_NEIGH);
    ct1 = msh_time_now();
    neigh_count_b[i] = kdtree->knnSearch( query_mat, indices_mat,
                                          dists_mat, knn,
                                          params );
    total_neigh_count_b += neigh_count_b[i];
    ct2 = msh_time_now();
    timers[1] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );
    // if( neigh_count_b[i] != neigh_count_a[i] )
    // {
    //   printf("OOPS!\n");
    // }
    // else
    // {
    //   for( int k = 0; k < neigh_count_b[i]; ++k )
    //   {
    //     printf("(%d) %6d %6d | %f %f\n", (flann_inds[k]==nn_inds[k]),flann_inds[k], nn_inds[k], flann_dists[k], nn_dists[k] );
    //   }
    //   printf("----------\n");
    // }
  }
  printf("KNN: Finding %d points using hash_grid.: %fms.\n", total_neigh_count_a, timers[0]);
  printf("KNN: Finding %d points using flann.: %fms.\n", total_neigh_count_b, timers[1]);

  msh_st_term( &spatial_table );
  return 0;
}
