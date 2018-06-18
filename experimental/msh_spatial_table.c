#define MSH_IMPLEMENTATION
#include <cmath>
#include "flann/flann.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "msh.h"


// Issue: Idea of reordering is really bad for meshes... So we would need a copy of data, and still not
//        have indices.

// Write binned vs flat
// Write using indices
// Compare to flann
// Different use case - find cloasest one, not append

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
  // TODO(maciej): actually put this into linear array
  // TODO(maciej): Compare linear array with binned data
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
msh_st__append_neigh_bin( const msh_st_t* st, int bin_idx, int n_bin_pts, float r_sq,
                          v3_t pt, v3_t* neighbors, int* indices, int* n_neigh )
{
  for( int i = 0; i < n_bin_pts; ++i ) 
  { 
    v3_t neigh = st->binned_data[bin_idx].pts[i]; 
    v3_t v = { neigh.x - pt.x, neigh.y - pt.y, neigh.z - pt.z };
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    
    if( dist_sq <= r_sq )
    {
      neighbors[(*n_neigh)] = neigh;
      indices[(*n_neigh)] = st->binned_data[bin_idx].ind[i]; 
      *n_neigh += 1;
    }
  }
}

// TODO(maciej): Check how fast will this work if 
void
msh_st__append_neigh_flat( const msh_st_t* st, int bin_idx, int n_bin_pts, float r_sq,
                           v3_t pt, v3_t* neighbors, int* indices, int* n_neigh )
{
  bin_info_t bi = st->offsets[bin_idx];
  v3_t* data = &st->data[bi.offset];
  int* ind = &st->indices[bi.offset];
  for( int i = 0; i < bi.length; ++i ) 
  { 
    v3_t neigh = data[i]; 
    v3_t v = { neigh.x - pt.x, neigh.y - pt.y, neigh.z - pt.z };
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    
    if( dist_sq <= r_sq )
    {
      neighbors[(*n_neigh)] = neigh;
      indices[(*n_neigh)]   = ind[i];
      *n_neigh += 1;
    }
  }
}

void
msh_st__find_neighbors_in_bin( const msh_st_t* st, int bin_idx, float radius_sq,
                               const v3_t pt, float* dists_sq, int* indices, int* n_neigh, const int max_n_neigh )
{
  if( (*n_neigh) >= max_n_neigh ) { return; }
  bin_info_t bi = st->offsets[bin_idx];
  v3_t* data = &st->data[bi.offset];
  int* ind = &st->indices[bi.offset];

  for( int i = 0; i < bi.length; ++i ) 
  { 
    v3_t neigh = data[i]; 
    v3_t v = { neigh.x - pt.x, neigh.y - pt.y, neigh.z - pt.z };
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    
    if( dist_sq <= radius_sq )
    {
      dists_sq[(*n_neigh)] = dist_sq;
      indices[(*n_neigh)]  = ind[i];
      *n_neigh += 1;
      if( (*n_neigh) >= max_n_neigh ) { return; }
    }
  }
}

int
msh_st_find_neighbors_flat( const msh_st_t* st, float *in_pt, float radius_sq )
{
  // gather bins & number of points in them
  v3_t pt = (v3_t){ .x = in_pt[0], .y = in_pt[1], .z = in_pt[2] };
  int ix = floor( (pt.x - st->min_pt.x) / st->cell_size );
  int iy = floor( (pt.y - st->min_pt.y) / st->cell_size );
  int iz = floor( (pt.z - st->min_pt.z) / st->cell_size );
  
  int slab_size = st->height*st->width;

  int bin_idx_00 = (iz-1) * slab_size + (iy-1) * st->width + (ix-1);
  int bin_idx_01 = (iz-1) * slab_size + (iy-1) * st->width + (ix);
  int bin_idx_02 = (iz-1) * slab_size + (iy-1) * st->width + (ix+1);
  
  int bin_idx_03 = (iz-1) * slab_size + (iy) * st->width + (ix-1);
  int bin_idx_04 = (iz-1) * slab_size + (iy) * st->width + (ix);
  int bin_idx_05 = (iz-1) * slab_size + (iy) * st->width + (ix+1);

  int bin_idx_06 = (iz-1) * slab_size + (iy+1) * st->width + (ix-1);
  int bin_idx_07 = (iz-1) * slab_size + (iy+1) * st->width + (ix);
  int bin_idx_08 = (iz-1) * slab_size + (iy+1) * st->width + (ix+1);


  int bin_idx_09 = (iz) * slab_size + (iy-1) * st->width + (ix-1);
  int bin_idx_10 = (iz) * slab_size + (iy-1) * st->width + (ix);
  int bin_idx_11 = (iz) * slab_size + (iy-1) * st->width + (ix+1);
  
  int bin_idx_12 = (iz) * slab_size + (iy) * st->width + (ix-1);
  int bin_idx_13 = (iz) * slab_size + (iy) * st->width + (ix);
  int bin_idx_14 = (iz) * slab_size + (iy) * st->width + (ix+1);

  int bin_idx_15 = (iz) * slab_size + (iy+1) * st->width + (ix-1);
  int bin_idx_16 = (iz) * slab_size + (iy+1) * st->width + (ix);
  int bin_idx_17 = (iz) * slab_size + (iy+1) * st->width + (ix+1);


  int bin_idx_18 = (iz+1) * slab_size + (iy-1) * st->width + (ix-1);
  int bin_idx_19 = (iz+1) * slab_size + (iy-1) * st->width + (ix);
  int bin_idx_20 = (iz+1) * slab_size + (iy-1) * st->width + (ix+1);
  
  int bin_idx_21 = (iz+1) * slab_size + (iy) * st->width + (ix-1);
  int bin_idx_22 = (iz+1) * slab_size + (iy) * st->width + (ix);
  int bin_idx_23 = (iz+1) * slab_size + (iy) * st->width + (ix+1);

  int bin_idx_24 = (iz+1) * slab_size + (iy+1) * st->width + (ix-1);
  int bin_idx_25 = (iz+1) * slab_size + (iy+1) * st->width + (ix);
  int bin_idx_26 = (iz+1) * slab_size + (iy+1) * st->width + (ix+1);
  
  int n_pts_00 = st->offsets[bin_idx_00].length;
  int n_pts_01 = st->offsets[bin_idx_01].length;
  int n_pts_02 = st->offsets[bin_idx_02].length;
  int n_pts_03 = st->offsets[bin_idx_03].length;
  int n_pts_04 = st->offsets[bin_idx_04].length;
  int n_pts_05 = st->offsets[bin_idx_05].length;
  int n_pts_06 = st->offsets[bin_idx_06].length;
  int n_pts_07 = st->offsets[bin_idx_07].length;
  int n_pts_08 = st->offsets[bin_idx_08].length;
  int n_pts_09 = st->offsets[bin_idx_09].length;
  int n_pts_10 = st->offsets[bin_idx_10].length;
  int n_pts_11 = st->offsets[bin_idx_11].length;
  int n_pts_12 = st->offsets[bin_idx_12].length;
  int n_pts_13 = st->offsets[bin_idx_13].length;
  int n_pts_14 = st->offsets[bin_idx_14].length;
  int n_pts_15 = st->offsets[bin_idx_15].length;
  int n_pts_16 = st->offsets[bin_idx_16].length;
  int n_pts_17 = st->offsets[bin_idx_17].length;
  int n_pts_18 = st->offsets[bin_idx_18].length;
  int n_pts_19 = st->offsets[bin_idx_19].length;
  int n_pts_20 = st->offsets[bin_idx_20].length;
  int n_pts_21 = st->offsets[bin_idx_21].length;
  int n_pts_22 = st->offsets[bin_idx_22].length;
  int n_pts_23 = st->offsets[bin_idx_23].length;
  int n_pts_24 = st->offsets[bin_idx_24].length;
  int n_pts_25 = st->offsets[bin_idx_25].length;
  int n_pts_26 = st->offsets[bin_idx_26].length;

  v3_t neighbors[4096] = {0};
  int indices[4096] = {0};
  int neigh_idx = 0;
  msh_st__append_neigh_flat( st, bin_idx_00, n_pts_00, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_01, n_pts_01, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_02, n_pts_02, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_03, n_pts_03, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_04, n_pts_04, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_05, n_pts_05, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_06, n_pts_06, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_07, n_pts_07, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_08, n_pts_08, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_09, n_pts_09, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_10, n_pts_10, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_11, n_pts_11, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_12, n_pts_12, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_13, n_pts_13, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_14, n_pts_14, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_15, n_pts_15, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_16, n_pts_16, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_17, n_pts_17, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_18, n_pts_18, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_19, n_pts_19, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_20, n_pts_20, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_21, n_pts_21, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_22, n_pts_22, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_23, n_pts_23, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_24, n_pts_24, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_25, n_pts_25, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_flat( st, bin_idx_26, n_pts_26, radius_sq, pt, neighbors, indices, &neigh_idx );

  return neigh_idx;
}

int
msh_st_radius_search( const msh_st_t* st, const float* query_pt, const float radius_sq,
                      float* dists_sq, int* indices, int max_n_results )
{
  v3_t pt = (v3_t){ .x = query_pt[0], .y = query_pt[1], .z = query_pt[2] };

  // gather bins & number of points in them
  int ix = floor( (pt.x - st->min_pt.x) / st->cell_size );
  int iy = floor( (pt.y - st->min_pt.y) / st->cell_size );
  int iz = floor( (pt.z - st->min_pt.z) / st->cell_size );
  
  int slab_size = st->height*st->width;
  int slab_size_a = (iz-1) * slab_size; 
  int slab_size_b = (iz) * slab_size; 
  int slab_size_c = (iz+1) * slab_size; 

  int row_size_a = (iy-1) * st->width;
  int row_size_b = (iy) * st->width;
  int row_size_c = (iy+1) * st->width;

  int bin_idx_00 = slab_size_a + row_size_a + (ix-1);
  int bin_idx_01 = slab_size_a + row_size_a + (ix);
  int bin_idx_02 = slab_size_a + row_size_a + (ix+1);
  
  int bin_idx_03 = slab_size_a + row_size_b + (ix-1);
  int bin_idx_04 = slab_size_a + row_size_b + (ix);
  int bin_idx_05 = slab_size_a + row_size_b + (ix+1);

  int bin_idx_06 = slab_size_a + row_size_c + (ix-1);
  int bin_idx_07 = slab_size_a + row_size_c + (ix);
  int bin_idx_08 = slab_size_a + row_size_c + (ix+1);


  int bin_idx_09 = slab_size_b + row_size_a + (ix-1);
  int bin_idx_10 = slab_size_b + row_size_a + (ix);
  int bin_idx_11 = slab_size_b + row_size_a + (ix+1);
  
  int bin_idx_12 = slab_size_b + row_size_b + (ix-1);
  int bin_idx_13 = slab_size_b + row_size_b + (ix);
  int bin_idx_14 = slab_size_b + row_size_b + (ix+1);

  int bin_idx_15 = slab_size_b + row_size_c + (ix-1);
  int bin_idx_16 = slab_size_b + row_size_c + (ix);
  int bin_idx_17 = slab_size_b + row_size_c + (ix+1);


  int bin_idx_18 = slab_size_c + row_size_a + (ix-1);
  int bin_idx_19 = slab_size_c + row_size_a + (ix);
  int bin_idx_20 = slab_size_c + row_size_a + (ix+1);
  
  int bin_idx_21 = slab_size_c + row_size_b + (ix-1);
  int bin_idx_22 = slab_size_c + row_size_b + (ix);
  int bin_idx_23 = slab_size_c + row_size_b + (ix+1);

  int bin_idx_24 = slab_size_c + row_size_c + (ix-1);
  int bin_idx_25 = slab_size_c + row_size_c + (ix);
  int bin_idx_26 = slab_size_c + row_size_c + (ix+1);

  int neigh_idx = 0;
  msh_st__find_neighbors_in_bin( st, bin_idx_00, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_01, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_02, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_03, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_04, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_05, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_06, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_07, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_08, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_09, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_10, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_11, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_12, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_13, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_14, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_15, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_16, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_17, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_18, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_19, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_20, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_21, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_22, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_23, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_24, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_25, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );
  msh_st__find_neighbors_in_bin( st, bin_idx_26, radius_sq, pt, dists_sq, indices, &neigh_idx, max_n_results );

  return neigh_idx;
}

int
msh_st_find_neighbors_bin( const msh_st_t* st, float *in_pt, float radius_sq )
{
  v3_t pt = (v3_t){ .x = in_pt[0], .y = in_pt[1], .z = in_pt[2] };

  // gather bins & number of points in them
  int ix = floor( (pt.x - st->min_pt.x) / st->cell_size );
  int iy = floor( (pt.y - st->min_pt.y) / st->cell_size );
  int iz = floor( (pt.z - st->min_pt.z) / st->cell_size );
  
  int slab_size = st->height*st->width;

  int base_slab = (iz-1) * slab_size;
  int bin_idx_00 = (iz-1) * slab_size + (iy-1) * st->width + (ix-1);
  int bin_idx_01 = (iz-1) * slab_size + (iy-1) * st->width + (ix);
  int bin_idx_02 = (iz-1) * slab_size + (iy-1) * st->width + (ix+1);
  
  int bin_idx_03 = (iz-1) * slab_size + (iy) * st->width + (ix-1);
  int bin_idx_04 = (iz-1) * slab_size + (iy) * st->width + (ix);
  int bin_idx_05 = (iz-1) * slab_size + (iy) * st->width + (ix+1);

  int bin_idx_06 = (iz-1) * slab_size + (iy+1) * st->width + (ix-1);
  int bin_idx_07 = (iz-1) * slab_size + (iy+1) * st->width + (ix);
  int bin_idx_08 = (iz-1) * slab_size + (iy+1) * st->width + (ix+1);


  int bin_idx_09 = (iz) * slab_size + (iy-1) * st->width + (ix-1);
  int bin_idx_10 = (iz) * slab_size + (iy-1) * st->width + (ix);
  int bin_idx_11 = (iz) * slab_size + (iy-1) * st->width + (ix+1);
  
  int bin_idx_12 = (iz) * slab_size + (iy) * st->width + (ix-1);
  int bin_idx_13 = (iz) * slab_size + (iy) * st->width + (ix);
  int bin_idx_14 = (iz) * slab_size + (iy) * st->width + (ix+1);

  int bin_idx_15 = (iz) * slab_size + (iy+1) * st->width + (ix-1);
  int bin_idx_16 = (iz) * slab_size + (iy+1) * st->width + (ix);
  int bin_idx_17 = (iz) * slab_size + (iy+1) * st->width + (ix+1);


  int bin_idx_18 = (iz+1) * slab_size + (iy-1) * st->width + (ix-1);
  int bin_idx_19 = (iz+1) * slab_size + (iy-1) * st->width + (ix);
  int bin_idx_20 = (iz+1) * slab_size + (iy-1) * st->width + (ix+1);
  
  int bin_idx_21 = (iz+1) * slab_size + (iy) * st->width + (ix-1);
  int bin_idx_22 = (iz+1) * slab_size + (iy) * st->width + (ix);
  int bin_idx_23 = (iz+1) * slab_size + (iy) * st->width + (ix+1);

  int bin_idx_24 = (iz+1) * slab_size + (iy+1) * st->width + (ix-1);
  int bin_idx_25 = (iz+1) * slab_size + (iy+1) * st->width + (ix);
  int bin_idx_26 = (iz+1) * slab_size + (iy+1) * st->width + (ix+1);

  //NOTE(maciej): Need to binning by the cell size in each dim.
  int n_pts_00 = (int)st->binned_data[bin_idx_00].n_pts; 
  int n_pts_01 = (int)st->binned_data[bin_idx_01].n_pts; 
  int n_pts_02 = (int)st->binned_data[bin_idx_02].n_pts; 
  int n_pts_03 = (int)st->binned_data[bin_idx_03].n_pts; 
  int n_pts_04 = (int)st->binned_data[bin_idx_04].n_pts; 
  int n_pts_05 = (int)st->binned_data[bin_idx_05].n_pts; 
  int n_pts_06 = (int)st->binned_data[bin_idx_06].n_pts; 
  int n_pts_07 = (int)st->binned_data[bin_idx_07].n_pts; 
  int n_pts_08 = (int)st->binned_data[bin_idx_08].n_pts; 
  int n_pts_09 = (int)st->binned_data[bin_idx_09].n_pts; 
  int n_pts_10 = (int)st->binned_data[bin_idx_10].n_pts; 
  int n_pts_11 = (int)st->binned_data[bin_idx_11].n_pts; 
  int n_pts_12 = (int)st->binned_data[bin_idx_12].n_pts; 
  int n_pts_13 = (int)st->binned_data[bin_idx_13].n_pts; 
  int n_pts_14 = (int)st->binned_data[bin_idx_14].n_pts; 
  int n_pts_15 = (int)st->binned_data[bin_idx_15].n_pts; 
  int n_pts_16 = (int)st->binned_data[bin_idx_16].n_pts; 
  int n_pts_17 = (int)st->binned_data[bin_idx_17].n_pts; 
  int n_pts_18 = (int)st->binned_data[bin_idx_18].n_pts; 
  int n_pts_19 = (int)st->binned_data[bin_idx_19].n_pts; 
  int n_pts_20 = (int)st->binned_data[bin_idx_20].n_pts; 
  int n_pts_21 = (int)st->binned_data[bin_idx_21].n_pts; 
  int n_pts_22 = (int)st->binned_data[bin_idx_22].n_pts; 
  int n_pts_23 = (int)st->binned_data[bin_idx_23].n_pts; 
  int n_pts_24 = (int)st->binned_data[bin_idx_24].n_pts; 
  int n_pts_25 = (int)st->binned_data[bin_idx_25].n_pts; 
  int n_pts_26 = (int)st->binned_data[bin_idx_26].n_pts; 
  
  // NOTE(maciej): We possibly want to avoid malloc
  v3_t neighbors[4096] = {0};
  int indices[4096] = {0};
  int neigh_idx = 0;
  msh_st__append_neigh_bin( st, bin_idx_00, n_pts_00, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_01, n_pts_01, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_02, n_pts_02, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_03, n_pts_03, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_04, n_pts_04, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_05, n_pts_05, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_06, n_pts_06, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_07, n_pts_07, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_08, n_pts_08, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_09, n_pts_09, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_10, n_pts_10, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_11, n_pts_11, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_12, n_pts_12, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_13, n_pts_13, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_14, n_pts_14, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_15, n_pts_15, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_16, n_pts_16, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_17, n_pts_17, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_18, n_pts_18, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_19, n_pts_19, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_20, n_pts_20, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_21, n_pts_21, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_22, n_pts_22, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_23, n_pts_23, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_24, n_pts_24, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_25, n_pts_25, radius_sq, pt, neighbors, indices, &neigh_idx );
  msh_st__append_neigh_bin( st, bin_idx_26, n_pts_26, radius_sq, pt, neighbors, indices, &neigh_idx );
  
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

  int total_neigh_count = 0;
  for( int i = 0 ; i < n_query_pts; ++i )
  {
    float* pt = &query_pts[3*i];
    total_neigh_count += msh_st_find_neighbors_bin(&spatial_table, pt, cell_size*cell_size );
  }
  t2 = msh_time_now();
  printf("Finding %d points using binned data: %fms.\n", total_neigh_count, msh_time_diff( MSHT_MILLISECONDS, t2, t1 ) );
  

  t1 = msh_time_now();
  total_neigh_count = 0;
  int nn_inds[128] = {-1}; float nn_dists[128] = {1e9};
  for( int i = 0 ; i < n_query_pts; ++i )
  {
    float* pt = &query_pts[3*i];
    total_neigh_count += msh_st_radius_search( &spatial_table, pt, cell_size*cell_size, nn_dists, nn_inds, 128 );
  }
  t2 = msh_time_now();
  printf("Finding %d points using radius seach funct.: %fms.\n", total_neigh_count, msh_time_diff( MSHT_MILLISECONDS, t2, t1 ) );
  
  total_neigh_count = 0;
  t1 = msh_time_now();
  for( int i = 0 ; i < n_query_pts; ++i )
  {
    float* pt = &query_pts[3*i];
    total_neigh_count += msh_st_find_neighbors_flat(&spatial_table, pt, cell_size*cell_size );
  }
  t2 = msh_time_now();
  printf("Finding %d points using flattened data: %fms.\n", total_neigh_count, msh_time_diff( MSHT_MILLISECONDS, t2, t1 ) );
  
  // NOTE(maciej): Flann might be slower because of the sorting it does,
  double total_time = 0;
  total_neigh_count = 0;
  
  int flann_inds[128] = {-1}; float flann_dists[128] = {1e9};
  for( int i = 0 ; i < n_query_pts; ++i )
  {
  
    t1 = msh_time_now();
    float *query = &query_pts[3*i];
    // for( int i = 0 ; i < 128; ++i )
    flann::Matrix<float> query_mat(&query[0], 1, 3);
    flann::Matrix<int> indices_mat(&flann_inds[0], 1, 128);
    flann::Matrix<float> dists_mat(&flann_dists[0], 1, 128);
    int test = kdtree->radiusSearch( query_mat, indices_mat, 
                          dists_mat, cell_size*cell_size, 
                          flann::SearchParams(128));
    t2 = msh_time_now();
    total_time += (double)msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
    for( int j = 0 ; j < test; ++j )
    {
      if( nn_inds[j] == -1 ) { printf("%d\n", j); break; }
      total_neigh_count++;
    }
  }
  t2 = msh_time_now();
  printf("Finding %d points using kdtree: %fms.\n", total_neigh_count, total_time );
  
  msh_st_term( &spatial_table );

  return 0;
}