#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_CONTAINERS_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_containers.h"

#include "munit/munit.h"

#include "msh_containers_test.inl"
#include "msh_stats_test.inl"


int main(int argc, char* const argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
  MunitTest array_tests[] = 
  {
    { .name = (char*) "/api",    .test = test_msh_array_api, },
    { .name = (char*) "/copy",   .test = test_msh_array_copy, },
    { .name = (char*) "/printf", .test = test_msh_array_printf, },
    { 0 }
  };

  MunitSuite array_test_suite = 
  {
    .prefix = (char*) "/array",
    .tests = array_tests,
    .suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE
  };

  MunitTest heap_tests[] = 
  {
    { .name = (char*) "/api",    .test = test_msh_heap_api, },
    { 0 }
  };

  MunitSuite heap_test_suite = 
  {
    .prefix = (char*) "/heap",
    .tests = heap_tests,
    .suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE
  };

  MunitTest map_tests[] = 
  {
    { .name = (char*) "/api",       .test = test_msh_map_api, },
    { .name = (char*) "/iteration", .test = test_msh_map_iteration, },
    { .name = (char*) "/str_hash",  .test = test_msh_map_str_hash, },
    { 0 }
  };

  MunitSuite map_test_suite = 
  {
    .prefix = (char*) "/map",
    .tests = map_tests,
    .suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE
  };

  MunitTest stats_tests[] = 
  {
    { .name = (char*) "/pdf_sampling", .test = test_msh_discrete_distrib_sampling, },
    { 0 }
  };

  MunitSuite stats_test_suite = 
  {
    .prefix = (char*) "/stats",
    .tests = stats_tests,
    .suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE
  };

  MunitSuite test_suites_array[] = { array_test_suite, heap_test_suite, map_test_suite, stats_test_suite, {0} };

  const MunitSuite test_suites = 
  {
    .prefix = (char*) "msh_std",
    .tests = NULL,
    .suites = test_suites_array,
    .iterations = 0,
    .options = MUNIT_SUITE_OPTION_NONE
  };

  return munit_suite_main( &test_suites, NULL, argc, argv );
}




// /* Poor man's tests for various parts of msh_std.h */

// #define MSH_STD_INCLUDE_LIBC_HEADERS
// #define MSH_STD_IMPLEMENTATION
// #include "msh/msh_std.h"

// #define MSH_TEST_DEBUG_PRINT 0
// #define MSH_TEST_TIME_ACCURACY_MS 2.5

// void 
// msh_time_test()
// {
//   uint64_t t1, t2;
//   t1 = msh_time_now();
//   msh_sleep( 100 );
//   t2 = msh_time_now();

//   assert( fabs( msh_time_diff_ms( t2, t1 ) - 100 ) < MSH_TEST_TIME_ACCURACY_MS );

//   t1 = msh_time_now();
//   msh_sleep( 200 );
//   t2 = msh_time_now();

//   assert( fabs( msh_time_diff_ms( t2, t1 ) - 200 ) < MSH_TEST_TIME_ACCURACY_MS );
// }

// void
// msh_dset_test()
// {
//   int32_t map_size = 6;
//   char map[] = "111000"
//                "111000"
//                "100010"
//                "011000"
//                "011001"
//                "011010";
//   uint32_t num_sets = 6;
//   int32_t n_vals = map_size*map_size;
//   msh_dset_t dset = {0};
//   msh_dset_init( &dset, n_vals );
  
//   int32_t first_zero_idx = 0;
//   for( int32_t i = 0; i < map_size*map_size; ++i )
//   {
//     if( map[i] == '0' )
//     {
//       first_zero_idx = i;
//       break;
//     }
//   }

//   for( int32_t y = 0; y < map_size; ++y )
//   {
//     for( int32_t x = 0; x < map_size; ++x )
//     {
//       int32_t idx = y * map_size + x;
//       char c = map[idx];
//       if( c == '0' ) // background
//       {
//         msh_dset_union( &dset, first_zero_idx, idx );
//       }
//       else // islands
//       {
//         int32_t idx_u = msh_max( y - 1, 0 ) * map_size + x;
//         int32_t idx_d = msh_min( y + 1, map_size - 1 ) * map_size + x;
//         int32_t idx_l = y * map_size + msh_max( x - 1, 0 );
//         int32_t idx_r = y * map_size + msh_min( x + 1, map_size - 1 );
//         if( c == map[idx_u] ) { msh_dset_union( &dset, idx_u, idx ); }
//         if( c == map[idx_d] ) { msh_dset_union( &dset, idx_d, idx ); }
//         if( c == map[idx_l] ) { msh_dset_union( &dset, idx_l, idx ); }
//         if( c == map[idx_r] ) { msh_dset_union( &dset, idx_r, idx ); }
//       }
//     }
//   }

//   assert( dset.num_sets == num_sets );
//   assert( msh_dset_find( &dset, 1 ) == msh_dset_find( &dset,  6 ) );
//   assert( msh_dset_find( &dset, 5 ) == msh_dset_find( &dset, 11 ) );

// #if MSH_TEST_DEBUG_PRINT
//   printf("Map:\n");
//   for( int32_t y = 0; y < map_size; ++y )
//   {
//     printf("  | ");
//     for( int32_t x = 0; x < map_size; ++x )
//     {
//       int idx = y * map_size + x;
//       int32_t p = msh_dset_find( &dset, idx );
//       int32_t s = dset.elems[p].size;
//       printf("%2d-%-2d | ", p, s );
//     }
//     printf("\n");
//   }
//   printf( "Found number of sets: %d | Target number of sets: %d\n", dset.num_sets, num_sets );
// #endif
//   msh_dset_term(&dset);
//   assert( dset.num_sets == 0 );
//   assert( dset.elems == NULL );
// }

// void 
// msh_block_timers_test()
// {

//   // MSH_BEGIN_TIMED_BLOCK( 0 );

//   // MSH_BEGIN_TIMED_BLOCK( 1 );
//   // msh_sleep(1);
//   // MSH_END_TIMED_BLOCK( 1 );

//   // MSH_BEGIN_TIMED_BLOCK( 3 );
//   // MSH_BEGIN_TIMED_BLOCK( 4 );
//   // msh_sleep(1);
//   // MSH_END_TIMED_BLOCK( 4 );
//   // MSH_BEGIN_TIMED_BLOCK( 5 );
//   // msh_sleep(1);
//   // MSH_END_TIMED_BLOCK( 5 );
//   // MSH_END_TIMED_BLOCK( 3 );


//   // MSH_BEGIN_TIMED_BLOCK( 2 );
//   // msh_sleep(1);
//   // MSH_END_TIMED_BLOCK( 2 );

//   // MSH_END_TIMED_BLOCK( 0 );

//   // msh_debug_report_debug_events();
// }

// typedef struct test_struct
// {
//   int32_t a;
//   float b;
//   double c;
//   char* p;
// } test_struct_t;

// void
// msh_array_test()
// {
//   msh_array(int32_t) arr1 = {0};
//   assert( msh_array_isempty( arr1 ) );
//   msh_array_fit( arr1, 32 );
//   assert( msh_array_cap( arr1 ) == 32 );
//   assert( msh_array_len( arr1 ) == 0 );
//   for( size_t i = 0; i < 32; ++i )
//   {
//     msh_array_push( arr1, (int32_t) i );
//   }
//   assert( msh_array_len( arr1 ) == 32 );
//   assert( *msh_array_front( arr1 ) == 0 );
//   assert( *msh_array_back( arr1 ) == 31 );
//   int32_t counter = 0;
//   for( int32_t* iter = msh_array_front( arr1 ); iter < msh_array_end( arr1 ); ++iter )
//   {
//     assert( *iter == counter );
//     counter++;
//   }
//   msh_array_clear( arr1 );
//   assert( msh_array_len( arr1 ) == 0 );
//   assert( msh_array_isempty( arr1 ) );
//   int32_t big_size = 2<<20;
//   for( int32_t i = 0; i < big_size; ++i )
//   {
//     msh_array_push( arr1, i );
//   }
//   assert( msh_array_len( arr1 ) == (uint32_t)big_size );
//   assert( *msh_array_back( arr1 ) == big_size - 1 );
//   msh_array_free( arr1 );
//   assert( msh_array_len( arr1 ) == 0 );
//   assert( msh_array_cap( arr1 ) == 0 );
//   assert( msh_array_isempty( arr1 ) );


//   msh_array(test_struct_t) arr2 = {0};
//   assert( msh_array_isempty( arr2 ) );
//   msh_array_fit( arr2, 32 );
//   assert( msh_array_cap( arr2 ) == 32 );
//   assert( msh_array_len( arr2 ) == 0 );
//   for( size_t i = 0; i < 32; ++i )
//   {
//     test_struct_t s;
//     s.a = (int32_t)i;
//     s.b = (float)i;
//     s.c = (double)i;
//     s.p = 0x0;
//     msh_array_push( arr2, s );
//   }
//   assert( msh_array_len( arr2 ) == 32 );
//   assert( msh_array_front( arr2 )->a == 0 );
//   assert( msh_array_back( arr2 )->a == 31 );
//    counter = 0;
//   for( test_struct_t* iter = msh_array_front( arr2 ); iter < msh_array_end( arr2 ); ++iter )
//   { 
//     assert( iter->a == counter );
//     counter++;
//   }
//   msh_array_clear( arr2 );
//   assert( msh_array_len( arr2 ) == 0 );
//   assert( msh_array_isempty( arr2 ) );
//   for( int32_t i = 0; i < big_size; ++i )
//   {
//     test_struct_t s;
//     s.a = (int32_t)i;
//     s.b = (float)i;
//     s.c = (double)i;
//     s.p = 0x0;
//     msh_array_push( arr2, s );
//   }
//   assert( msh_array_len( arr2 ) == (uint32_t)big_size );
//   assert( msh_array_back( arr2 )->a == big_size - 1 );
//   msh_array_free( arr2 );
//   assert( msh_array_len( arr2 ) == 0 );
//   assert( msh_array_cap( arr2 ) == 0 );
//   assert( msh_array_isempty( arr2 ) );
// }

// void
// msh_strcpy_test()
// {
//   int n;
//   char* test = "Hello, world!";
//   char dst[1024];
  
//   n = msh_strncpy( dst, test, 7 );
//   printf("%s\n", dst );
//   n = msh_strncpy( dst + n, test, strlen(test) );
//   printf("%s\n %d\n", dst, n );
//   memset( dst, 0, 1024 );
//   n = msh_strcpy_range( dst, test, 0, 7 );
//   printf("%s\n", dst );
//   n = msh_strcpy_range( dst, test, n, strlen(test) );
//   printf("%s\n %d\n", dst, n );
//   n = msh_strcpy_range( dst, " Word is up!", n, 1024 );
//   printf("%s\n %d\n", dst, n );
// }


// void 
// msh_heap_test()
// {
//   msh_array( real32_t ) heap = {0};

//   msh_array_push( heap, 1 );
//   msh_array_push( heap, 8 );
//   msh_array_push( heap, 10 );
//   msh_array_push( heap, 5 );
//   msh_array_push( heap, 3 );

//   assert( !msh_heap_isvalid( heap, msh_array_len(heap) ) );

//   msh_heap_make( heap, msh_array_len(heap) );

//   assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );

//   msh_array_push( heap, 18 );
//   msh_heap_push( heap, msh_array_len(heap) );

//   assert( heap[0] == 18 );
//   assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );

//   msh_array_push( heap, 17 );
//   msh_heap_push( heap, msh_array_len(heap) );

//   assert( heap[0] == 18 );
//   assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );

//   msh_heap_pop( heap, msh_array_len( heap ) );
//   real32_t max = *msh_array_back( heap );
//   msh_array_pop( heap );

//   assert( heap[0] == 17 );
//   assert( max == 18 );
//   assert( msh_heap_isvalid( heap, msh_array_len(heap) ) );
// }

// void
// msh_map_test()
// {
//   msh_map_t map_a = {0};
//   msh_map_t map_b = {0};
  
//   // Initialization
//   msh_map_init( &map_a, 1023 );
//   assert( msh_map_len(&map_a) == 0 );
//   assert( msh_map_cap(&map_a) == 1024 );

//   // Insertion
//   msh_map_insert( &map_b, 0,  9 );
//   msh_map_insert( &map_b, 0, 10 );
//   msh_map_insert( &map_b, 1, 11 );
//   assert( msh_map_len(&map_b) == 2 );
//   msh_map_insert( &map_b, 2, 12 );
//   msh_map_insert( &map_b, 3, 13 );
//   assert( msh_map_len(&map_b) == 4 );
//   msh_map_insert( &map_b, 4, 14 );
//   assert( msh_map_len(&map_b) == 5 );

//   // Finding elements
//   uint64_t* val = msh_map_get( &map_b, 0 );
//   assert( val != NULL );
//   assert( *val == 10 );
//   val = msh_map_get( &map_b, 1 );
//   assert( val != NULL );
//   assert( *val == 11 );
//   val = msh_map_get( &map_b, 4 );
//   assert( val != NULL );
//   assert( *val == 14 );
//   val = msh_map_get( &map_b, 5 );
//   assert( val == NULL );

//   // Getting iterable elements
//   uint64_t* keys = NULL;
//   uint64_t* vals = NULL;
//   msh_map_get_iterable_keys_and_vals( &map_b, &keys, &vals );
//   assert(keys != NULL );
//   assert(vals != NULL );
//   for( size_t i = 0; i < msh_map_len( &map_b ); ++i )
//   {
//     if( keys[i] == 0 ) { assert( vals[i] == 10 ); }
//     if( keys[i] == 1 ) { assert( vals[i] == 11 ); }
//     if( keys[i] == 2 ) { assert( vals[i] == 12 ); }
//     if( keys[i] == 3 ) { assert( vals[i] == 13 ); }
//     if( keys[i] == 4 ) { assert( vals[i] == 14 ); }
//   }
//   free( keys );
//   free( vals );

//   // Freeing
//   msh_map_free( &map_a );
//   msh_map_free( &map_b );
//   assert( msh_map_len(&map_a) == 0 && msh_map_cap(&map_a) == 0 );
//   assert( msh_map_len(&map_b) == 0 && msh_map_cap(&map_b) == 0 );
// }

// void run_tests()
// {
//   printf( "Running msh_std.h tests!\n" );

//   printf( "| Testing msh_time\n" );
//   msh_time_test();
//   printf( "|    -> Passed!\n" );

//   // printf( "| Testing msh block timers\n" );
//   // msh_block_timers_test();
//   // printf( "|    -> Passed!\n" );

//   printf( "| Testing msh_array\n" );
//   msh_array_test();
//   printf( "|    -> Passed!\n" );

//   printf( "| Testing msh_map\n" );
//   msh_map_test();
//   printf( "|    -> Passed!\n" );

//   printf( "| Testing msh_heap\n" );
//   msh_heap_test();
//   printf( "|    -> Passed!\n" );

//   printf( "| Testing msh_dset\n" );
//   msh_dset_test();
//   printf( "|    -> Passed!\n" );
// }


// int
// main( void )
// {
//   run_tests();
//   return 1;
// }