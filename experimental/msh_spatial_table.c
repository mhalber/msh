#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Issue: Idea of reordering is really bad for meshes... So we would need a copy of data, and still not
//        have indices.

typedef struct msh_spatial_table
{
  int width;
  int height;
  int depth;
  float cell_size;
  float *data;
} msh_st_t;

void msh_st_init( msh_st_t* st, float* pts, int n_pts, float cell_size );
void msh_st_term( msh_st_t* st );
void msh_st_find_neighbors( msh_st_t* st, float radius );

void
msh_st_init( msh_st_t* st, float* pts, int n_pts, float cell_size )
{
  typedef struct v3 { float x, y, z; } v3_t;

  // build a bbox
  v3_t min_pt = { .x = 1e9, .y = 1e9, .z = 1e9 };
  v3_t max_pt = { .x = -1e9, .y = -1e9, .z = -1e9 };
  for( int i = 0; i < n_pts; ++i )
  {
    float* pt_ptr = &pts[3*i];
    v3_t pt = { .x = pt_ptr[0], .y = pt_ptr[1], .z = pt_ptr[2] };
    min_pt.x = (min_pt.x > pt.x) ? pt.x : min_pt.x;
    min_pt.y = (min_pt.y > pt.y) ? pt.y : min_pt.y;
    min_pt.z = (min_pt.z > pt.z) ? pt.z : min_pt.z;
    max_pt.x = (max_pt.x < pt.x) ? pt.x : max_pt.x;
    max_pt.y = (max_pt.y < pt.y) ? pt.y : max_pt.y;
    max_pt.z = (max_pt.z < pt.z) ? pt.z : max_pt.z;
  }
  printf("  %f %f %f\n", min_pt.x, min_pt.y, min_pt.z );
  printf("  %f %f %f\n", max_pt.x, max_pt.y, max_pt.z );
  // get with height and size
  st->cell_size = cell_size;
  st->width     = (int)ceilf((float)(max_pt.x - min_pt.x) / st->cell_size);
  st->height    = (int)ceilf((float)(max_pt.y - min_pt.y) / st->cell_size);
  st->depth     = (int)ceilf((float)(max_pt.z - min_pt.z) / st->cell_size);

  // create tmp binned storage
  // TODO(maciej): Use ion array here.
  int n_bins = st->width * st->height * st->depth;
  float** binned_data = (float**)malloc( n_bins * sizeof(float*) );
  for( int i = 0; i < n_bins; ++i )
  {
    int bin_size = 1024 + 1;
    binned_data[i] = (float*)malloc( bin_size * 3 * sizeof(float) );
    binned_data[i][0] = 0.0f;
    for( int j = 0; j < bin_size; j++ )
    {
      binned_data[i][1+3*j+0] = -1.0f;
      binned_data[i][1+3*j+1] = -1.0f;
      binned_data[i][1+3*j+2] = -1.0f;
    }
  }

  float eps = 0.0f;
  for( int i = 0 ; i < n_pts; ++i )
  {
    float x = pts[3*i+0];
    float y = pts[3*i+1];
    float z = pts[3*i+2];
    
    int ix = floor( (x - min_pt.x + eps) / st->cell_size );
    int iy = floor( (y - min_pt.y + eps) / st->cell_size );
    int iz = floor( (z - min_pt.z + eps) / st->cell_size );
    int bin_idx = iz * st->height*st->width + iy * st->width + ix;

    int n_pts = binned_data[bin_idx][0];
    binned_data[bin_idx][0] += 1.0f;
    binned_data[bin_idx][1+3*n_pts+0] = x;
    binned_data[bin_idx][1+3*n_pts+1] = y;
    binned_data[bin_idx][1+3*n_pts+2] = z;
  }

  // reorder the binned data into linear array for better cache friendliness
  // TODO(maciej): actually put this into linear array
  // TODO(maciej): Compare linear array with binned data
  for( int z = 0; z < st->depth; ++z )
  {
    for( int y = 0; y < st->height; ++y )
    {
      for( int x = 0; x < st->width; ++x )
      {
        int bin_idx = z * st->height*st->width + y * st->width + x;
        int n_pts = (int)binned_data[bin_idx][0];
        if( n_pts )
        {
          printf("Bin %d(%3.2f) %d(%3.2f) %d(%3.2f) -> %d : %d pts.\n", x, x*st->cell_size, 
                                                                        y, y*st->cell_size,
                                                                        z, z*st->cell_size, 
                                                                        bin_idx, n_pts );
        }
        for( int i = 0; i < n_pts; ++i )
        {
          printf("  %f %f %f\n", binned_data[bin_idx][1+3*i+0],
                                 binned_data[bin_idx][1+3*i+1],
                                 binned_data[bin_idx][1+3*i+2] );
        }
      }
    }
  }


  for( int i = 0 ; i < n_bins; ++i ) { free( binned_data[i] ); }
  free(binned_data);
}

void
msh_st_term( msh_st_t* st )
{
  st->width = -1;
  st->height = -1;
  st->depth = -1;
  st->cell_size = 0.0f;
  free( st->data );
}

int main( int argc, char** argv )
{
  printf("This program is an example of a spatial hashtable use\n");

  float cell_size = 0.1f;
  float n_pts = 10000;
  float* pts = (float*)malloc( n_pts * 3 * sizeof(float) );
  for( int i = 0 ; i < n_pts; ++i )
  {
    pts[3*i+0] = (rand() % 10000) / 10000.0f;
    pts[3*i+1] = (rand() % 10000) / 10000.0f;
    pts[3*i+2] = (rand() % 10000) / 10000.0f;
  }


  msh_st_t spatial_table;
  msh_st_init( &spatial_table, pts, n_pts, cell_size );
  msh_st_term( &spatial_table );

  return 0;
}