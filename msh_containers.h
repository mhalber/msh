/*
  ==================================================================================================
  Licensing information can be found at the end of the file.
  ==================================================================================================

  MSH_CONTAINERS.H v0.5

  Minimalistic containers header, contains implementation of a dynamic array (stretchy buffer) and a simple
  uint64 to uint64 hashtable.

  To use the library you simply add:
  
  #define MSH_STD_IMPLEMENTATION
  #include "msh_std.h"

  The define should only include once in your source.

  ==================================================================================================
  DEPENDENCIES
    This file depends on a number c stdlib header (see below).
    By default, these are not included by this library. If you'd like these to be included,
    define:
    #define MSH_STD_INCLUDE_LIBC_HEADERS

  ==================================================================================================
  AUTHORS:
    Maciej Halber

  ADDITIONAL CREDITS:
    Sean T. Barrett, Per Vognsen, Mattias Gustavsson, Randy Gaul, Ginger Bill, 
    Бранимир Караџић
    
    Please see particular sections for exact source of code / inspiration

  ==================================================================================================
  TODOs:
  [ ] Sorting and Searching
    [ ] Insetion sort
    [ ] Radix sort
    [ ] QuickSort
    [ ] Binary Search
    [ ] Quick Select
    [ ] Common qsort comparator functions

  ==================================================================================================
  REFERENCES:
  [1] stb.h           https://github.com/nothings/stb/blob/master/stb.h
  [2] gb.h            https://github.com/gingerBill/gb/blob/master/gb.h
  [3] stretchy_buffer https://github.com/nothings/stb/blob/master/stretchy_buffer.h
  [4] cute_headers    https://github.com/RandyGaul/cute_headers
  [5] libs            https://github.com/gingerBill/gb
*/
#ifndef MSH_CONTAINERS
#define MSH_CONTAINERS

#ifdef MSH_CONTAINERS_INCLUDE_HEADERS
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#endif

#ifdef MSH_CONTAINERS_STATIC
#define MSH_CONT_DEF static
#else
#define MSH_CONT_DEF extern
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic Size Array
// Credits
//   Seat T. Barrett - idea of stretchy buffers (?)
//   Per Vognsen     - various improvements from his ion implementation
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct msh_array_header
{
  size_t len;
  size_t cap;
} msh_array_hdr_t;

#define msh_array(T) T*

MSH_CONT_DEF void* msh_array__grow_fnct( void *array, size_t new_len, size_t elem_size );
MSH_CONT_DEF char* msh_array__printf(char *buf, const char *fmt, ...);

#define msh_array__grow_formula(x) ( (2.0*(x))+8)
#define msh_array__hdr(a)          ( (msh_array_hdr_t*)( (char*)(a) - sizeof(msh_array_hdr_t)) )

#define msh_array_len(a)           ( (a) ? (msh_array__hdr((a))->len) : 0 )
#define msh_array_cap(a)           ( (a) ? (msh_array__hdr((a))->cap) : 0 )
#define msh_array_sizeof(a)        ( (a) ? (msh_array__hdr((a))->len * sizeof(*(a))) : 0)
#define msh_array_isempty(a)       ( (a) ? (msh_array__hdr((a))->len <= 0) : 1 )

#define msh_array_front(a)         ( (a) ? (a) : NULL)
#define msh_array_back(a)          ( msh_array_len((a)) ? ((a) + msh_array_len((a)) - 1 ) : NULL)
#define msh_array_end(a)           ( (a) + msh_array_len((a)) ) // One past the end
#define msh_array_pop(a)           ( (a) ? (msh_array__hdr((a))->len--) : 0 )

#define msh_array_free(a)          ( (a) ? (free(msh_array__hdr(a)), (a) = NULL) : 0 )
#define msh_array_clear(a)         ( (a) ? (msh_array__hdr((a))->len = 0) : 0 )
#define msh_array_fit(a, n)        ( (n) <= msh_array_cap(a) ? (0) : msh_array__grow(a, n) )
#define msh_array_push(a, ...)     ( msh_array_fit((a), 1 + msh_array_len((a))),\
                                     (a)[msh_array__hdr(a)->len++] = (__VA_ARGS__) )
#define msh_array_copy( d, s, n )  ( msh_array_fit((d), (n)),\
                                     msh_array__hdr((d))->len = (n),\
                                     memcpy( (void*)(d), (void*)(s), (n) * sizeof(*(d) )))
#define msh_array_printf(b, ...)   ( (b) = msh_array__printf((b), __VA_ARGS__))
#define msh_array__grow(a, n)      ( *((void **)&(a)) = msh_array__grow_fnct((a), (n), sizeof(*(a))))

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sorting
// [ ] Make eneric with a code gen?
////////////////////////////////////////////////////////////////////////////////////////////////////

MSH_CONT_DEF void msh_insertion_sort( int32_t* arr, size_t n );
MSH_CONT_DEF void msh_insertion_sortf( float* arr, size_t n );
MSH_CONT_DEF void msh_insertion_sortd( double* arr, size_t n );

////////////////////////////////////////////////////////////////////////////////////////////////////
// Hash Table
// TODO(maciej):
//  [ ] Remove elements
//  [ ] Benchmarking?
//
// Credits
//   Seat T. Barrett: Judy Array vs. Hash-table text
//   Niklas Frykholm: The Machinery Container system blog-series
//   Per Vognsen    : ios's open-addressing, linear probing hashtable
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct msh_map
{
  uint64_t* keys;
  uint64_t* vals;
  size_t _len;
  size_t _cap;
} msh_map_t;

MSH_CONT_DEF uint64_t  msh_hash_uint64( uint64_t x );
MSH_CONT_DEF uint64_t  msh_hash_ptr( void *ptr );
MSH_CONT_DEF uint64_t  msh_hash_str( const char *str );

MSH_CONT_DEF void      msh_map_init( msh_map_t* map, uint32_t cap );
MSH_CONT_DEF void      msh_map_free( msh_map_t* map );

MSH_CONT_DEF size_t    msh_map_len( msh_map_t* map );
MSH_CONT_DEF size_t    msh_map_cap( msh_map_t* map ); 

MSH_CONT_DEF void      msh_map_insert( msh_map_t* map, uint64_t key, uint64_t val );
MSH_CONT_DEF uint64_t* msh_map_get( const msh_map_t* map, uint64_t key );

MSH_CONT_DEF void      msh_map_get_iterable_keys( const msh_map_t* map, uint64_t** keys );
MSH_CONT_DEF void      msh_map_get_iterable_vals( const msh_map_t* map, uint64_t** vals );
MSH_CONT_DEF void      msh_map_get_iterable_keys_and_vals( const msh_map_t* map, uint64_t** key, uint64_t** val );

////////////////////////////////////////////////////////////////////////////////////////////////////
// Disjoint Set
////////////////////////////////////////////////////////////////////////////////////////////////////

struct msh_dset;
MSH_CONT_DEF void     msh_dset_init( struct msh_dset* dset, size_t n_vals );
MSH_CONT_DEF void     msh_dset_term( struct msh_dset* dset );
MSH_CONT_DEF uint64_t msh_dset_find( struct msh_dset* dset, uint64_t idx );
MSH_CONT_DEF void     msh_dset_union( struct msh_dset* dset, uint64_t idx_a, uint64_t idx_b );

////////////////////////////////////////////////////////////////////////////////////////////////////
// Heap for all basic numeric types using C11 Generics
//
// Credits:
//   Joerg Arndt, 'Matters Computational'
////////////////////////////////////////////////////////////////////////////////////////////////////

#define msh_heap_make(x,y) _Generic((x),     \
            int8_t*: msh_heap_make_i8,       \
            int16_t*: msh_heap_make_i16,     \
            int32_t*: msh_heap_make_i32,     \
            int64_t*: msh_heap_make_i64,     \
            uint8_t*: msh_heap_make_ui8,     \
            uint16_t*: msh_heap_make_ui16,   \
            uint32_t*: msh_heap_make_ui32,   \
            uint64_t*: msh_heap_make_ui64,   \
            float*: msh_heap_make_f32,       \
            double*: msh_heap_make_f64,      \
            default: msh_heap_make_i32)(x, y)

#define msh_heap_pop(x,y) _Generic((x),     \
            int8_t*: msh_heap_pop_i8,       \
            int16_t*: msh_heap_pop_i16,     \
            int32_t*: msh_heap_pop_i32,     \
            int64_t*: msh_heap_pop_i64,     \
            uint8_t*: msh_heap_pop_ui8,     \
            uint16_t*: msh_heap_pop_ui16,   \
            uint32_t*: msh_heap_pop_ui32,   \
            uint64_t*: msh_heap_pop_ui64,   \
            float*: msh_heap_pop_f32,       \
            double*: msh_heap_pop_f64,      \
            default: msh_heap_pop_i32)(x, y)

#define msh_heap_push(x,y) _Generic((x),     \
            int8_t*: msh_heap_push_i8,       \
            int16_t*: msh_heap_push_i16,     \
            int32_t*: msh_heap_push_i32,     \
            int64_t*: msh_heap_push_i64,     \
            uint8_t*: msh_heap_push_ui8,     \
            uint16_t*: msh_heap_push_ui16,   \
            uint32_t*: msh_heap_push_ui32,   \
            uint64_t*: msh_heap_push_ui64,   \
            float*: msh_heap_push_f32,       \
            double*: msh_heap_push_f64,      \
            default: msh_heap_push_i32)(x, y)

#define msh_heap_isvalid(x,y) _Generic((x),     \
            int8_t*: msh_heap_isvalid_i8,       \
            int16_t*: msh_heap_isvalid_i16,     \
            int32_t*: msh_heap_isvalid_i32,     \
            int64_t*: msh_heap_isvalid_i64,     \
            uint8_t*: msh_heap_isvalid_ui8,     \
            uint16_t*: msh_heap_isvalid_ui16,   \
            uint32_t*: msh_heap_isvalid_ui32,   \
            uint64_t*: msh_heap_isvalid_ui64,   \
            float*: msh_heap_isvalid_f32,       \
            double*: msh_heap_isvalid_f64,      \
            default: msh_heap_isvalid_i32)(x, y)

#endif  /* MSH_CONTAINERS */

#ifdef MSH_CONTAINERS_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic Array
////////////////////////////////////////////////////////////////////////////////////////////////////

MSH_CONT_DEF void*
msh_array__grow_fnct( void *array, size_t new_len, size_t elem_size ) {
  if( new_len <= msh_array_len( array ) )
  {
    return array;
  }
  size_t old_cap = msh_array_cap( array );
  size_t new_cap = (size_t)msh_array__grow_formula( old_cap );
  new_len = (new_len < 16) ? 16 : new_len;
  new_cap = (new_cap < new_len) ? new_len : new_cap;
  assert( new_len <= new_cap );
  size_t new_size = sizeof(msh_array_hdr_t) + new_cap * elem_size;
  msh_array_hdr_t *new_hdr;
  if( array )
  {
    new_hdr = (msh_array_hdr_t*)realloc( msh_array__hdr( array ), new_size );
  } 
  else
  {
    new_hdr = (msh_array_hdr_t*)malloc( new_size );
    new_hdr->len = 0;
  }
  new_hdr->cap = new_cap;
  return (void*)((char*)new_hdr + sizeof(msh_array_hdr_t));
}

MSH_CONT_DEF char*
msh_array__printf(char *buf, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  size_t cap = msh_array_cap(buf) - msh_array_len(buf) ;
  int64_t len = vsnprintf( msh_array_end(buf), cap, fmt, args );
  if( len < 0 ) { len = cap; }
  size_t n = 1 + len;
  va_end(args);
  if( n > cap ) {
    msh_array_fit( buf, n + msh_array_len(buf) );
    va_start(args, fmt);
    size_t new_cap = msh_array_cap(buf) - msh_array_len(buf);
    len = vsnprintf( msh_array_end(buf), new_cap, fmt, args);
    if( len < 0 ) { len = new_cap; }
    n = 1 + len;
    assert(n <= new_cap);
    va_end(args);
  }
  msh_array__hdr(buf)->len += (n - 1);
  return buf;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Sort
////////////////////////////////////////////////////////////////////////////////////////////////////

void
msh_insertion_sort( int32_t* arr, size_t n )
{
  assert( arr );
  if( n <= 1 ) { return; }
  size_t i, j;
  int32_t x;

  i = 1;
  while( i < n )
  {
    x = arr[i];
    j = i - 1;
    while( j > 0 && arr[j] > x )
    {
      arr[j+1] = arr[j];
      j--;
    }
    arr[j+1] = x;
    i++;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Hash Table
////////////////////////////////////////////////////////////////////////////////////////////////////

MSH_CONT_DEF uint64_t
msh_hash_uint64( uint64_t x ) 
{
  x *= 0xff51afd7ed558ccd;
  x ^= x >> 32;
  return x;
}

MSH_CONT_DEF uint64_t
msh_hash_ptr( void *ptr ) 
{
  return msh_hash_uint64( (uintptr_t)ptr );
}

MSH_CONT_DEF uint64_t
msh_hash_str( const char *str ) 
{
  uint64_t x = 0xcbf29ce484222325;
  char *buf = (char*)str;
  while( *buf != 0 )
  {
    x ^= *buf;
    x *= 0x100000001b3;
    x ^= x >> 32;
    buf++;
  }
  return x;
}

static size_t
msh_map__pow2ceil( uint32_t v )
{
  --v;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  ++v;
  v += ( v == 0 );
  return v;
}

MSH_CONT_DEF void
msh_map_init( msh_map_t *map, uint32_t cap )
{
  assert( !map->keys && !map->vals );
  cap = msh_map__pow2ceil( cap );
  map->keys = (uint64_t*)calloc( cap, sizeof(uint64_t) );
  map->vals = (uint64_t*)malloc( cap * sizeof(uint64_t) );
  map->_len = 0;
  map->_cap = cap;
}

MSH_CONT_DEF void
msh_map__grow( msh_map_t *map, size_t new_cap ) {
  new_cap = (new_cap < 16) ? 16 : new_cap;
  msh_map_t new_map;
  new_map.keys = (uint64_t*)calloc( new_cap, sizeof(uint64_t) );
  new_map.vals = (uint64_t*)malloc( new_cap * sizeof(uint64_t) );
  new_map._len = 0;
  new_map._cap = new_cap;

  for( size_t i = 0; i < map->_cap; i++ ) 
  {
    if( map->keys[i] )
    {
      msh_map_insert( &new_map, map->keys[i] - 1, map->vals[i] );
    }
  }
  free( map->keys );
  free( map->vals );
  *map = new_map;
}

MSH_CONT_DEF size_t
msh_map_len( msh_map_t* map )
{
  return map->_len;
}

MSH_CONT_DEF size_t
msh_map_cap( msh_map_t* map )
{
  return map->_cap;
}

MSH_CONT_DEF void
msh_map_insert( msh_map_t* map, uint64_t key, uint64_t val )
{
  // Increment the key, so that key == 0 is valid, even though 0 is marking empty slot.
  assert( key < (0xffffffffffffffffULL - 1) );
  key += 1;
  if( 2 * map->_len >= map->_cap) { msh_map__grow( map, 2 * map->_cap ); }
  assert( 2 * map->_len < map->_cap );
  size_t i = (size_t)msh_hash_uint64( key );
  for (;;) 
  {
    i &= map->_cap - 1;
    if( !map->keys[i] )
    {
      map->_len++;
      map->keys[i] = key;
      map->vals[i] = val;
      return;
    } 
    else if( map->keys[i] == key )
    {
      map->vals[i] = val;
      return;
    }
    i++;
  }
}

MSH_CONT_DEF uint64_t*
msh_map_get( const msh_map_t* map, uint64_t key )
{
  if( map->_len == 0 ) { return NULL; }
  key += 1;
  size_t i = (size_t)msh_hash_uint64( key );
  assert( map->_len < map->_cap );
  for( ;; ) {
    i &= map->_cap - 1;
    if( map->keys[i] == key )
    {
      return &map->vals[i];
    } 
    else if( !map->keys[i] ) 
    {
      return NULL;
    }
    i++;
  }
}

MSH_CONT_DEF void
msh_map_free( msh_map_t* map )
{
  free( map->keys );
  free( map->vals );
  map->_cap = 0;
  map->_len = 0;
}

MSH_CONT_DEF void
msh_map_get_iterable_keys( const msh_map_t* map, uint64_t** keys )
{
  assert( (*keys) == NULL );
  (*keys) = (uint64_t*)malloc( map->_len * sizeof(uint64_t) );
  size_t j = 0;
  for( size_t i = 0; i < map->_cap; ++i )
  {
    if( map->keys[i] ) { (*keys)[j++] = map->keys[i] - 1; }
  }
}

MSH_CONT_DEF void
msh_map_get_iterable_vals( const msh_map_t* map, uint64_t** vals )
{
  assert( (*vals) == NULL );
  (*vals) = (uint64_t*)malloc( map->_len * sizeof(uint64_t) );
  size_t j = 0;
  for( size_t i = 0; i < map->_cap; ++i )
  {
    if( map->keys[i] ) { (*vals)[j++] = map->vals[i]; }
  }
}

MSH_CONT_DEF void
msh_map_get_iterable_keys_and_vals( const msh_map_t* map, uint64_t** keys, uint64_t** vals )
{
  assert( (*keys) == NULL );
  assert( (*vals) == NULL );
  (*keys) = (uint64_t*)malloc( map->_len * sizeof(uint64_t) );
  (*vals) = (uint64_t*)malloc( map->_len * sizeof(uint64_t) );
  size_t j = 0;
  for( size_t i = 0; i < map->_cap; ++i )
  {
    if( map->keys[i] )
    { 
      (*keys)[j] = map->keys[i] - 1;
      (*vals)[j] = map->vals[i];
      j++;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Generic Heap
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MSH__HEAPIFY_DEF(T, postfix)                                          \
static void                                                                   \
msh__heapify_##postfix( T *vals, size_t vals_len, size_t cur )                \
{                                                                             \
  size_t max = cur;                                                           \
  const size_t left  = (cur<<1) + 1; /* multiply by two */                    \
  const size_t right = (cur<<1) + 2; /* multiply by two */                    \
  if( (left < vals_len)  && ( vals[left] > vals[cur] ) )  { max = left; }     \
  if( (right < vals_len) && ( vals[right] > vals[max] ) ) { max = right; }    \
  if( max != cur ) /* need to swap */                                         \
  {                                                                           \
    T tmp = vals[cur];                                                        \
    vals[cur] = vals[max];                                                    \
    vals[max] = tmp;                                                          \
    msh__heapify_##postfix( vals, vals_len, max );                            \
  }                                                                           \
}

#define MSH_HEAP_MAKE_DEF(T, postfix)                                         \
MSH_CONT_DEF void                                                             \
msh_heap_make_##postfix( T* vals, size_t vals_len )                           \
{                                                                             \
  int64_t i = vals_len >> 1;                                                  \
  while ( i >= 0 ) { msh__heapify_##postfix( vals, vals_len, i-- ); }         \
}                                                                             \

#define MSH_HEAP_POP_DEF(T, postfix)                                          \
MSH_CONT_DEF void                                                             \
msh_heap_pop_##postfix( T* vals, size_t vals_len )                            \
{                                                                             \
  T max = vals[0];                                                            \
  vals[0] = vals[vals_len-1];                                                 \
  vals[vals_len-1] = max;                                                     \
  vals_len--;                                                                 \
  if( vals_len > 0 ){ msh__heapify_##postfix( vals, vals_len, 0 ); }          \
}

#define MSH_HEAP_PUSH_DEF(T, postfix)                                         \
MSH_CONT_DEF void                                                             \
msh_heap_push_##postfix( T* vals, size_t vals_len )                           \
{                                                                             \
  int64_t i = vals_len-1;                                                     \
  T v = vals[i];                                                              \
  while( i > 0 )                                                              \
  {                                                                           \
    int64_t j = (i-1) >> 1;                                                   \
    if( vals[j] >= v ) break;                                                 \
    vals[i] = vals[j];                                                        \
    i = j;                                                                    \
  }                                                                           \
  vals[i] = v;                                                                \
}

#define MSH_HEAP_ISVALID_DEF(T, postfix)                                      \
MSH_CONT_DEF bool                                                             \
msh_heap_isvalid_##postfix( T* vals, size_t vals_len )                        \
{                                                                             \
  for( int i = vals_len - 1; i > 0; --i )                                     \
  {                                                                           \
    size_t parent = (i-1) >> 1;                                               \
    if( vals[i] > vals[parent] ) { return false; }                            \
  }                                                                           \
  return true;                                                                \
}

MSH__HEAPIFY_DEF(int8_t, i8)
MSH_HEAP_MAKE_DEF(int8_t, i8)
MSH_HEAP_POP_DEF(int8_t, i8)
MSH_HEAP_PUSH_DEF(int8_t, i8)
MSH_HEAP_ISVALID_DEF(int8_t, i8)

MSH__HEAPIFY_DEF(int16_t, i16)
MSH_HEAP_MAKE_DEF(int16_t, i16)
MSH_HEAP_POP_DEF(int16_t, i16)
MSH_HEAP_PUSH_DEF(int16_t, i16)
MSH_HEAP_ISVALID_DEF(int16_t, i16)

MSH__HEAPIFY_DEF(int32_t, i32)
MSH_HEAP_MAKE_DEF(int32_t, i32)
MSH_HEAP_POP_DEF(int32_t, i32)
MSH_HEAP_PUSH_DEF(int32_t, i32)
MSH_HEAP_ISVALID_DEF(int32_t, i32)

MSH__HEAPIFY_DEF(int64_t, i64)
MSH_HEAP_MAKE_DEF(int64_t, i64)
MSH_HEAP_POP_DEF(int64_t, i64)
MSH_HEAP_PUSH_DEF(int64_t, i64)
MSH_HEAP_ISVALID_DEF(int64_t, i64)

MSH__HEAPIFY_DEF(uint8_t, ui8)
MSH_HEAP_MAKE_DEF(uint8_t, ui8)
MSH_HEAP_POP_DEF(uint8_t, ui8)
MSH_HEAP_PUSH_DEF(uint8_t, ui8)
MSH_HEAP_ISVALID_DEF(uint8_t, ui8)

MSH__HEAPIFY_DEF(uint16_t, ui16)
MSH_HEAP_MAKE_DEF(uint16_t, ui16)
MSH_HEAP_POP_DEF(uint16_t, ui16)
MSH_HEAP_PUSH_DEF(uint16_t, ui16)
MSH_HEAP_ISVALID_DEF(uint16_t, ui16)

MSH__HEAPIFY_DEF(uint32_t, ui32)
MSH_HEAP_MAKE_DEF(uint32_t, ui32)
MSH_HEAP_POP_DEF(uint32_t, ui32)
MSH_HEAP_PUSH_DEF(uint32_t, ui32)
MSH_HEAP_ISVALID_DEF(uint32_t, ui32)

MSH__HEAPIFY_DEF(uint64_t, ui64)
MSH_HEAP_MAKE_DEF(uint64_t, ui64)
MSH_HEAP_POP_DEF(uint64_t, ui64)
MSH_HEAP_PUSH_DEF(uint64_t, ui64)
MSH_HEAP_ISVALID_DEF(uint64_t, ui64)

MSH__HEAPIFY_DEF(float, f32)
MSH_HEAP_MAKE_DEF(float, f32)
MSH_HEAP_POP_DEF(float, f32)
MSH_HEAP_PUSH_DEF(float, f32)
MSH_HEAP_ISVALID_DEF(float, f32)

MSH__HEAPIFY_DEF(double, f64)
MSH_HEAP_MAKE_DEF(double, f64)
MSH_HEAP_POP_DEF(double, f64)
MSH_HEAP_PUSH_DEF(double, f64)
MSH_HEAP_ISVALID_DEF(double, f64)


////////////////////////////////////////////////////////////////////////////////////////////////////
// Disjoint Set
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct msh_dset_el
{
  uint64_t parent;
  uint32_t rank;
  uint32_t size;
} msh_dset_el_t;

typedef struct msh_dset
{
  msh_dset_el_t* elems;
  uint32_t num_sets;
} msh_dset_t;

MSH_CONT_DEF void
msh_dset_init( msh_dset_t* dset, size_t n_vals )
{
  assert( dset );
  assert( dset->elems == NULL );
  dset->elems = (msh_dset_el_t*)malloc( sizeof(msh_dset_el_t) * n_vals );
  for( size_t i = 0; i < n_vals; ++i )
  {
    dset->elems[i].parent = i;
    dset->elems[i].rank   = 0;
    dset->elems[i].size   = 1;
  }
  dset->num_sets = n_vals;
}

MSH_CONT_DEF void
msh_dset_term( msh_dset_t* dset )
{
  assert( dset );
  assert( dset->elems );
  free( dset->elems );
  dset->elems    = NULL;
  dset->num_sets = 0;
}

MSH_CONT_DEF uint64_t
msh_dset_find( msh_dset_t* dset, uint64_t idx )
{
  // Path compression
  uint64_t parent = dset->elems[idx].parent;
  if( parent == idx )
  {
    return parent;
  }
  parent = msh_dset_find( dset, parent );
  return parent;
}

MSH_CONT_DEF void
msh_dset_union( msh_dset_t* dset, uint64_t idx_a, uint64_t idx_b )
{
  uint64_t root_a = msh_dset_find( dset, idx_a );
  uint64_t root_b = msh_dset_find( dset, idx_b );
 
  // check if in the same set
  if( root_a == root_b)
  {
    return;
  }
 
  // if not need to do union(by rank)
  if( root_a < root_b )
  {
    uint64_t tmp = root_a;
    root_a = root_b;
    root_b = tmp;
  }
  dset->elems[root_b].parent = root_a;
  dset->elems[root_a].size += dset->elems[root_b].size;
  if( dset->elems[root_a].rank == dset->elems[root_b].rank )
  {
    dset->elems[root_a].rank++;
  }
  dset->num_sets--;
}


#endif  /* MSH_CONTAINERS_IMPLEMENTATION*/

/*
	------------------------------------------------------------------------------
	This software is available under 2 licenses - you may choose the one you like.
	------------------------------------------------------------------------------
	ALTERNATIVE A - zlib license
	Copyright (c) 2016-2019 Maciej Halber; Mattias Gustavsson; Randy Gaul http://www.randygaul.net
	This software is provided 'as-is', without any express or implied warranty.
	In no event will the authors be held liable for any damages arising from
	the use of this software.
	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:
	  1. The origin of this software must not be misrepresented; you must not
	     claim that you wrote the original software. If you use this software
	     in a product, an acknowledgment in the product documentation would be
	     appreciated but is not required.
	  2. Altered source versions must be plainly marked as such, and must not
	     be misrepresented as being the original software.
	  3. This notice may not be removed or altered from any source distribution.
	------------------------------------------------------------------------------
	ALTERNATIVE B - Public Domain (www.unlicense.org)
	This is free and unencumbered software released into the public domain.
	Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
	software, either in source code form or as a compiled binary, for any purpose, 
	commercial or non-commercial, and by any means.
	In jurisdictions that recognize copyright laws, the author or authors of this 
	software dedicate any and all copyright interest in the software to the public 
	domain. We make this dedication for the benefit of the public at large and to 
	the detriment of our heirs and successors. We intend this dedication to be an 
	overt act of relinquishment in perpetuity of all present and future rights to 
	this software under copyright law.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
	AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
	ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
	------------------------------------------------------------------------------
*/