
void
assert_equal_vec3i( msh_vec3i_t v, int a, int b, int c )
{
  munit_assert_int( v.x, ==, a );
  munit_assert_int( v.y, ==, b );
  munit_assert_int( v.z, ==, c );
}

void
assert_equal_vec3f( msh_vec3f_t v, float a, float b, float c )
{
  munit_assert_float( v.x, ==, a );
  munit_assert_float( v.y, ==, b );
  munit_assert_float( v.z, ==, c );
}

void
assert_equal_vec3d( msh_vec3d_t v, double a, double b, double c )
{
  munit_assert_double( v.x, ==, a );
  munit_assert_double( v.y, ==, b );
  munit_assert_double( v.z, ==, c );
}

MunitResult
test_msh_vector( const MunitParameter params[], void* fixture )
{
  (void) params;
  (void) fixture;

  msh_vec3i_t v01 = msh_vec3i_init( 1, 2, 3 );
  msh_vec3f_t v02 = msh_vec3f_init( 4.0, 5.0, 6.0 );
  msh_vec3d_t v03 = msh_vec3d_init( 7.0, 8.0, 9.0 );
  assert_equal_vec3i( v01, 1, 2, 3 );
  assert_equal_vec3f( v02, 4.0, 5.0, 6.0 );
  assert_equal_vec3d( v03, 7.0, 8.0, 9.0 );

  msh_vec3i_t v04 = msh_vec3i_init( 1, 2, 3 );
  msh_vec3i_t v05 = msh_vec3i_add( v01, v04 );
  assert_equal_vec3i( v05, 2, 4, 6 );

  msh_vec3i_t v06 = msh_vec3i_sub( v05, msh_vec3i_ones() );
  assert_equal_vec3i( v06, 1, 3, 5 );

  msh_vec3i_t v07 = msh_vec3i_mul( v06, msh_vec3i_init( 2, 4, 6 ) );
  assert_equal_vec3i( v07, 2, 12, 30 );

  msh_vec3i_t v08 = msh_vec3i_div( v07, msh_vec3i_init(2, 4, 6));
  assert_equal_vec3i( v08, 1, 3, 5 );

  int dot = msh_vec3i_dot( v08, v08 );
  munit_assert_int( dot, ==, 35 );

  msh_vec3i_t v09 = msh_scalar_vec3i_mul( 2, v06 );
  assert_equal_vec3i( v09, 2, 6, 10 );

  return MUNIT_OK;
}
