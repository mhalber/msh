/* Poor man's tests for various parts of msh_std.h */
#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#define MSH_HASH_GRID_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_vec_math.h"
#include "msh/msh_hash_grid.h"

msh_vec3_t
generate_random_point_within_sphere_shell( msh_rand_ctx_t* rand_gen, msh_vec3_t center,
                                           real32_t radius_a, real32_t radius_b )
{
  assert( radius_a > radius_b );
  assert( radius_a >= 0.0f );
  assert( radius_b >= 0.0f );

  real32_t x = 2.0f * msh_rand_nextf( rand_gen ) - 1.0f;
  real32_t y = 2.0f * msh_rand_nextf( rand_gen ) - 1.0f;
  real32_t z = 2.0f * msh_rand_nextf( rand_gen ) - 1.0f;
  real32_t s = msh_rand_nextf( rand_gen ) * (radius_a - radius_b) + radius_b ;
  msh_vec3_t pt = msh_vec3( x, y, z );
  pt = msh_vec3_scalar_mul( msh_vec3_normalize( pt ), s );
  pt = msh_vec3_add( pt, center );
  return pt;
}

msh_vec3_t
generate_random_point_within_sphere( msh_rand_ctx_t* rand_gen, msh_vec3_t center, real32_t radius )
{
  return generate_random_point_within_sphere_shell( rand_gen, center, radius, 0.0f );
}

void
knn_search_test()
{
  msh_rand_ctx_t rand_gen = {0};
  msh_rand_init( &rand_gen, 12346ULL );
  msh_array( msh_vec3_t ) pts = {0};

  // generate knn pts around origin 
  size_t knn = 10;
  real32_t radius_a = 0.3;
  for( size_t i = 0; i < knn; ++i )
  {
    msh_vec3_t pt = generate_random_point_within_sphere( &rand_gen, msh_vec3_zeros(), radius_a );
    msh_array_push( pts, pt );
  }

  // generate 1000 points around in a shell
  real32_t radius_b = 0.6;
  real32_t radius_c = 0.4;
  for( size_t i = 0; i < 1000; ++i )
  {
    msh_vec3_t pt = generate_random_point_within_sphere_shell( &rand_gen, msh_vec3_zeros(), 
                                                                radius_b, radius_c );
    msh_array_push( pts, pt );
  }

  // setup the hash grid
  msh_hash_grid_t hg = {0};
  msh_hash_grid_init_3d( &hg, (real32_t*)&pts[0], msh_array_len(pts), 0.1 );

  msh_hash_grid_search_desc_t search_opts = 
  {
    .n_query_pts = 1,
    .k = knn,
    .distances_sq = malloc( sizeof(real32_t) * knn ),
    .indices = malloc( sizeof(int32_t) * knn ),
  };

  // check for points produced around query
  msh_vec3_t query = msh_vec3_zeros();
  search_opts.query_pts = (float*)&query;
  size_t n_neigh = msh_hash_grid_knn_search( &hg, &search_opts );
  assert( n_neigh == knn );
  for( size_t i = 0; i < n_neigh; ++i )
  {
    assert( search_opts.indices[i] < (int32_t)knn );
  }
}

void
radius_search_test()
{
  msh_rand_ctx_t rand_gen = {0};
  msh_rand_init( &rand_gen, 12346ULL );
  msh_array( msh_vec3_t ) pts = {0};
  
  // randomly generate 10 points in a volume of a sphere with 0.1 radius around origin
  size_t n_pts_a = 10;
  real32_t radius_a = 0.1;
  for( size_t i = 0; i < n_pts_a; ++i )
  {
    msh_vec3_t pt = generate_random_point_within_sphere( &rand_gen, msh_vec3_zeros(), radius_a );
    msh_array_push( pts, pt );
  }

  // randomly generate 100 points in a volume of a sphere with 0.3 radius around pt. 3.0, 3.0, 3.0
  size_t n_pts_b = 100;
  real32_t radius_b = 0.3;
  for( size_t i = 0; i < n_pts_b; ++i )
  {
    msh_vec3_t pt = generate_random_point_within_sphere( &rand_gen, msh_vec3(3.0f, 3.0f, 3.0f), 
                                                                    radius_b );
    msh_array_push( pts, pt );
  }


  // now randomly generate 1000 points in a volume that is a difference of two spheres
  real32_t radius_c = 0.5;
  real32_t radius_d = 0.6;
  for( size_t i = 0; i < 1000; ++i )
  {
    msh_vec3_t pt = generate_random_point_within_sphere_shell( &rand_gen, msh_vec3_zeros(), 
                                                                radius_d, radius_c );
    msh_array_push( pts, pt );
  }



  // Move all points away from origin by the vector given by a query_a pt
  msh_vec3_t query_a = msh_vec3( msh_rand_nextf(&rand_gen),
                                 msh_rand_nextf(&rand_gen),
                                 msh_rand_nextf(&rand_gen) );
  msh_vec3_t query_b = msh_vec3_add( query_a, msh_vec3( 3.0f, 3.0f, 3.0f ) );
  for( size_t i = 0; i < msh_array_len(pts); ++i )
  {
    pts[i] = msh_vec3_add( query_a, pts[i] );
  }


  // setup the hash grid
  msh_hash_grid_t hg = {0};
  msh_hash_grid_init_3d( &hg, (real32_t*)&pts[0], msh_array_len(pts), radius_a );

  size_t max_n_neigh = msh_max( n_pts_a, n_pts_b);
  msh_hash_grid_search_desc_t search_opts = 
  {
    .n_query_pts = 1,
    .max_n_neigh = max_n_neigh,
    .distances_sq = malloc( sizeof(real32_t) * max_n_neigh ),
    .indices = malloc( sizeof(int32_t) * max_n_neigh ),
    .sort = 1
  };

  // check for points produced around query_a
  search_opts.query_pts = (float*)&query_a;
  search_opts.radius = radius_a;
  size_t n_neigh = msh_hash_grid_radius_search( &hg, &search_opts );
  assert( n_neigh == n_pts_a );
  for( size_t i = 0; i < n_pts_a; ++i )
  {
    assert( search_opts.indices[i] < (int32_t)n_pts_a );
  }


  // check for points produced around query_b
  search_opts.query_pts = (float*)&query_b;
  search_opts.radius = radius_b;
  n_neigh = msh_hash_grid_radius_search( &hg, &search_opts );
  assert( n_neigh == n_pts_b );
  for( size_t i = 0; i < n_pts_b; ++i )
  {
    assert( (search_opts.indices[i] >= (int32_t)n_pts_a) &&
            (search_opts.indices[i] <  (int32_t)(n_pts_a + n_pts_b)) );
  }
}

int
main()
{
  printf( "Running msh_hash_grid.h tests!\n" );

  printf( "| Testing msh_hash_grid_radius_search\n" );
  radius_search_test();
  printf( "|    -> Passed!\n" );

  printf( "| Testing msh_hash_grid_knn_search\n" );
  knn_search_test();
  printf( "|    -> Passed!\n" );

  return 1;
}