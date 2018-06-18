#define MSH_IMPLEMENTATION
#include <cmath>
#include "flann/flann.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "msh.h"


typedef flann::Index<flann::L2_Simple<float>> flann_index_t;
typedef struct v3 { float x, y, z; } v3_t;
typedef struct bin_data { int n_pts; v3_t* pts; int* ind; } bin_data_t;
typedef struct bin_info{ int offset; int length; } bin_info_t;
typedef struct msh_spatial_table
{
  int width;
  int height;
  int depth;
  float cell_size;
  v3_t min_pt;
  v3_t max_pt;  
  
  v3_t *data;
  int *indices;
  bin_info_t* offsets;
  
  bin_data_t* binned_data; // this might end up being temporary
} msh_st_t;

void msh_st_init( msh_st_t* st, const float* pts, const int n_pts, const float cell_size );
void msh_st_term( msh_st_t* st );


int
msh_st__bin_pt( const msh_st_t* st, v3_t pt )
{
  int ix = floor( (pt.x - st->min_pt.x ) / st->cell_size );
  int iy = floor( (pt.y - st->min_pt.y ) / st->cell_size );
  int iz = floor( (pt.z - st->min_pt.z ) / st->cell_size );
  int bin_idx = iz * st->height*st->width + iy * st->width + ix;
  return bin_idx;
}

void
msh_st_init( msh_st_t* st, const float* pts, const int n_pts, const float cell_size )
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
  // add padding
  float eps = 0.001f;
  st->min_pt.x -= (cell_size+eps);
  st->min_pt.y -= (cell_size+eps);
  st->min_pt.z -= (cell_size+eps);
  st->max_pt.x += (cell_size+eps);
  st->max_pt.y += (cell_size+eps);
  st->max_pt.z += (cell_size+eps);

  // printf("  %f %f %f\n", st->min_pt.x, st->min_pt.y, st->min_pt.z );
  // printf("  %f %f %f\n", st->max_pt.x, st->max_pt.y, st->max_pt.z );

  // get with height and size
  st->cell_size = cell_size;
  st->width     = (int)ceilf((float)(st->max_pt.x - st->min_pt.x) / st->cell_size);
  st->height    = (int)ceilf((float)(st->max_pt.y - st->min_pt.y) / st->cell_size);
  st->depth     = (int)ceilf((float)(st->max_pt.z - st->min_pt.z) / st->cell_size);

  // create tmp binned storage
  // TODO(maciej): Use ion array here.
  int n_bins = st->width * st->height * st->depth;
  st->binned_data = (bin_data_t*)malloc( n_bins * sizeof(bin_data_t) );
  for( int i = 0; i < n_bins; ++i )
  {
    int bin_size = 1024 + 1; 
    st->binned_data[i].pts = (v3_t*)malloc( bin_size * sizeof(v3_t) );
    st->binned_data[i].ind =  (int*)malloc( bin_size * sizeof(int) );
    st->binned_data[i].n_pts = 0.0f;
    for( int j = 0; j < bin_size; j++ )
    {
      st->binned_data[i].pts[j] = { .x = -1.0f, .y = -1.0f, .z = -1.0f};
      st->binned_data[i].ind[j] = -1;
    }
  }

  for( int i = 0 ; i < n_pts; ++i )
  {
    v3_t pt = (v3_t){ .x = pts[3*i+0], .y = pts[3*i+1], .z = pts[3*i+2] };
    int bin_idx = msh_st__bin_pt( st, pt );

    int n_pts = st->binned_data[bin_idx].n_pts;
    st->binned_data[bin_idx].n_pts += 1;
    st->binned_data[bin_idx].pts[n_pts] = pt;
    st->binned_data[bin_idx].ind[n_pts] = i;
  }

  st->offsets = (bin_info_t*)malloc( n_bins * sizeof(bin_info_t) );
  st->data = (v3_t*)malloc( n_pts * sizeof(v3_t) );
  st->indices = (int*)malloc( n_pts * sizeof(int) );
  for( int i = 0; i < n_bins; ++i ) { st->offsets[i] = (bin_info_t){.offset = -1, .length = 0}; }
  
  // reorder the binned data into linear array for better cache friendliness
  int offset = 0;
  for( int i = 0; i < n_bins; ++i )
  {
    int n_bin_pts = st->binned_data[i].n_pts;
    for( int j = 0; j < n_bin_pts; ++j )
    {
      st->data[offset+j] = st->binned_data[i].pts[j];
      st->indices[offset+j] = st->binned_data[i].ind[j];
    }
    if( n_bin_pts )
    {
      st->offsets[i] = (bin_info_t) { .offset = offset, .length = n_bin_pts };
      offset += n_bin_pts;
    }
  }
}


void
msh_st__find_neighbors_in_bin( const msh_st_t* st, const uint32_t bin_idx, const float radius_sq,
                               const v3_t pt, float* dists_sq, int* indices, int* n_neigh, const int max_n_neigh )
{
  bin_info_t bi = st->offsets[bin_idx];
  if( !bi.length ) { return; }
  if( (*n_neigh) >= max_n_neigh ) { return; }
  
  v3_t* data = &st->data[bi.offset];
  int* ind = &st->indices[bi.offset];

  for( uint32_t i = 0; i < bi.length; ++i ) 
  { 
    v3_t neigh = data[i]; 
    v3_t v = { neigh.x - pt.x, neigh.y - pt.y, neigh.z - pt.z };
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    
    if( dist_sq <= radius_sq )
    {
      dists_sq[(*n_neigh)] = dist_sq;
      indices[(*n_neigh)]  = ind[i];
      ++(*n_neigh);
      if( (*n_neigh) >= max_n_neigh ) { return; }
    }
  }
}


// TODO: test SOA vs AOS
// TODO: test binned
// TODO: Add optional sorting
// TODO: Allow searching over a number of pts.
int
msh_st_radius_search( const msh_st_t* st, const float* query_pt, const float radius_sq,
                      float* dists_sq, int* indices, int max_n_results )
{
  v3_t pt = (v3_t){ .x = query_pt[0], .y = query_pt[1], .z = query_pt[2] };

  // gather bins & number of points in them
  int ix = (int)( (pt.x - st->min_pt.x) / st->cell_size );
  int iy = (int)( (pt.y - st->min_pt.y) / st->cell_size );
  int iz = (int)( (pt.z - st->min_pt.z) / st->cell_size );
  
  
  uint32_t slab_size = st->height*st->width;
  int neigh_idx = 0;
  for( int32_t oz = -1; oz <= 1; ++oz )
  {
    for( int32_t oy = -1; oy <= 1; ++oy )
    {
      for( int32_t ox = -1; ox <= 1; ++ox )
      { 
        uint32_t bin_idx = (iz + oz) * slab_size + (iy + oy) * st->width + (ix + ox);
        msh_st__find_neighbors_in_bin( st, bin_idx, radius_sq, pt, 
                                       dists_sq, indices, &neigh_idx, max_n_results );
      }
    }
  }
  return neigh_idx;
}

void
msh_st_term( msh_st_t* st )
{
  int n_bins = st->width * st->height * st->depth;
  // for( int i = 0 ; i < n_bins; ++i ) { free( st->binned_data[i] ); }
  // free( st->data );
  // free( st->binned_data );
  st->width = -1;
  st->height = -1;
  st->depth = -1;
  st->cell_size = 0.0f;
}

int main( int argc, char** argv )
{
  printf("This program is an example of a spatial hashtable use\n");

  float cell_size = 0.1f;
  float n_pts = 100000;
  float* pts = (float*)malloc( n_pts * 3 * sizeof(float) );
  for( int i = 0 ; i < n_pts; ++i )
  {
    pts[3*i+0] = (rand() % 100000) / 100000.0f;
    pts[3*i+1] = (rand() % 100000) / 100000.0f;
    pts[3*i+2] = (rand() % 100000) / 100000.0f;
  }
 
  msh_st_t spatial_table;
  uint64_t t2, t1;
  t1 = msh_time_now();
  msh_st_init( &spatial_table, pts, n_pts, cell_size );
  t2 = msh_time_now();
  printf("Table creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  t1 = msh_time_now();
  flann::Matrix<float> pts_mat((float*)&pts[0], n_pts, 3);
  flann_index_t* kdtree = new flann_index_t( pts_mat, flann::KDTreeSingleIndexParams(8));
  kdtree->buildIndex();
  t2 = msh_time_now();
  printf("KD-Tree creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));


  int n_query_pts = 10000;
  float* query_pts = (float*)malloc( n_query_pts * sizeof(float) * 3 );
  for( int i = 0; i < n_query_pts; ++i )
  {
    query_pts[3*i+0] = ((rand() % 100000) / 100000.0f);
    query_pts[3*i+1] = ((rand() % 100000) / 100000.0f);
    query_pts[3*i+2] = ((rand() % 100000) / 100000.0f);
  }

  // msh_st_radius_search_bin( &spatial_table, pt, radius_sq, dists, indices, max_n_results  );
  // msh_st_radius_search_flat( &spatial_table, pt, radius_sq, dists, indices, max_n_results );

  // KNN is much harder, because we need to do a form of breadth first search
  // msh_st_knn_search_bin( &spatial_table, pt, k, dists, indices );
  // msh_st_knn_search_flat( &spatial_table, pt, k, dists, indices );


  t1 = msh_time_now();
  int total_neigh_count = 0;
  float radius_sq = 0.1f * 0.1f;
#define MAX_N_NEIGH 1024
  int nn_inds[MAX_N_NEIGH] = {-1}; float nn_dists[MAX_N_NEIGH] = {1e9};
  for( int i = 0 ; i < n_query_pts; ++i )
  {
    float* pt = &query_pts[3*i];
    total_neigh_count += msh_st_radius_search( &spatial_table, pt, radius_sq, nn_dists, nn_inds, MAX_N_NEIGH );
  }
  t2 = msh_time_now();
  printf("Finding %d points using radius seach funct.: %fms.\n", total_neigh_count, 
                                                       msh_time_diff( MSHT_MILLISECONDS, t2, t1 ) );
  
  double total_time = 0;
  total_neigh_count = 0;
  
  int flann_inds[MAX_N_NEIGH] = {-1}; float flann_dists[MAX_N_NEIGH] = {1e9};
  for( int i = 0 ; i < n_query_pts; ++i )
  {
  
    t1 = msh_time_now();
    float *query = &query_pts[3*i];
    flann::Matrix<float> query_mat(&query[0], 1, 3);
    flann::Matrix<int> indices_mat(&flann_inds[0], 1, MAX_N_NEIGH);
    flann::Matrix<float> dists_mat(&flann_dists[0], 1, MAX_N_NEIGH);
    int test = kdtree->radiusSearch( query_mat, indices_mat, 
                          dists_mat, radius_sq, 
                          flann::SearchParams(128));
    t2 = msh_time_now();
    total_time += (double)msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
    for( int j = 0 ; j < test; ++j )
    {
      // if( nn_inds[j] == -1 ) { printf("%d\n", j); break; }
      total_neigh_count++;
    }
  }
  t2 = msh_time_now();
  printf("Finding %d points using kdtree: %fms.\n", total_neigh_count, total_time );
  
  msh_st_term( &spatial_table );

  return 0;
}