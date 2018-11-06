/* Poor man's tests for various parts of msh_std.h */

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#include "msh/msh_std.h"

void
msh_array_test()
{

}

void 
msh_heap_test()
{
  msh_array( real32_t ) heap = {0};

  msh_array_push( heap, 1 );
  msh_array_push( heap, 8 );
  msh_array_push( heap, 10 );
  msh_array_push( heap, 5 );
  msh_array_push( heap, 3 );

  assert( !msh_heap_isvalid( heap, msh_array_len(heap) ) );

  msh_heap_make( heap, msh_array_len(heap) );

  assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );

  msh_array_push( heap, 18 );
  msh_heap_push( heap, msh_array_len(heap) );

  assert( heap[0] == 18 );
  assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );

  msh_array_push( heap, 17 );
  msh_heap_push( heap, msh_array_len(heap) );

  assert( heap[0] == 18 );
  assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );

  msh_heap_pop( heap, msh_array_len( heap ) );
  real32_t max = *msh_array_back( heap );
  msh_array_pop( heap );

  assert( heap[0] == 17 );
  assert( max == 18 );
  assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );
}

void
msh_map_test()
{
  msh_map_t map_a = {0};
  msh_map_t map_b = {0};
  
  // Initialization
  msh_map_init( &map_a, 1023 );
  assert( msh_map_len(&map_a) == 0 );
  assert( msh_map_cap(&map_a) == 1024 );

  // Insertion
  msh_map_insert( &map_b, 0,  9 );
  msh_map_insert( &map_b, 0, 10 );
  msh_map_insert( &map_b, 1, 11 );
  assert( msh_map_len(&map_b) == 2 );
  msh_map_insert( &map_b, 2, 12 );
  msh_map_insert( &map_b, 3, 13 );
  assert( msh_map_len(&map_b) == 4 );
  msh_map_insert( &map_b, 4, 14 );
  assert( msh_map_len(&map_b) == 5 );

  // Finding elements
  uint64_t* val = msh_map_get( &map_b, 0 );
  assert( val != NULL );
  assert( *val == 10 );
  val = msh_map_get( &map_b, 1 );
  assert( val != NULL );
  assert( *val == 11 );
  val = msh_map_get( &map_b, 4 );
  assert( val != NULL );
  assert( *val == 14 );
  val = msh_map_get( &map_b, 5 );
  assert( val == NULL );

  // Getting iterable elements
  uint64_t* keys = NULL;
  uint64_t* vals = NULL;
  msh_map_get_iterable_keys_and_vals( &map_b, &keys, &vals );
  assert(keys != NULL );
  assert(vals != NULL );
  for( size_t i = 0; i < msh_map_len( &map_b ); ++i )
  {
    if( keys[i] == 0 ) { assert( vals[i] == 10 ); }
    if( keys[i] == 1 ) { assert( vals[i] == 11 ); }
    if( keys[i] == 2 ) { assert( vals[i] == 12 ); }
    if( keys[i] == 3 ) { assert( vals[i] == 13 ); }
    if( keys[i] == 4 ) { assert( vals[i] == 14 ); }
  }
  free( keys );
  free( vals );

  // Freeing
  msh_map_free( &map_a );
  msh_map_free( &map_b );
  assert( msh_map_len(&map_a) == 0 && msh_map_cap(&map_a) == 0 );
  assert( msh_map_len(&map_b) == 0 && msh_map_cap(&map_b) == 0 );
}

void run_tests()
{
  printf( "Running msh_std.h tests!\n" );

  printf( "| Testing msh_array\n" );
  msh_array_test();
  printf( "|    -> Passed!\n" );

  printf( "| Testing msh_map\n" );
  msh_map_test();
  printf( "|    -> Passed!\n" );

  printf( "| Testing msh_heap\n" );
  msh_heap_test();
  printf( "|    -> Passed!\n" );
}


int
main( int argc, char** argv )
{
  run_tests();

  // uint64_t t2, t1;
  // size_t n = 10000000;
  // real32_t* dists_a = (real32_t*)malloc( (n+1) * sizeof(real32_t) );
  // real32_t* dists_b = (real32_t*)malloc( (n+1) * sizeof(real32_t) );
  // msh_rand_ctx_t rand_gen;
  // msh_rand_init( &rand_gen, 12346ULL );
  // for( size_t i = 0; i < n; ++i )
  // {
  //   dists_a[i] = msh_rand_range( &rand_gen, 1.0, msh_max( n, 10 ) );
  //   dists_b[i] = dists_a[i];
  // }

  // for( size_t i = 0; i < msh_min(n, 10); ++i ) { printf("%6.1f ", dists_a[i]); }
  // printf("\n");
  // t1 = msh_time_now();
  // std::make_heap( dists_a, dists_a + n);
  // t2 = msh_time_now();
  // for( size_t i = 0; i < msh_min(n, 10); ++i ) { printf("%6.1f ", dists_a[i]); }
  // printf("\n std::make_heap completed in %fms.\n", msh_time_diff_ms( t2, t1) );
  
  // t1 = msh_time_now();
  // msh_heap_make( dists_b, n );
  // t2 = msh_time_now();
  // for( size_t i = 0; i < msh_min(n, 10); ++i ) { printf("%6.1f ", dists_b[i]); }
  // printf("\n msh_heap_make completed in %fms.\n", msh_time_diff_ms( t2, t1) );

  // // for( size_t i = 0; i < n ; ++i ) { msh_cprintf(dists_a[i] != dists_b[i], "Dissagreement at %d :%f %f\n", i, dists_a[i], dists_b[i]); }

  // msh_cprintf( std::is_heap(dists_a, dists_a + n), "A is heap!\n" );
  // msh_cprintf( std::is_heap(dists_b, dists_b + n), "B is heap!\n" );
  return 1;
}