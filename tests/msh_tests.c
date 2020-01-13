#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#define MSH_VEC_MATH_DOUBLE_PRECISION
#define MSH_CONTAINERS_IMPLEMENTATION
#include "msh/msh_std.h"
#include "msh/msh_containers.h"
#include "msh/msh_vec_math.h"

#include "munit/munit.h"

#include "msh_containers_test.inl"
#include "msh_stats_test.inl"
#include "msh_vec_math_test.inl"


int main(int argc, char* const argv[MUNIT_ARRAY_PARAM(argc + 1)])
{
  MunitTest vec_math_tests[] = 
  {
    { .name = (char*) "/vector_init",          .test = test_msh_vec_math_vector_init },
    { .name = (char*) "/vector_arithmetic",    .test = test_msh_vec_math_vector_arithmetic },
    { 0 }
  };

  MunitSuite vec_math_test_suite = 
  {
    .prefix = (char*) "/vec_math",
    .tests = vec_math_tests,
    .suites = NULL,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE
  };

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

  MunitTest misc_test[] = 
  {
    { .name = (char*) "/disjoint_set_api",    .test = test_msh_disjoint_set_api, },
    { .name = (char*) "/heap_api",            .test = test_msh_heap_api, },
    { 0 }
  };

  MunitSuite misc_test_suite = 
  {
    .prefix = (char*) "/misc",
    .tests = misc_test,
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

  MunitSuite test_suites_array[] = { vec_math_test_suite, array_test_suite, misc_test_suite, map_test_suite, stats_test_suite, {0} };

  const MunitSuite test_suites = 
  {
    .prefix = (char*) "msh",
    .tests = NULL,
    .suites = test_suites_array,
    .iterations = 1,
    .options = MUNIT_SUITE_OPTION_NONE
  };

  return munit_suite_main( &test_suites, NULL, argc, argv );
}