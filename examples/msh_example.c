#include <stdio.h>
#include <stdlib.h>
#include "msh.h"



// #define gb_array_set_capacity(x, capacity) do { \
// 	if (x) { \
// 		void **gb__array_ = cast(void **)&(x); \
// 		*gb__array_ = gb__array_set_capacity((x), (capacity), gb_size_of(*(x))); \
// 	} \
// } while (0)

// // NOTE(bill): Do not use the thing below directly, use the macro
// GB_DEF void *gb__array_set_capacity(void *array, isize capacity, isize element_size);


// // TODO(bill): Decide on a decent growing formula for gbArray
// #define gb_array_grow(x, min_capacity) do { \
// 	isize new_capacity = GB_ARRAY_GROW_FORMULA(gb_array_capacity(x)); \
// 	if (new_capacity < (min_capacity)) \
// 		new_capacity = (min_capacity); \
// 	gb_array_set_capacity(x, new_capacity); \
// } while (0)


// #define gb_array_append(x, item) do { \
// 	if (gb_array_capacity(x) < gb_array_count(x)+1) \
// 		gb_array_grow(x, 0); \
// 	(x)[gb_array_count(x)++] = (item); \
// } while (0)

// #define gb_array_appendv(x, items, item_count) do { \
// 	gbArrayHeader *gb__ah = GB_ARRAY_HEADER(x); \
// 	GB_ASSERT(gb_size_of((items)[0]) == gb_size_of((x)[0])); \
// 	if (gb__ah->capacity < gb__ah->count+(item_count)) \
// 		gb_array_grow(x, gb__ah->count+(item_count)); \
// 	gb_memcopy(&(x)[gb__ah->count], (items), gb_size_of((x)[0])*(item_count));\
// 	gb__ah->count += (item_count); \
// } while (0)



// #define gb_array_pop(x)   do { GB_ASSERT(GB_ARRAY_HEADER(x)->count > 0); GB_ARRAY_HEADER(x)->count--; } while (0)
// #define gb_array_clear(x) do { GB_ARRAY_HEADER(x)->count = 0; } while (0)

// #define gb_array_resize(x, new_count) do { \
// 	if (GB_ARRAY_HEADER(x)->capacity < (new_count)) \
// 		gb_array_grow(x, (new_count)); \
// 	GB_ARRAY_HEADER(x)->count = (new_count); \
// } while (0)


// #define gb_array_reserve(x, new_capacity) do { \
// 	if (GB_ARRAY_HEADER(x)->capacity < (new_capacity)) \
// 		gb_array_set_capacity(x, new_capacity); \
// } while (0)

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