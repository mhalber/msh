
MunitResult
test_msh_array_api(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;

  msh_array( int32_t ) arr = {0};
  munit_assert_size( msh_array_len(arr), ==, 0 );
  munit_assert_size( msh_array_cap(arr), ==, 0 );
  msh_array_fit( arr, 32 );
  munit_assert_size( msh_array_len(arr), ==, 0 );
  munit_assert_size( msh_array_cap(arr), ==, 32 );

  munit_assert_size( msh_array_len(arr), ==, 0 );
  munit_assert_size( msh_array_cap(arr), ==, 32 );
  munit_assert_size( msh_array_sizeof(arr), ==, 0 );
  munit_assert_int8( msh_array_isempty(arr), ==, true );

  msh_array_push( arr, 10 );
  msh_array_push( arr, 20 );
  msh_array_push( arr, 30 );
  munit_assert_size( msh_array_len(arr), ==, 3 );
  munit_assert_size( msh_array_cap(arr), ==, 32 );

  msh_array_pop( arr );
  munit_assert_size( msh_array_len(arr), ==, 2 );
  munit_assert_size( msh_array_cap(arr), ==, 32 );

  int32_t* front = msh_array_front( arr );
  int32_t* back  = msh_array_back( arr );
  munit_assert_int32( *front, ==, 10 );
  munit_assert_int32( *back, ==, 20 );

  int32_t* end = msh_array_end(arr);
  munit_assert_int32( *end, ==, 30 );

  msh_array_clear( arr );
  munit_assert_size( msh_array_len(arr), ==, 0 );
  munit_assert_size( msh_array_cap(arr), ==, 32 );

  for( int32_t i = 0; i < 33; ++i )
  {
    msh_array_push( arr, i*10 );
  }
  munit_assert_size( msh_array_sizeof(arr), ==, 33 * sizeof(int32_t) );
  munit_assert_size( msh_array_len(arr), ==, 33 );
  munit_assert_size( msh_array_cap(arr), ==, msh_array__grow_formula(32) );

  for( int32_t i = 0; i < 33; ++i )
  {
    munit_assert_int32( arr[i], ==, i*10 );
  }

  msh_array_free( arr );
  munit_assert_size( msh_array_len(arr), ==, 0 );
  munit_assert_size( msh_array_cap(arr), ==, 0 );

  return MUNIT_OK;
}

MunitResult
test_msh_array_printf(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;
  
  msh_array( char ) str = {0};
  msh_array_printf( str, "This" );
  munit_assert_string_equal( str, "This" );
  msh_array_printf( str, " is a ");
  munit_assert_string_equal( str, "This is a " );
  msh_array_printf( str, "string-builder. ");
  munit_assert_string_equal( str, "This is a string-builder. " );
  msh_array_printf( str, "It supports formatting: %03d %2.1f", 12, 0.5 );
  munit_assert_string_equal( str, "This is a string-builder. It supports formatting: 012 0.5" );

  msh_array_free(str);
  munit_assert_size( msh_array_len(str), ==, 0 );
  munit_assert_size( msh_array_cap(str), ==, 0 );

  return MUNIT_OK;
}

MunitResult
test_msh_array_copy(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;

  msh_array( int16_t ) src = {0};
  msh_array( int16_t ) dst = {0};
  for( int16_t i = 0; i < 100; ++i )
  {
    msh_array_push( src, i );
  }
  msh_array_copy( dst, src, 20 );
  munit_assert_size( msh_array_len(src), ==, 100 );
  munit_assert_size( msh_array_len(dst), ==, 20 );

  for( size_t i = 0; i < msh_array_len(dst); ++i )
  {
    munit_assert_int32( src[i], ==, dst[i] );
  }

  msh_array_free( src );
  msh_array_free( dst );
  munit_assert_size( msh_array_len(src), ==, 0 );
  munit_assert_size( msh_array_len(dst), ==, 0 );

  return MUNIT_OK;
}

MunitResult
test_msh_map_api(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;
  
  msh_map_t map = {0};
  
  msh_map_init( &map, 255 );
  munit_assert_size( msh_map_len(&map), ==, 0 );
  munit_assert_size( msh_map_cap(&map), ==, 256 );

  msh_map_insert( &map, 0,  9 );
  msh_map_insert( &map, 0, 10 );
  msh_map_insert( &map, 1, 11 );
  munit_assert_size( msh_map_len( &map ), ==, 2 );
  msh_map_insert( &map, 2, 12 );
  msh_map_insert( &map, 3, 13 );
  msh_map_insert( &map, 3, 14 );
  munit_assert_size( msh_map_len(&map), ==, 4 );
  
  uint64_t* val = NULL;
  val = msh_map_get( &map, 0 );
  munit_assert_ptr_not_equal( val, NULL );
  munit_assert_uint64( *val, ==, 10 );
  
  val = msh_map_get( &map, 1 );
  munit_assert_ptr_not_equal( val, NULL );
  munit_assert_uint64( *val, ==, 11 );
  
  val = msh_map_get( &map, 2 );
  munit_assert_ptr_not_equal( val, NULL );
  munit_assert_uint64( *val, ==, 12 );

  val = msh_map_get( &map, 3 );
  munit_assert_ptr_not_equal( val, NULL );
  munit_assert_uint64( *val, ==, 14 );

  val = msh_map_get( &map, 5 );
  munit_assert_ptr_equal( val, NULL );

  msh_map_remove( &map, 0 );
  munit_assert_size( msh_map_len( &map ), ==, 3 );
  
  val = msh_map_get( &map, 0 );
  munit_assert_ptr_equal( val, NULL );

  msh_map_remove( &map, 7 );
  munit_assert_size( msh_map_len( &map ), ==, 3 );

  msh_map_insert( &map, 0, 9 );
  val = msh_map_get( &map, 0 );
  munit_assert_ptr_not_equal( val, NULL );
  munit_assert_uint64( *val, ==, 9 );
  munit_assert_size( msh_map_len(&map), ==, 4 );
  
  msh_map_free( &map );
  munit_assert_size( msh_map_len( &map ), ==, 0 );
  munit_assert_size( msh_map_cap( &map ), ==, 0 );

  return MUNIT_OK;
}

MunitResult
test_msh_map_iteration(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;
  
  msh_map_t map = {0};
  
  msh_map_init( &map, 255 );
  munit_assert_size( msh_map_len(&map), ==, 0 );
  munit_assert_size( msh_map_cap(&map), ==, 256 );

  for( int32_t i = 0; i < 10; ++i )
  {
    msh_map_insert( &map, i,  i*10 + i);
  }
 
  uint64_t* keys = NULL;
  uint64_t* vals = NULL;
  msh_map_get_iterable_keys_and_vals( &map, &keys, &vals );
  munit_assert_ptr_not_equal( keys, NULL );
  munit_assert_ptr_not_equal( vals, NULL );
  for( size_t i = 0; i < msh_map_len( &map ); ++i )
  {
    if( keys[i] == i ) 
    { 
      munit_assert_uint64( vals[i], ==, keys[i]*10+keys[i] ); 
    }
  }
  free( keys );
  free( vals );
  keys = NULL; vals = NULL;

  msh_map_get_iterable_keys( &map, &keys );
  munit_assert_ptr_not_equal( keys, NULL );
  for( size_t i = 0; i < msh_map_len( &map ); ++i )
  {
    uint64_t* val = msh_map_get( &map, keys[i] );
    munit_assert_ptr_not_equal( val, NULL);
    munit_assert_uint64( *val, ==, keys[i]*10 + keys[i] );
  }
  free(keys);
  keys = NULL;

  msh_map_get_iterable_keys( &map, &vals );
  munit_assert_ptr_not_equal( vals, NULL );
  for( size_t i = 0; i < msh_map_len( &map ); ++i )
  {
    uint64_t* val = msh_map_get( &map, i );
    munit_assert_ptr_not_equal( val, NULL);
    munit_assert_uint64( *val, ==, i*10 + i );
  }
  free( vals );
  return MUNIT_OK;
}


MunitResult
test_msh_map_str_hash(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;

  uint64_t h1 = msh_hash_str( "Dog" );
  uint64_t h2 = msh_hash_str( "Dog" );
  uint64_t h3 = msh_hash_str( "dog" );

  munit_assert_uint64( h1, ==, h2 );
  munit_assert_uint64( h2, !=, h3 );
  munit_assert_uint64( h1, !=, h3 );
  
  return MUNIT_OK;
}

MunitResult
test_msh_heap_api(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;

  msh_array( int64_t ) heap = {0};

  msh_array_push( heap, 1 );
  msh_array_push( heap, 8 );
  msh_array_push( heap, 10 );
  msh_array_push( heap, 5 );
  msh_array_push( heap, 3 );
  
  munit_assert_false( msh_heap_isvalid_i64(heap, (int32_t)msh_array_len(heap)) );
  msh_heap_make_i64( heap, msh_array_len(heap));
  munit_assert_true( msh_heap_isvalid_i64(heap, (int32_t)msh_array_len(heap)) );
  
  msh_array_push( heap, 18 );
  msh_heap_push_i64( heap, msh_array_len(heap) );
  munit_assert_int64( heap[0], ==, 18 );
  munit_assert_true( msh_heap_isvalid_i64(heap, (int32_t)msh_array_len(heap)) );
  
  msh_array_push( heap, 17 );
  msh_heap_push_i64( heap, msh_array_len(heap) );
  
  munit_assert_int64( heap[0], ==, 18 );
  munit_assert_true( msh_heap_isvalid_i64(heap, (int32_t)msh_array_len(heap)) );

  msh_heap_pop_i64( heap, msh_array_len(heap) );
  int64_t max = *msh_array_back(heap);
  msh_array_pop(heap);
  
  munit_assert_int64( heap[0], ==, 17 );
  munit_assert_int64( max, ==, 18 );
  munit_assert_true( msh_heap_isvalid_i64(heap, (int32_t)msh_array_len(heap)) );

  return MUNIT_OK;
}

MunitResult
test_msh_disjoint_set_api(const MunitParameter params[], void* fixture) 
{
  (void) params;
  (void) fixture;

  static const int32_t map_size = 6;
  char map[] = "111000"
               "111000"
               "100010"
               "011000"
               "011001"
               "011010";
  static const uint64_t num_sets = 6;
  int32_t n_vals = map_size*map_size;
  msh_dset_t dset = {0};
  msh_dset_init( &dset, n_vals );
  
  int32_t first_zero_idx = 0;
  for( int32_t i = 0; i < map_size * map_size; ++i )
  {
    if( map[i] == '0' )
    {
      first_zero_idx = i;
      break;
    }
  }

  for( int32_t y = 0; y < map_size; ++y )
  {
    for( int32_t x = 0; x < map_size; ++x )
    {
      int32_t idx = y * map_size + x;
      char c = map[idx];
      if( c == '0' ) // background
      {
        msh_dset_union( &dset, first_zero_idx, idx );
      }
      else // islands
      {
        int32_t idx_u = msh_max( y - 1, 0 ) * map_size + x;
        int32_t idx_d = msh_min( y + 1, map_size - 1 ) * map_size + x;
        int32_t idx_l = y * map_size + msh_max( x - 1, 0 );
        int32_t idx_r = y * map_size + msh_min( x + 1, map_size - 1 );
        if( c == map[idx_u] ) { msh_dset_union( &dset, idx_u, idx ); }
        if( c == map[idx_d] ) { msh_dset_union( &dset, idx_d, idx ); }
        if( c == map[idx_l] ) { msh_dset_union( &dset, idx_l, idx ); }
        if( c == map[idx_r] ) { msh_dset_union( &dset, idx_r, idx ); }
      }
    }
  }

  munit_assert_uint64( dset.num_sets, ==, num_sets );
  munit_assert_uint64( msh_dset_find( &dset, 1 ), ==, msh_dset_find( &dset,  6 ) );
  munit_assert_uint64( msh_dset_find( &dset, 5 ), ==, msh_dset_find( &dset, 11 ) );

  msh_dset_term(&dset);
  munit_assert_uint64( dset.num_sets, ==, 0 );
  munit_assert_ptr_null( dset.elems );

  return MUNIT_OK;
}