#define MSH_IMPLEMENTATION
#define MSH_PLY_IMPLEMENTATION
#include <cmath>
#include "flann/flann.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "msh.h"
#include "experimental/msh_ply.h"


// TODO: test SOA vs AOS -- no appreciable difference. Will need to benchmark this better.
// TODO: Test with actual meshes.
// TODO: Creation should aim at some set density of hash table, and then select cell size based on that.
// TODO: Check what causes a slow down in terms of creating the table if cells are small
// TODO: Add optional sorting
// TODO: Allow searching over a number of pts.




typedef flann::Index<flann::L2_Simple<float>> flann_index_t;
typedef struct v3 { float x, y, z; } v3_t;
typedef struct bin_data { int n_pts; v3_t* pts; int* ind; } bin_data_t;
typedef struct bin_info{ int offset; int length; } bin_info_t;
typedef struct pt_idx{ v3_t pt; int idx; } pt_idx_t;

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
  pt_idx_t* pt_idx_data; 
  
  bin_data_t* binned_data; // this might end up being temporary
  int   _slab_size;
  float _inv_cell_size;
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
  // printf("  %f %f %f\n", st->min_pt.x, st->min_pt.y, st->min_pt.z );
  // printf("  %f %f %f\n", st->max_pt.x, st->max_pt.y, st->max_pt.z );
  // st->cell_size = cell_size;
  // st->width     = (int)ceilf((float)(st->max_pt.x - st->min_pt.x) / st->cell_size);
  // st->height    = (int)ceilf((float)(st->max_pt.y - st->min_pt.y) / st->cell_size);
  // st->depth     = (int)ceilf((float)(st->max_pt.z - st->min_pt.z) / st->cell_size);
  // printf("%f\n", st->max_pt.x - st->min_pt.x );
  // printf("%d %d %d\n", st->width, st->height, st->depth );

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
  
  st->_inv_cell_size = 1.0f / st->cell_size;
  st->_slab_size = st->height * st->width;

  // create tmp binned storage
  // TODO(maciej): Use ion array here.
  int n_bins = st->width * st->height * st->depth;
  st->binned_data = (bin_data_t*)malloc( n_bins * sizeof(bin_data_t) );
  for( int i = 0; i < n_bins; ++i )
  {
    st->binned_data[i].pts = {0};
    st->binned_data[i].ind = {0};
    st->binned_data[i].n_pts = 0;
  }

  for( int i = 0 ; i < n_pts; ++i )
  {
    v3_t pt = (v3_t){ .x = pts[3*i+0], .y = pts[3*i+1], .z = pts[3*i+2] };
    int bin_idx = msh_st__bin_pt( st, pt );

    int n_pts = st->binned_data[bin_idx].n_pts;
    st->binned_data[bin_idx].n_pts += 1;
    msh_array_push( st->binned_data[bin_idx].pts, pt);
    msh_array_push( st->binned_data[bin_idx].ind, i);
  }

  st->offsets = (bin_info_t*)malloc( n_bins * sizeof(bin_info_t) );
  st->data = (v3_t*)malloc( n_pts * sizeof(v3_t) );
  st->indices = (int*)malloc( n_pts * sizeof(int) );
  st->pt_idx_data = (pt_idx_t*)malloc( n_pts * sizeof(pt_idx_t) );
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
      st->pt_idx_data[offset+j] = (pt_idx_t){ .pt = st->binned_data[i].pts[j], .idx = st->binned_data[i].ind[j] };
    }
    if( n_bin_pts )
    {
      st->offsets[i] = (bin_info_t) { .offset = offset, .length = n_bin_pts };
      offset += n_bin_pts;
    }
  }
}


inline void
msh_st__find_neighbors_in_bin_flat( const msh_st_t* st, const uint32_t bin_idx, const double radius_sq,
                               const v3_t pt, float* dists_sq, int* indices, int* n_neigh, const int max_n_neigh )
{
  bin_info_t bi = st->offsets[bin_idx];
  if( !bi.length ) { return; }
  if( (*n_neigh) >= max_n_neigh ) { return; }
  
  const v3_t* data = &st->data[bi.offset];
  const int* ind = &st->indices[bi.offset];
  // const pt_idx_t* data = &st->pt_idx_data[bi.offset];
  for( uint32_t i = 0; i < bi.length; ++i ) 
  { 
    v3_t v = { data[i].x - pt.x, data[i].y - pt.y, data[i].z - pt.z };
    // v3_t v = { data[i].pt.x - pt.x, data[i].pt.y - pt.y, data[i].pt.z - pt.z };
    double dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    
    if( dist_sq <= radius_sq )
    {
      dists_sq[(*n_neigh)] = dist_sq;
      indices[(*n_neigh)]  = ind[i];
      // indices[(*n_neigh)]  = data[i].idx;
      ++(*n_neigh);
      if( (*n_neigh) >= max_n_neigh ) { return; }
    }
  }
}


int
msh_st_radius_search_flat( const msh_st_t* st, const float* query_pt, const float radius,
                      float* dists_sq, int* indices, int max_n_results )
{
  v3_t pt = (v3_t){ .x = query_pt[0], .y = query_pt[1], .z = query_pt[2] };

  // gather bins & number of points in them
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
  
  // printf("pt: %f %f %f | %f | (%d, %d) (%d, %d) (%d, %d)\n", pt.x, pt.y, pt.z, radius, onx, opx, ony, opy, onz, opz);
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
        // printf("%d %d %d\n", iz+oz, iy+oy, ix +oz);
        if( ix + ox < 0) continue;
        if( ix + ox >= st->width ) continue;
        uint32_t bin_idx = idx_z + idx_y + (ix + ox);
        msh_st__find_neighbors_in_bin_flat( st, bin_idx, radius_sq, pt, 
                                            dists_sq, indices, &neigh_idx, max_n_results );
      }
    }
  }

  return neigh_idx;
}

inline void
msh_st__find_neighbors_in_bin_bin( const msh_st_t* st, const uint32_t bin_idx, const float radius_sq,
                               const v3_t pt, float* dists_sq, int* indices, int* n_neigh, const int max_n_neigh )
{
  const bin_data_t* bi = &st->binned_data[bin_idx];
  if( !bi->n_pts ) { return; }
  if( (*n_neigh) >= max_n_neigh ) { return; }

  for( uint32_t i = 0; i < bi->n_pts; ++i ) 
  { 
    v3_t v = { bi->pts[i].x - pt.x, bi->pts[i].y - pt.y, bi->pts[i].z - pt.z };
    float dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    
    if( dist_sq <= radius_sq )
    {
      dists_sq[(*n_neigh)] = dist_sq;
      indices[(*n_neigh)]  = bi->ind[i];
      ++(*n_neigh);
      if( (*n_neigh) >= max_n_neigh ) { return; }
    }
  }
}

int
msh_st_radius_search_bin( const msh_st_t* st, const float* query_pt, const float radius,
                         float* dists_sq, int* indices, int max_n_results )
{
  v3_t pt = (v3_t){ .x = query_pt[0], .y = query_pt[1], .z = query_pt[2] };

  // gather bins & number of points in them
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
  
  // printf("pt: %f %f %f | %f | (%d, %d) (%d, %d) (%d, %d)\n", pt.x, pt.y, pt.z, sqrt(radius_sq), onx, opx, ony, opy, onz, opz);
  int neigh_idx = 0;
  float radius_sq = radius * radius;
  for( int8_t oz = onz; oz <= opz; ++oz )
  {
    uint32_t idx_z = (iz + oz) * st->_slab_size;
    for( int8_t oy = ony; oy <= opy; ++oy )
    {
      uint32_t idx_y = (iy + oy) * st->width;
      for( int8_t ox = onx; ox <= opx; ++ox )
      { 
        uint32_t bin_idx = idx_z + idx_y + (ix + ox);
        msh_st__find_neighbors_in_bin_bin( st, bin_idx, radius_sq, pt, 
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

typedef struct pointcloud
{
  int n_pts;
  v3_t* pts;
} PointCloud;

void read_ply( const char* filename, PointCloud* pc )
{
  ply_file_t* pf = ply_file_open( filename, "rb");
  if( pf )
  {
    const char* positions_names[] = { "x", "y", "z" };
    ply_file_property_desc_t vertex_desc = { .element_name = (char*)"vertex",
                                             .property_names = positions_names,
                                             .num_properties = 3,
                                             .data_type = PLY_FLOAT,
                                             .data = &pc->pts,
                                             .data_count = &pc->n_pts };
    ply_file_add_descriptor( pf, &vertex_desc );
    int test = ply_file_read(pf);
    printf("Error = %d \n", test);
  }
  ply_file_close(pf);
}

int main( int argc, char** argv )
{
  printf("This program is an example of a spatial hashtable use\n");
  uint64_t t2, t1;
  t1 = msh_time_now();
  PointCloud pc = {0};
  read_ply( argv[1], &pc );
  t2 = msh_time_now();
  printf("Reading ply file with %d pts took %fms.\n", pc.n_pts, msh_time_diff(MSHT_MILLISECONDS, t2, t1));
  float cell_size = 0.01f; 
 
  msh_st_t spatial_table;
  t1 = msh_time_now();
  msh_st_init( &spatial_table, (float*)&pc.pts[0], pc.n_pts, cell_size );
  t2 = msh_time_now();
  printf("Table creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  t1 = msh_time_now();
  flann::Matrix<float> pts_mat((float*)&pc.pts[0], pc.n_pts, 3);
  flann_index_t* kdtree = new flann_index_t( pts_mat, flann::KDTreeSingleIndexParams(16));
  kdtree->buildIndex();
  t2 = msh_time_now();
  printf("KD-Tree creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));


  int n_query_pts = 100;
  float* query_pts = (float*)malloc( n_query_pts * sizeof(float) * 3 );
  for( int i = 0; i < n_query_pts; ++i )
  {
    int idx = rand() % pc.n_pts;
    query_pts[3*i+0] = pc.pts[idx].x;
    query_pts[3*i+1] = pc.pts[idx].y;
    query_pts[3*i+2] = pc.pts[idx].z;
  }
  
  // msh_st_radius_search_bin( &spatial_table, pt, radius_sq, dists, indices, max_n_results  );
  // msh_st_radius_search_flat( &spatial_table, pt, radius_sq, dists, indices, max_n_results );

  // KNN is much harder, because we need to do a form of breadth first search
  // msh_st_knn_search_bin( &spatial_table, pt, k, dists, indices );
  // msh_st_knn_search_flat( &spatial_table, pt, k, dists, indices );

  float radius = 0.02f;
  float radius_sq = radius*radius;
  t1 = msh_time_now();
  int total_neigh_count = 0;
  int* neigh_count_a = (int*)malloc(n_query_pts * sizeof(int));
  int* neigh_count_b = (int*)malloc(n_query_pts * sizeof(int));
#define MAX_N_NEIGH 32000
  int nn_inds[MAX_N_NEIGH] = {-1}; float nn_dists[MAX_N_NEIGH] = {1e9};
  for( int i = 0 ; i < n_query_pts; ++i )
  {
    float* pt = &query_pts[3*i];
    neigh_count_a[i] = msh_st_radius_search_flat( &spatial_table, pt, radius, nn_dists, nn_inds, MAX_N_NEIGH );
    total_neigh_count += neigh_count_a[i];
    
  }
  t2 = msh_time_now();
  printf("Finding %d points using radius seach flat funct.: %fms.\n", total_neigh_count, 
                                                       msh_time_diff( MSHT_MILLISECONDS, t2, t1 ) );
  
  // total_neigh_count = 0;
  // t1 = msh_time_now();
  // for( int i = 0 ; i < n_query_pts; ++i )
  // {
  //   float* pt = &query_pts[3*i];
  //   total_neigh_count += msh_st_radius_search_bin( &spatial_table, pt, radius, nn_dists, nn_inds, MAX_N_NEIGH );
  // }
  // t2 = msh_time_now();
  // printf("Finding %d points using radius seach bin funct.: %fms.\n", total_neigh_count, 
  //                                                      msh_time_diff( MSHT_MILLISECONDS, t2, t1 ) );
  
  total_neigh_count = 0;
  flann::SearchParams params = flann::SearchParams(128, 0, 1);
  params.max_neighbors = MAX_N_NEIGH;
  t1 = msh_time_now();
  for( int i = 0 ; i < n_query_pts; ++i )
  {
      int flann_inds[MAX_N_NEIGH] = {-1}; float flann_dists[MAX_N_NEIGH] = {1e9};
    float *query = &query_pts[3*i];
    flann::Matrix<float> query_mat(&query[0], 1, 3);
    flann::Matrix<int> indices_mat(&flann_inds[0], 1, MAX_N_NEIGH);
    flann::Matrix<float> dists_mat(&flann_dists[0], 1, MAX_N_NEIGH);
    neigh_count_b[i] = kdtree->radiusSearch( query_mat, indices_mat, 
                                             dists_mat, radius_sq, 
                                             params );
    total_neigh_count += neigh_count_b[i];
    // if( i == 74 )
    // {
    //   for( int j = 0; j < neigh_count_b[i]; ++j )
    //   {
    //     v3_t pt_a = (v3_t){ .x = query[0], .y = query[1], .z = query[2] };
    //     v3_t pt_b =  pc.pts[flann_inds[j]];
    //     v3_t v = (v3_t){ .x = pt_a.x - pt_b.x, .y = pt_a.y -pt_b.y, .z = pt_a.z -pt_b.z };
    //     double dist_sq = v.x * v.x + v.y * v.y + v.z * v.z;
    //     printf("%d %f %f\n", j, sqrt(dist_sq), radius );
    //   }
    // }
    
  }
  
  t2 = msh_time_now();
  printf("Finding %d points using kdtree: %fms.\n", total_neigh_count, 
                                                       msh_time_diff( MSHT_MILLISECONDS, t2, t1 ) );
  
  msh_st_term( &spatial_table );
  // for( int i = 0 ; i < n_query_pts; ++i )
  // {
  //   printf("%d %d %d\n", i, neigh_count_a[i], neigh_count_b[i] );
  // }
  return 0;
}