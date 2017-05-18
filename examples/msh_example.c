#include <stdio.h>
#include <stdlib.h>
#include "msh.h"

typedef struct S {
  int a;
  int b;
  float c;
} S_t;

int main()
{
  // ASSERT tests
  int a = 5;
  MSH_ASSERT_MSG( a == 5, "A is should be 5!" );
  a = 2;
  MSH_ASSERT_MSG( a == 2, "A is should be 2!" );
  int * int_ptr = malloc(10);
  int_ptr = realloc(int_ptr, 20);
  MSH_ASSERT_NOT_NULL( int_ptr );
  free( int_ptr );

  // MACRO tests
  int standard_array[512];
  printf("Size of int in bytes: %lld\n", msh_size_of(int));
  printf("Size of standard_array: %lld\n", msh_count_of(standard_array) );
  printf("Offset of member c for Structure S: %lld\n", msh_offset_of(S_t, c) );
  printf("Minumum of 56, -5, 76: %d\n", msh_min3(56, -5, 76));
  printf("Maximum of 56, -5, 76: %d\n", msh_max3(56, -5, 76));
  printf("Clamping 1.5f to 0-1 range: %f\n", msh_clamp01(1.5));
  printf("Clamping -1.5f to 0-1 range: %f\n", msh_clamp01(-1.5));
  printf("Clamping 259 to 0-255 range: %d\n", msh_clamp(259, 0, 255));
  printf("Clamping -59 to 0-255 range: %d\n", msh_clamp(-59, 0, 255));
  printf("256 %s within the range 0 - 512\n", msh_within(256, 0, 512) ? "is" : "is not" );
  printf("1444 %s within the range 0 - 512\n", msh_within(1444, 0, 512) ? "is" : "is not" );
  printf("Absolute value of -3.712 is : %f\n", msh_abs(-3.712) );
  printf("PI is equal to : %f rad ( %f deg )\n", msh_deg2rad(msh_rad2deg(msh_PI)), 
                                                 msh_rad2deg(msh_PI) );

  // ARRAY test
  msh_array(int32_t) array = NULL;
  msh_array_init( array, 5 );
  printf( "\nStart Count: %lld Capacity: %lld\n", msh_array_count(array), 
                                                msh_array_capacity(array) );
  msh_array_push( array, 1 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 3 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 4 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 6 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 7 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 28 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 9 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 10 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 64 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 72 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 23 );
  printf("Count: %lld Capacity: %lld\n", msh_array_count(array), msh_array_capacity(array) );
  msh_array_push( array, 81 );

  msh_array_free( array );
  msh_array_resize( array, 100 );
  msh_array_pop(array);
  msh_array_push( array, 201);
  msh_array_push( array, 101);

  printf( "End Count: %lld Capacity: %lld\n", msh_array_count(array), 
                                                msh_array_capacity(array) );
  msh_array_free(array);



  return 1;
}