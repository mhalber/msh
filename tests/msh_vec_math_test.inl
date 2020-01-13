static inline void
assert_equal_vec2( msh_vec2_t v, msh_scalar_t a, msh_scalar_t b )
{
#ifndef MSH_VEC_MATH_DOUBLE_PRECISION
  munit_assert_float( v.x, ==, a );
  munit_assert_float( v.y, ==, b );
#else
  munit_assert_double( v.x, ==, a );
  munit_assert_double( v.y, ==, b );
#endif
}

static inline void
assert_equal_vec3( msh_vec3_t v, msh_scalar_t a, msh_scalar_t b, msh_scalar_t c )
{
#ifndef MSH_VEC_MATH_DOUBLE_PRECISION
  munit_assert_float( v.x, ==, a );
  munit_assert_float( v.y, ==, b );
  munit_assert_float( v.z, ==, c );
#else
  munit_assert_double( v.x, ==, a );
  munit_assert_double( v.y, ==, b );
  munit_assert_double( v.z, ==, c );
#endif
}

static inline void
assert_equal_vec4( msh_vec4_t v, msh_scalar_t a, msh_scalar_t b, msh_scalar_t c, msh_scalar_t d)
{
#ifndef MSH_VEC_MATH_DOUBLE_PRECISION
  munit_assert_float( v.x, ==, a );
  munit_assert_float( v.y, ==, b );
  munit_assert_float( v.z, ==, c );
  munit_assert_float( v.w, ==, d );
#else
  munit_assert_double( v.x, ==, a );
  munit_assert_double( v.y, ==, b );
  munit_assert_double( v.z, ==, c );
  munit_assert_double( v.w, ==, d );
#endif
}


MunitResult
test_msh_vec_math_vector_init( const MunitParameter params[], void* fixture )
{
  msh_vec2_t v2; msh_vec3_t v3; msh_vec4_t v4;
  v2 = msh_vec2_zeros();
  v3 = msh_vec3_zeros();
  v4 = msh_vec4_zeros();
  assert_equal_vec2( v2, 0.0, 0.0 );
  assert_equal_vec3( v3, 0.0, 0.0, 0.0 );
  assert_equal_vec4( v4, 0.0, 0.0, 0.0, 0.0 );

  v2 = msh_vec2_ones();
  v3 = msh_vec3_ones();
  v4 = msh_vec4_ones();
  assert_equal_vec2( v2, 1.0, 1.0 );
  assert_equal_vec3( v3, 1.0, 1.0, 1.0 );
  assert_equal_vec4( v4, 1.0, 1.0, 1.0, 1.0 );

  v2 = msh_vec2_posx();
  v3 = msh_vec3_posx();
  v4 = msh_vec4_posx();
  assert_equal_vec2( v2, 1.0, 0.0 );
  assert_equal_vec3( v3, 1.0, 0.0, 0.0 );
  assert_equal_vec4( v4, 1.0, 0.0, 0.0, 0.0 );

  v2 = msh_vec2_posy();
  v3 = msh_vec3_posy();
  v4 = msh_vec4_posy();
  assert_equal_vec2( v2, 0.0, 1.0 );
  assert_equal_vec3( v3, 0.0, 1.0, 0.0 );
  assert_equal_vec4( v4, 0.0, 1.0, 0.0, 0.0 );

  v3 = msh_vec3_posz();
  v4 = msh_vec4_posz();
  assert_equal_vec3( v3, 0.0, 0.0, 1.0 );
  assert_equal_vec4( v4, 0.0, 0.0, 1.0, 0.0 );

  v4 = msh_vec4_posw();
  assert_equal_vec4( v4, 0.0, 0.0, 0.0, 1.0 );

  v2 = msh_vec2_negx();
  v3 = msh_vec3_negx();
  v4 = msh_vec4_negx();
  assert_equal_vec2( v2, -1.0, 0.0 );
  assert_equal_vec3( v3, -1.0, 0.0, 0.0 );
  assert_equal_vec4( v4, -1.0, 0.0, 0.0, 0.0 );

  v2 = msh_vec2_negy();
  v3 = msh_vec3_negy();
  v4 = msh_vec4_negy();
  assert_equal_vec2( v2, 0.0, -1.0 );
  assert_equal_vec3( v3, 0.0, -1.0, 0.0 );
  assert_equal_vec4( v4, 0.0, -1.0, 0.0, 0.0 );

  v3 = msh_vec3_negz();
  v4 = msh_vec4_negz();
  assert_equal_vec3( v3, 0.0, 0.0, -1.0 );
  assert_equal_vec4( v4, 0.0, 0.0, -1.0, 0.0 );

  v4 = msh_vec4_negw();
  assert_equal_vec4( v4, 0.0, 0.0, 0.0, -1.0 );

  v2 = msh_vec2( 1.0, 2.0 );
  v3 = msh_vec3( 1.0, 2.0, 3.0 );
  v4 = msh_vec4( 1.0, 2.0, 3.0, 4.0 );
  assert_equal_vec2( v2, 1.0, 2.0 );
  assert_equal_vec3( v3, 1.0, 2.0, 3.0 );
  assert_equal_vec4( v4, 1.0, 2.0, 3.0, 4.0 );

  v2 = msh_vec2_value( 2.0 );
  v3 = msh_vec3_value( 2.0 );
  v4 = msh_vec4_value( 2.0 );
  assert_equal_vec2( v2, 2.0, 2.0 );
  assert_equal_vec3( v3, 2.0, 2.0, 2.0 );
  assert_equal_vec4( v4, 2.0, 2.0, 2.0, 2.0 );

  return MUNIT_OK;
}

MunitResult
test_msh_vec_math_vector_convert( const MunitParameter params[], void* fixture )
{
  msh_vec2_t v2; msh_vec3_t v3; msh_vec4_t v4;
  v4 = msh_vec4( 1.0, 2.0, 3.0, 4.0 );
  v3 = msh_vec4_to_vec3( v4 );
  return MUNIT_FAIL;
}

MunitResult
test_msh_vec_math_vector_arithmetic( const MunitParameter params[], void* fixture )
{
  msh_scalar_t d = 2.0;
  msh_vec2_t a2, b2, c2;
  a2 = msh_vec2( 1.0, 2.0 );
  b2 = msh_vec2( 3.0, 4.0 );

  c2 = msh_vec2_add( a2, b2 );
  assert_equal_vec2( c2, 4.0, 6.0 );
  c2 = msh_vec2_sub( b2, a2 );
  assert_equal_vec2( c2, 2.0, 2.0 );
  c2 = msh_vec2_mul( a2, b2 );
  assert_equal_vec2( c2, 3.0, 8.0 );
  c2 = msh_vec2_div( b2, a2 );
  assert_equal_vec2( c2, 3.0, 2.0 );

  c2 = msh_vec2_scalar_add( a2, d );
  assert_equal_vec2( c2, 3.0, 4.0 );
  c2 = msh_vec2_scalar_sub( a2, d );
  assert_equal_vec2( c2, -1.0, 0.0 );
  c2 = msh_vec2_scalar_mul( a2, d );
  assert_equal_vec2( c2, 2.0, 4.0 );
  c2 = msh_vec2_scalar_div( a2, d );
  assert_equal_vec2( c2, 0.5, 1.0 );

  msh_vec3_t a3, b3, c3;
  a3 = msh_vec3( 1.0, 2.0, 3.0 );
  b3 = msh_vec3( 3.0, 4.0, 6.0 );

  c3 = msh_vec3_add( a3, b3 );
  assert_equal_vec3( c3, 4.0, 6.0, 9.0 );
  c3 = msh_vec3_sub( b3, a3 );
  assert_equal_vec3( c3, 2.0, 2.0, 3.0 );
  c3 = msh_vec3_mul( a3, b3 );
  assert_equal_vec3( c3, 3.0, 8.0, 18.0 );
  c3 = msh_vec3_div( b3, a3 );
  assert_equal_vec3( c3, 3.0, 2.0, 2.0 );

  c3 = msh_vec3_scalar_add( a3, d );
  assert_equal_vec3( c3, 3.0, 4.0, 5.0 );
  c3 = msh_vec3_scalar_sub( a3, d );
  assert_equal_vec3( c3, -1.0, 0.0, 1.0 );
  c3 = msh_vec3_scalar_mul( a3, d );
  assert_equal_vec3( c3, 2.0, 4.0, 6.0 );
  c3 = msh_vec3_scalar_div( a3, d );
  assert_equal_vec3( c3, 0.5, 1.0, 1.5 );

  msh_vec4_t a4, b4, c4;
  a4 = msh_vec4( 1.0, 2.0, 3.0, 4.0 );
  b4 = msh_vec4( 3.0, 4.0, 6.0, 8.0 );

  c4 = msh_vec4_add( a4, b4 );
  assert_equal_vec4( c4, 4.0, 6.0, 9.0, 12.0 );
  c4 = msh_vec4_sub( b4, a4 );
  assert_equal_vec4( c4, 2.0, 2.0, 3.0, 4.0 );
  c4 = msh_vec4_mul( a4, b4 );
  assert_equal_vec4( c4, 3.0, 8.0, 18.0, 32.0 );
  c4 = msh_vec4_div( b4, a4 );
  assert_equal_vec4( c4, 3.0, 2.0, 2.0, 2.0 );

  c4 = msh_vec4_scalar_add( a4, d );
  assert_equal_vec4( c4, 3.0, 4.0, 5.0, 6.0 );
  c4 = msh_vec4_scalar_sub( a4, d );
  assert_equal_vec4( c4, -1.0, 0.0, 1.0, 2.0 );
  c4 = msh_vec4_scalar_mul( a4, d );
  assert_equal_vec4( c4, 2.0, 4.0, 6.0, 8.0 );
  c4 = msh_vec4_scalar_div( a4, d );
  assert_equal_vec4( c4, 0.5, 1.0, 1.5, 2.0 );

  return MUNIT_OK;
}