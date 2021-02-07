MunitResult
test_msh_discrete_distrib_sampling( const MunitParameter params[], void* fixture )
{
  (void) params;
  (void) fixture;

  double weights[] = { 1, 2, 3, 4, 5, 4, 3, 2, 1 };
  msh_discrete_distrib_t sampler = {0};
  msh_discrete_distribution_init( &sampler, weights, msh_count_of(weights), munit_rand_uint32() );
  munit_assert_size( msh_count_of(weights), ==, sampler.n_weights );

  int32_t n_samples = 1000000;
  int32_t counts[msh_count_of(weights)] = {0};
  for( int32_t i = 0; i < n_samples; ++i )
  {
    int32_t sample = msh_discrete_distribution_sample( &sampler );
    counts[sample]++;
  }

  munit_assert_double_equal( ((float)counts[4] / counts[0]), (double)weights[4], 1 );
  munit_assert_double_equal( ((float)counts[4] / counts[8]), (double)weights[4], 1 );
  munit_assert_double_equal( ((float)counts[1] / counts[7]), 1.0, 1 );
  munit_assert_double_equal( ((float)counts[2] / counts[6]), 1.0, 1 );
  munit_assert_double_equal( ((float)counts[3] / counts[5]), 1.0, 1 );

  for( size_t i = 0; i < msh_count_of(weights); ++i )
  {
    weights[i] = 0;
    counts[i] = 0;
  }
  weights[8] = 1;

  msh_discrete_distribution_update( &sampler, weights, msh_count_of(weights) );
  for( int32_t i = 0; i < n_samples; ++i )
  {
    int32_t sample = msh_discrete_distribution_sample( &sampler );
    counts[sample]++;
  }
  munit_assert_int32( counts[8], ==, n_samples );

  msh_discrete_distribution_free( &sampler );
  munit_assert_size( 0, ==, sampler.n_weights );
  return MUNIT_OK;
}
