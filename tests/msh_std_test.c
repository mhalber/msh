/* Poor man's tests for various parts of msh_std.h */

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#include "msh/msh_std.h"

#define MSH_TEST_TIME_ACCURACY_MS 2.5

void 
msh_time_test()
{
  uint64_t t1, t2;
  t1 = msh_time_now();
  msh_sleep( 100 );
  t2 = msh_time_now();

  assert( fabs( msh_time_diff_ms( t2, t1 ) - 100 ) < MSH_TEST_TIME_ACCURACY_MS );

  t1 = msh_time_now();
  msh_sleep( 200 );
  t2 = msh_time_now();

  assert( fabs( msh_time_diff_ms( t2, t1 ) - 200 ) < MSH_TEST_TIME_ACCURACY_MS );
}

void test_funct_a()
{
}

void 
msh_block_timers_test()
{

  MSH_BEGIN_TIMED_BLOCK( 0 );

  MSH_BEGIN_TIMED_BLOCK( 1 );
  msh_sleep(1);
  MSH_END_TIMED_BLOCK( 1 );

  MSH_BEGIN_TIMED_BLOCK( 3 );
  MSH_BEGIN_TIMED_BLOCK( 4 );
  msh_sleep(1);
  MSH_END_TIMED_BLOCK( 4 );
  MSH_BEGIN_TIMED_BLOCK( 5 );
  msh_sleep(1);
  MSH_END_TIMED_BLOCK( 5 );
  MSH_END_TIMED_BLOCK( 3 );


  MSH_BEGIN_TIMED_BLOCK( 2 );
  msh_sleep(1);
  MSH_END_TIMED_BLOCK( 2 );

  MSH_END_TIMED_BLOCK( 0 );

  msh_debug_report_debug_events();
}

void 
msh_memleak_test()
{
  
}

void
msh_array_test()
{

}


void
msh_strcpy_test()
{
  int n;
  char* test = "Hello, world!";
  char dst[1024];
  
  n = msh_strncpy( dst, test, 7 );
  printf("%s\n", dst );
  n = msh_strncpy( dst + n, test, strlen(test) );
  printf("%s\n %d\n", dst, n );
  memset( dst, 0, 1024 );
  n = msh_strcpy_range( dst, test, 0, 7 );
  printf("%s\n", dst );
  n = msh_strcpy_range( dst, test, n, strlen(test) );
  printf("%s\n %d\n", dst, n );
  n = msh_strcpy_range( dst, " Word is up!", n, 1024 );
  printf("%s\n %d\n", dst, n );
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

  printf( "| Testing msh_time\n" );
  msh_time_test();
  printf( "|    -> Passed!\n" );

  printf( "| Testing msh block timers\n" );
  msh_block_timers_test();
  printf( "|    -> Passed!\n" );

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
main( void )
{
  run_tests();
  return 1;
}