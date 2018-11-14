/*
  ==============================================================================
  
  MSH_STD.H v0.5
  
  A single header library for extending the C standard library.
  This file is partially written by myself, but also includes a lot
  of code copied / adapted from other libraries. Please see credits for details.

  To use the library you simply add:
  
  #define MSH_STD_IMPLEMENTATION
  #include "msh_std.h"

  The define should only include once in your source.

  ==============================================================================
  DEPENDENCIES
    This file includes and depends a number c stdlib headers at this point:
     - <assert.h>
     - <math.h>
     - <string.h>
     - <stdint.h>
     - <stdarg.h>
     - <stddef.h>
     - <stdbool.h>
     - <stdio.h>
     - <stdlib.h>
     - <float.h>
     - <sys/stat.h> (on Linux)
    By default, these are not included by this library. If you'd like these to be included,
    define:
    #define MSH_STD_INCLUDE_C_HEADERS


  ==============================================================================
  AUTHORS:
    Maciej Halber

  CREDITS:
    Sean T. Barrett, Per Vognsen, Mattias Gustavsson, Randy Gaul, Ginger Bill
    Please see particular sections for exact source of code / inspiration

  ==============================================================================
  TODOs:
  [x] Limits
  [ ] Simple set implementation
  [ ] Path manipulation
      [ ] Implement the string concatenation code with some version of vsnprintf
  [ ] Memory allocation
    [ ] Tracking memory allocs
    [ ] Alternative allocators
  [ ] Inline keyword fixes
  [ ] Sorting and Searching
    [ ] Common qsort comparator functions
    [ ] binary and linear searches over arrays
  [ ] Multithreading / Scheduling
  [x] Stats - cdf inversion
  [ ] Custom prints (stb_sprintf inlined, look at replacing sprintf with "write" function in linux (unistd.h))

  ==============================================================================
  REFERENCES:
  [1] stb.h           https://github.com/nothings/stb/blob/master/stb.h
  [2] gb.h            https://github.com/gingerBill/gb/blob/master/gb.h
  [3] stretchy_buffer https://github.com/nothings/stb/blob/master/stretchy_buffer.h
  [4] cute_headers    https://github.com/RandyGaul/cute_headers
  [5] gb.h            https://github.com/gingerBill/gb
*/

#ifndef MSH_STD
#define MSH_STD

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MSH_STD_INCLUDE_LIBC_HEADERS

// c stdlib
// Need to double check how many of those are actually needed
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

// system specific
#ifdef __linux__ 
#include <sys/stat.h>
#endif

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous
//
// Credits
//  Ginger Bill: System and architecture detection from gb.h
////////////////////////////////////////////////////////////////////////////////////////////////////

#define msh_count_of(x) ((sizeof(x)/sizeof(*x)))

#ifdef MSH_STD_STATIC
#define MSHDEF static
#else
#define MSHDEF extern
#endif

#define msh_local_persitent static // Local variables with persisting values
#define msh_global          static // Global variables
#define msh_internal        static // Internal linkage

#if defined(_WIN32) || defined(_WIN64)
  #if defined(__MINGW32__)
    #ifndef MSH_SYSTEM_MSYS
    #define MSH_SYSTEM_MSYS 1
    #endif
  #else
    #ifndef MSH_SYSTEM_WINDOWS
    #define MSH_SYSTEM_WINDOWS 1
    #endif
  #endif
#elif defined(__APPLE__) && defined(__MACH__)
  #ifndef MSH_SYSTEM_OSX
  #define MSH_SYSTEM_OSX 1
  #endif
#elif defined(__unix__)
  #ifndef MSH_SYSTEM_UNIX
  #define MSH_SYSTEM_UNIX 1
  #endif
#else
  #error This operating system is not supported
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__64BIT__) || defined(__powerpc64__) || defined(__ppc64__)
  #ifndef MSH_ARCH_64_BIT
    #define MSH_ARCH_64_BIT 1
  #endif
#else
  #ifndef MSH_ARCH_32_BIT
    #define MSH_ARCH_32_BIT 1
  #endif
#endif

#if MSH_SYSTEM_WINDOWS
  #ifndef LLU_SYMBOL
    #define LLU_SYMBOL "%I64d"
  #endif
#else
  #ifndef LLU_SYMBOL
    #define LLU_SYMBOL "%llu"
  #endif
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug 
//
// Not entirely sure if this is that useful over the assert.h
// Deprecating this
//
// Credit
//  This is taken from gb.h by Ginger Bill.
////////////////////////////////////////////////////////////////////////////////////////////////////

// #if defined(_MSC_VER)
//   #if _MSC_VER < 1300
//     #define MSH_DEBUG_TRAP() __asm int 3 /* Trap to debugger! */
//   #else
//     #define MSH_DEBUG_TRAP() __debugbreak()
//   #endif
// #elif defined(__TINYC__) /*If using tcc, just segfault*/
//   #define MSH_DEBUG_TRAP() do{ int* ptr=NULL; int val = *ptr; } while(0)
// #else
//   #define MSH_DEBUG_TRAP() __builtin_trap()
// #endif

// #ifndef MSH_NDEBUG
//   #define MSH_ASSERT_MSG(cond, msg) do {                                         
//     if (!(cond)) {                                                               
//       msh__assert_handler(#cond, __FILE__, (int64_t)__LINE__, msg );             
//       MSH_DEBUG_TRAP();                                                          
//     }                                                                            
//   } while (0)
// #else
//   #define MSH_ASSERT_MSG(cond, msg) /* Expands to nothing */
// #endif /*MSH_NDEBUG*/


// #define MSH_ASSERT(cond) MSH_ASSERT_MSG(cond, NULL)

// #define MSH_ASSERT_NOT_NULL(ptr) MSH_ASSERT_MSG((ptr) != NULL, #ptr " must not be NULL")

// void msh__assert_handler( char const *condition, char const *file, int32_t line, char const *msg );


////////////////////////////////////////////////////////////////////////////////////////////////////
// Printing
////////////////////////////////////////////////////////////////////////////////////////////////////
/* TODOs:
 *  [ ] inline vsnprintf from stb
 */
#define msh_cprintf(cond, fmt, ...) do {                      \
    if(cond)                                                  \
    {                                                         \
      printf( fmt, ##__VA_ARGS__);                            \
    }                                                         \
  }                                                           \
  while (0)

#define msh_eprintf(fmt, ...) do {                            \
    fprintf( stderr, fmt, ##__VA_ARGS__ );                    \
  }                                                           \
  while (0)

#define msh_panic_eprintf(fmt, ...) do {                      \
    fprintf( stderr, fmt, ##__VA_ARGS__ );                    \
    exit( EXIT_FAILURE) ;                                     \
  }                                                           \
  while (0)

#define msh_panic_ceprintf(cond, fmt, ...) do {               \
    if(cond)                                                  \
    {                                                         \
      fprintf( stderr, fmt, ##__VA_ARGS__ );                  \
      exit( EXIT_FAILURE );                                   \
    }                                                         \
  }                                                           \
  while (0)


////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory
////////////////////////////////////////////////////////////////////////////////////////////////////

// Look at Niklas Frykholm stuff on allocators to get some idea on when and what allocators are
// relevant
// Maybe start with simple stack allocator --> it is going to be similar to msh_array


////////////////////////////////////////////////////////////////////////////////////////////////////
// Array
//
// Credits
//   Seat T. Barrett - idea of stretchy buffers (?)
//   Per Vognsen     - various improvements from his ion implementation
////////////////////////////////////////////////////////////////////////////////////////////////////
/* TODOs 
  [ ] Prepare docs
  [ ] Replace malloc with jemalloc or TC malloc?
  [ ] What about alignment issues? http://pzemtsov.github.io/2016/11/06/bug-story-alignment-on-x86.html
  [ ] Convert this to a function based implementation with things like array bounds (?)
  [ ] Change behaviour of msh_array_pop
*/

typedef struct msh_array_header
{
  size_t len;
  size_t cap;
} msh_array_hdr_t;

#define msh_array(T) T*

void* msh_array__grow(const void *array, size_t new_len, size_t elem_size);

#define msh_array__grow_formula(x)    ( (2.0*(x))+10 )
#define msh_array__hdr(a)             ( (msh_array_hdr_t *)((char *)(a) - sizeof(msh_array_hdr_t)))

#define msh_array_len(a)              ( (a) ? (msh_array__hdr((a))->len) : 0)
#define msh_array_cap(a)              ( (a) ? (msh_array__hdr((a))->cap) : 0)
#define msh_array_sizeof(a)           ( (a) ? (msh_array__hdr((a))->len * sizeof(*(a))) : 0)
#define msh_array_isempty(a)          ( (a) ? (msh_array__hdr((a))->len <= 0) : 1)

#define msh_array_front(a)            ( (a) ? (a) : NULL)
#define msh_array_end(a)              ( (a) + msh_array_len((a)) ) // One past the end
#define msh_array_back(a)             ( msh_array_len((a)) ? ((a) + msh_array_len((a)) - 1 ) : NULL) // Ptr to last element

#define msh_array_free(a)             ( (a) ? (free(msh_array__hdr(a)), (a) = NULL) :0 )
#define msh_array_pop(a)              ( (a) ? (msh_array__hdr((a))->len--) : 0)
#define msh_array_clear(a)            ( (a) ? (msh_array__hdr((a))->len = 0) : 0)

#define msh_array_fit(a, n)           ( (n) <= msh_array_cap(a) ? (0) : ( *(void**)&(a) = msh_array__grow((a), (n), sizeof(*(a))) )) 
#define msh_array_push(a, ...)        ( msh_array_fit((a), 1 + msh_array_len((a))), (a)[msh_array__hdr(a)->len++] = (__VA_ARGS__))

#define msh_array_cpy( dst, src, n )  ( msh_array_fit( (dst), (n) ), msh_array__hdr((dst))->len = (n), memcpy( (void*)(dst), (void*)(src), (n) * sizeof(*(dst) )))
#define msh_array_printf(b, ...)      ( (b) = msh_array__printf((b), __VA_ARGS__))

////////////////////////////////////////////////////////////////////////////////////////////////////
// Hash Table
//
// Notes: This hashtable cannot use zero as key
//
// Credits
//   Seat T. Barrett: Judy Array vs. Hash-table text
//   Niklas Frykholm: The Machinery Container system blog-series
//   Per Vognsen    : ion open-addressing, linear probing hashtable
////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct msh_map
{
  uint64_t* keys;
  uint64_t* vals;
  size_t _len;
  size_t _cap;
} msh_map_t;

uint64_t  msh_hash_uint64( uint64_t x );
uint64_t  msh_hash_ptr( void *ptr );
uint64_t  msh_hash_str( char *str );

void      msh_map_init( msh_map_t *map, uint32_t cap );
void      msh_map_free( msh_map_t* map );

size_t    msh_map_len( msh_map_t* map );
size_t    msh_map_cap( msh_map_t* map ); 

void      msh_map_insert( msh_map_t* map, uint64_t key, uint64_t val );
uint64_t* msh_map_get( const msh_map_t* map, uint64_t key );

void      msh_map_get_iterable_keys( const msh_map_t* map, uint64_t** keys );
void      msh_map_get_iterable_vals( const msh_map_t* map, uint64_t** vals );
void      msh_map_get_iterable_keys_and_vals( const msh_map_t* map, uint64_t** key, uint64_t** val );

////////////////////////////////////////////////////////////////////////////////////////////////////
// String and path manipulation
////////////////////////////////////////////////////////////////////////////////////////////////////

#if MSH_SYSTEM_WINDOWS
  #define MSH_FILE_SEPARATOR '\\'
#else
  #define MSH_FILE_SEPARATOR '/'
#endif

char* msh_strdup( const char *src );

// inline void
// msh__path_concat(char* buf, va_list ap)
// {
//   // TODO(do cross platform path concatenation using variable arguments)
//   return ;
// }

// inline void
// msh_get_ext()

// inline void
//

////////////////////////////////////////////////////////////////////////////////////////////////////
// Time
// 
// Credits
//   Randy Gaul: based on tinyheaders https://github.com/RandyGaul/tinyheaders
////////////////////////////////////////////////////////////////////////////////////////////////////

enum msh__time_units
{
  MSHT_SECONDS,
  MSHT_MILLISECONDS,
  MSHT_MICROSECONDS,
  MSHT_NANOSECONDS
};

uint64_t msh_time_now();
double msh_time_diff( int32_t unit, uint64_t new_time, uint64_t old_time );

////////////////////////////////////////////////////////////////////////////////////////////////////
// PCG-based random number generation 
//
// Credits:
//   Mattias Gustavsson: internals of pcg seed generator
//   Jonatan Hedborg:    unsigned int to normalized float conversion
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MSH_RND_U32
    #define MSH_RND_U32 uint32_t
#endif

#ifndef MSH_RND_U64
    #define MSH_RND_U64 uint64_t
#endif

typedef struct msh_rand_ctx { 
  MSH_RND_U64 state[ 2 ];
} msh_rand_ctx_t;

void        msh_rand_init( msh_rand_ctx_t* pcg, MSH_RND_U32 seed );
MSH_RND_U32 msh_rand_next( msh_rand_ctx_t* pcg );
float       msh_rand_nextf( msh_rand_ctx_t* pcg );
int         msh_rand_range( msh_rand_ctx_t* pcg, int min, int max );

////////////////////////////////////////////////////////////////////////////////////////////////////
// Maths & stats helpers
//
// Credit:
//    Ginger Bill: Limits from gb.h by 
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MSH_U8_MIN
  #define MSH_U8_MIN 0u
  #define MSH_U8_MAX 0xffu
  #define MSH_I8_MIN (-0x7f - 1)
  #define MSH_I8_MAX 0x7f

  #define MSH_U16_MIN 0u
  #define MSH_U16_MAX 0xffffu
  #define MSH_I16_MIN (-0x7fff - 1)
  #define MSH_I16_MAX 0x7fff

  #define MSH_U32_MIN 0u
  #define MSH_U32_MAX 0xffffffffu
  #define MSH_I32_MIN (-0x7fffffff - 1)
  #define MSH_I32_MAX 0x7fffffff

  #define MSH_U64_MIN 0ull
  #define MSH_U64_MAX 0xffffffffffffffffull
  #define MSH_I64_MIN (-0x7fffffffffffffffll - 1)
  #define MSH_I64_MAX 0x7fffffffffffffffll

  #define MSH_F32_MIN 1.17549435e-38f
  #define MSH_F32_MAX 3.40282347e+38f

  #define MSH_F64_MIN 2.2250738585072014e-308
  #define MSH_F64_MAX 1.7976931348623157e+308
#endif

typedef float real32_t;
typedef double real64_t;

#define MSH_PI          3.1415926535897932384626433832
#define MSH_TWO_PI      6.2831853072
#define MSH_INV_PI      0.3183098862
#define MSH_PI_OVER_TWO 1.5707963268

#define msh_rad2deg(x) ((x) * 180.0 * MSH_INV_PI)
#define msh_deg2rad(x) ((x) * 0.005555555556 * MSH_PI)
#define msh_max(a, b) ((a) > (b) ? (a) : (b))
#define msh_min(a, b) ((a) < (b) ? (a) : (b))
#define msh_max3(a, b, c) msh_max(msh_max((a), (b)), (c))
#define msh_min3(a, b, c) msh_min(msh_min((a), (b)), (c))
#define msh_clamp(x, lower, upper) msh_min( msh_max((x), (lower)), (upper))
#define msh_clamp01(x) msh_clamp((x), 0, 1)
#define msh_iswithin(x, lower, upper) ( ((x) >= (lower)) && ((x) <= (upper)) )
#define msh_abs(x) ((x) < 0 ? -(x) : (x))

static inline int    msh_sqi(int a)    { return a*a; }
static inline float  msh_sqf(float a)  { return a*a; }
static inline double msh_sqd(double a) { return a*a; }

int32_t msh_accumulatei( const int32_t* vals, const size_t n_vals );
float   msh_accumulatef( const float *vals, const size_t n_vals );
float   msh_accumulated( const double *vals, const size_t n_vals );

float   msh_inner_product( const float *vals, const int n_vals );
float   msh_compute_mean( const float *vals, const int n_vals );
float   msh_compute_stddev( float mean, float *vals, int n_vals );

float   msh_gauss_1d( float x, float mu, float sigma );
float   msh_gausspdf_1d( float x, float mu, float sigma );

void    msh_distrib2pdf( const double* dist, double* pdf, size_t n_vals );
void    msh_pdf2cdf( const double* pdf, double* cdf, size_t n_vals );
void    msh_invert_cdf( const double* cdf, size_t n_vals, double* invcdf, size_t n_invcdf_bins );
int     msh_pdfsample_linear( const double* pdf, double prob, size_t n_vals);
int     msh_pdfsample_invcdf( const double* pdf, double prob, size_t n_vals);

typedef struct discrete_distribution_sampler msh_discrete_distrib_t;
void    msh_discrete_distribution_init( msh_discrete_distrib_t* ctx, 
                                        double* weights, size_t n_weights, size_t seed );
void    msh_discrete_distribution_free( msh_discrete_distrib_t* ctx );
int     msh_discrete_distribution_sample( msh_discrete_distrib_t* ctx );


////////////////////////////////////////////////////////////////////////////////////////////////////
// Heap
// TODO:
// [ ] Make generic
////////////////////////////////////////////////////////////////////////////////////////////////////

void msh_heap_make( real32_t* vals, size_t n_vals );
void msh_heap_push( real32_t* vals, size_t n_vals );
void msh_heap_pop(  real32_t* vals, size_t n_vals );
bool msh_heap_isvalid( real32_t* vals, size_t n_vals );

#ifdef __cplusplus
}
#endif

#endif /* MSH */









#ifdef MSH_STD_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////////////////////////
// ARRAY
////////////////////////////////////////////////////////////////////////////////////////////////////


MSHDEF void* 
msh_array__grow(const void *array, size_t new_len, size_t elem_size) {
  size_t old_cap = msh_array_cap( array );
  size_t new_cap = (size_t)msh_array__grow_formula( old_cap );
  new_cap = (size_t)msh_max( new_cap, msh_max(new_len, 16) );
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

MSHDEF char*
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
// HASHTABLE
////////////////////////////////////////////////////////////////////////////////////////////////////
uint64_t 
msh_hash_uint64( uint64_t x ) 
{
  x *= 0xff51afd7ed558ccd;
  x ^= x >> 32;
  return x;
}

uint64_t 
msh_hash_ptr( void *ptr ) 
{
  return msh_hash_uint64( (uintptr_t)ptr );
}

uint64_t 
msh_hash_str( char *str ) 
{
  uint64_t x = 0xcbf29ce484222325;
  char *buf = str;
  while( *buf != 0 )
  {
    x ^= *buf;
    x *= 0x100000001b3;
    x ^= x >> 32;
    buf++;
  }
  return x;
}

size_t 
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

void 
msh_map_init( msh_map_t *map, uint32_t cap )
{
  assert( !map->keys && !map->vals );
  cap = msh_map__pow2ceil( cap );
  map->keys = (uint64_t*)calloc( cap, sizeof(uint64_t) );
  map->vals = (uint64_t*)malloc( cap * sizeof(uint64_t) );
  map->_len = 0;
  map->_cap = cap;
}

void 
msh_map__grow( msh_map_t *map, size_t new_cap ) {
  new_cap = msh_max( new_cap, 16 );
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

size_t
msh_map_len( msh_map_t* map )
{
  return map->_len;
}

size_t
msh_map_cap( msh_map_t* map )
{
  return map->_cap;
}

void 
msh_map_insert( msh_map_t* map, uint64_t key, uint64_t val )
{
  // Increment the key, so that key == 0 is valid, even though 0 is marking empty slot.
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

uint64_t* 
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


void 
msh_map_free( msh_map_t* map )
{
  free( map->keys );
  free( map->vals );
  map->_cap = 0;
  map->_len = 0;
}

void
msh_map_get_iterable_keys( const msh_map_t* map, uint64_t** keys )
{
  assert( (*keys) == NULL );
  (*keys) = (uint64_t*)malloc( map->_len * sizeof(uint64_t) );
  size_t j = 0;
  for( size_t i = 0; i < map->_cap; ++i )
  {
    if( !map->keys[i] ) { (*keys)[j++] = map->keys[i] - 1; }
  }
}

void
msh_map_get_iterable_vals( const msh_map_t* map, uint64_t** vals )
{
  assert( (*vals) == NULL );
  (*vals) = (uint64_t*)malloc( map->_len * sizeof(uint64_t) );
  size_t j = 0;
  for( size_t i = 0; i < map->_cap; ++i )
  {
    if( !map->keys[i] ) { (*vals)[j++] = map->vals[i]; }
  }
}

void
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
// ASSERT
////////////////////////////////////////////////////////////////////////////////////////////////////
MSHDEF void 
msh__assert_handler( char const *condition, char const *file, int32_t line, char const *msg ) {
  fprintf( stderr, "%s:%4d: Assert Failure: ", file, line );
  if( condition ) { fprintf( stderr, "`%s` ", condition); }
  if( msg ) 
  {
    fprintf( stderr, "-> %s", msg );
  }
  fprintf( stderr, "\n" );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// STRINGS
////////////////////////////////////////////////////////////////////////////////////////////////////

MSHDEF char* 
msh_strdup( const char *src )
{
  size_t len = strlen( src );
  char* cpy = (char*)malloc( len+1 );
  strncpy( cpy, src, len );
  cpy[len] = 0;
  return cpy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME
////////////////////////////////////////////////////////////////////////////////////////////////////

MSHDEF float 
msh_rand__float_normalized_from_u32( MSH_RND_U32 value )
{
  MSH_RND_U32 exponent = 127;
  MSH_RND_U32 mantissa = value >> 9;
  MSH_RND_U32 result   = ( exponent << 23 ) | mantissa;
  float fresult        = 0.0f;
  memcpy( &fresult, &result, sizeof(float) );
  return fresult - 1.0f;
}

MSHDEF MSH_RND_U32 
msh_rand__murmur3_avalanche32( MSH_RND_U32 h )
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

MSHDEF MSH_RND_U64 
msh_rand__murmur3_avalanche64( MSH_RND_U64 h )
{
  h ^= h >> 33;
  h *= 0xff51afd7ed558ccd;
  h ^= h >> 33;
  h *= 0xc4ceb9fe1a85ec53;
  h ^= h >> 33;
  return h;
}

void 
msh_rand_init( msh_rand_ctx_t* pcg, MSH_RND_U32 seed )
{
  MSH_RND_U64 value = ( ( (MSH_RND_U64) seed ) << 1ULL ) | 1ULL;
  value = msh_rand__murmur3_avalanche64( value );
  pcg->state[ 0 ] = 0U;
  pcg->state[ 1 ] = ( value << 1ULL ) | 1ULL;
  msh_rand_next( pcg );
  pcg->state[ 0 ] += msh_rand__murmur3_avalanche64( value );
  msh_rand_next( pcg );
}


MSH_RND_U32 
msh_rand_next( msh_rand_ctx_t* pcg )
{
  MSH_RND_U64 oldstate = pcg->state[ 0 ];
  pcg->state[ 0 ] = oldstate * 0x5851f42d4c957f2dULL + pcg->state[ 1 ];
  MSH_RND_U32 xorshifted = (MSH_RND_U32)( ( ( oldstate >> 18ULL)  ^ oldstate ) >> 27ULL );
  MSH_RND_U32 rot = (MSH_RND_U32)( oldstate >> 59ULL );
  return ( xorshifted >> rot ) | ( xorshifted << ( ( -(int) rot ) & 31 ) );
}

float
msh_rand_nextf( msh_rand_ctx_t* pcg )
{
  return msh_rand__float_normalized_from_u32( msh_rand_next( pcg ) );
}

int32_t 
msh_rand_range( msh_rand_ctx_t* pcg, int32_t min, int32_t max )
{
  int32_t const range = ( max - min ) + 1;
  if( range <= 0 ) return min;
  int32_t const value = (int32_t)(msh_rand_nextf( pcg ) * range);
  return min + value; 
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME
////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(maciej): This time measurement might actually be bad. Model it after sokol_time.h

double 
msh_time_diff( int32_t unit, uint64_t new_time, uint64_t old_time )
{
  uint64_t diff = new_time - old_time;
  switch(unit)
  {
    case MSHT_SECONDS:      return (double)(diff * 1e-9);
    case MSHT_MILLISECONDS: return (double)(diff * 1e-6);
    case MSHT_MICROSECONDS: return (double)(diff * 1e-3);
    case MSHT_NANOSECONDS:  return (double)(diff);
  }
  return(double)diff;
}

double
msh_time_diff_sec( uint64_t t2, uint64_t t1 )
{
  return msh_time_diff( MSHT_SECONDS, t2, t1 );
}

double
msh_time_diff_ms( uint64_t t2, uint64_t t1 )
{
  return msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
}

double
msh_time_diff_us( uint64_t t2, uint64_t t1 )
{
  return msh_time_diff( MSHT_MICROSECONDS, t2, t1 );
}

double
msh_time_diff_ns( uint64_t t2, uint64_t t1 )
{
  return msh_time_diff( MSHT_NANOSECONDS, t2, t1 );
}

#if defined(_WIN32)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

uint64_t 
msh_time_now()
{
  static int first = 1;
  static LARGE_INTEGER freq;
  LARGE_INTEGER now;
  
  QueryPerformanceCounter(&now);
  if(first) { first = 0; QueryPerformanceFrequency(&freq);}
  return ((now.QuadPart * 1000000000) / freq.QuadPart);
}
#elif defined(__unix__)

#include <time.h>
uint64_t
msh_time_now()
{
  struct timespec now;
  clock_gettime( CLOCK_MONOTONIC, &now );
  double nano_time = ((now.tv_sec * 1000000000) + now.tv_nsec);
  return nano_time;
}

#elif defined(__APPLE__)

#include <mach/mach_time.h>
uint64_t
msh_time_now()
{
  static int first = 1;
  static uint64_t factor = 0;

  if(first)
  {
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    factor = (info.numer / info.denom);
    first = 0;
  }

  uint64_t nano_time = mach_absolute_time() * factor; 
  return nano_time;
}

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// MATHS
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t
msh_accumulatei( const int32_t* vals, const size_t n_vals )
{
  int32_t accum = 0;
  for( size_t i = 0; i < n_vals; ++i )
  {
    accum += vals[i];
  }
  return accum;
}

float 
msh_accumulatef( const float *vals, const size_t n_vals )
{
  float accum = 0;
  for ( size_t i = 0 ; i < n_vals ; ++i )
  {
    accum += vals[i];
  }
  return accum;
}

float 
msh_accumulated( const double *vals, const size_t n_vals )
{
  double accum = 0;
  for ( size_t i = 0 ; i < n_vals ; ++i )
  {
    accum += vals[i];
  }
  return accum;
}

float 
msh_inner_product( const float *vals, const int n_vals )
{
  float value = 0.0f;
  for ( int i = 0 ; i < n_vals ; ++i )
  {
    value += vals[i] * vals[i];
  }
  return value;
}

float 
msh_compute_mean( const float *vals, const int n_vals )
{
  float sum = msh_accumulatef( vals, n_vals );
  return sum / (float) n_vals;
}

// NOTE(maciej): Temporarily switch sqrtf to sqrt etc., 
//               since tcc does not seem to have correct libm on windows?
float 
msh_compute_stddev( float mean, float *vals, int n_vals )
{
  float sq_sum = msh_inner_product( vals, n_vals );
  return (float)sqrt( sq_sum / (float)n_vals - mean * mean );
}

// TODO(maciej): More analytical distributions in the future, like Poisson etc.
float 
msh_gauss1d( float x, float mu, float sigma )
{
  float exponential = (float)exp( -0.5f * msh_sqf((x-mu)/sigma));
  return exponential;
}

float 
msh_gausspdf1d( float x, float mu, float sigma )
{
  float scale = 1.0f * sigma * sqrt( 2.0f * (float)MSH_PI );
  float exponential = (float)exp(-0.5f * msh_sqf((x-mu)/sigma));
  return scale*exponential;
}

void 
msh_distrib2pdf( const double* dist, double* pdf, size_t n_vals )
{ 
  double sum = msh_accumulated(dist, n_vals);
  if( sum <= 0.00000001 ) return;
  double inv_sum = 1.0 / sum;
  for( size_t i = 0 ; i < n_vals; ++i ) { pdf[i] = (dist[i] * inv_sum); }
}

void
msh_pdf2cdf( const double* pdf, double* cdf, size_t n_vals )
{
  double accum = 0.0;
  for( size_t i = 0; i < n_vals; ++i ) { accum += pdf[i]; cdf[i] = accum; };
}

/* 
 * Alias method, after description and Java implementation by Keith Schwarz:
 * http://www.keithschwarz.com/darts-dice-coins/
 */


struct discrete_distribution_sampler
{
  double* prob;
  int* alias;
  size_t n_weights;
  msh_rand_ctx_t rand_gen;
};

void msh_discrete_distribution_init( msh_discrete_distrib_t* ctx, double* weights, size_t n_weights, size_t seed );
void msh_discrete_distribution_free( msh_discrete_distrib_t* ctx );
int  msh_discrete_distribution_sample( msh_discrete_distrib_t* ctx );

void
msh_discrete_distribution_init( msh_discrete_distrib_t* ctx, double* weights, size_t n_weights, size_t seed )
{
  msh_rand_init( &ctx->rand_gen, seed );
  ctx->n_weights = n_weights;
  ctx->prob  = (double*)malloc( ctx->n_weights * sizeof(double) );
  ctx->alias = (int*)malloc( ctx->n_weights * sizeof(int) );
  
  double *pdf = (double*)malloc( ctx->n_weights * sizeof(double) );
  msh_distrib2pdf(weights, pdf, ctx->n_weights );

  double avg_prob = 1.0 / (double)ctx->n_weights;
  // Do we really need msh array here?
  msh_array(int) small = {0};
  msh_array(int) large = {0};

  for( size_t i = 0; i < ctx->n_weights; ++i )
  {
    if( pdf[i] >= avg_prob ) { msh_array_push(large, i); }
    else                     { msh_array_push(small, i); }
  }

  // Start building the alias table.
  while( !msh_array_isempty(small) && !msh_array_isempty(large) )
  {
    int l = small[ msh_array_len(small)-1 ];
    int g = large[ msh_array_len(large)-1 ];
    msh_array_pop(small);
    msh_array_pop(large);

    ctx->prob[l] = pdf[l] * ctx->n_weights;
    ctx->alias[l] = g;
  
    pdf[g] = (pdf[g] + pdf[l]) - avg_prob;
    if( pdf[g] >= avg_prob ) { msh_array_push( large, g ); }
    else                     { msh_array_push( small, g ); }
  }

  while( !msh_array_isempty(small) )
  {
    int i = small[ msh_array_len(small)-1 ];
    msh_array_pop(small);
    ctx->prob[i] = 1.0;
  }
  while( !msh_array_isempty(large) )
  {
    int i = large[ msh_array_len(large)-1 ];
    msh_array_pop(large);
    ctx->prob[i] = 1.0;
  }

  msh_array_free(small);
  msh_array_free(large);
  free(pdf);
}

void 
msh_discrete_distribution_free( msh_discrete_distrib_t* ctx )
{
  free( ctx->prob );
  free( ctx->alias );
  ctx->n_weights = 0;
}

int 
msh_discrete_distribution_sample( msh_discrete_distrib_t* ctx )
{
  int column = msh_rand_range( &ctx->rand_gen, 0, ctx->n_weights - 1 );
  int coin_toss = msh_rand_nextf( &ctx->rand_gen ) < ctx->prob[column];
  return coin_toss ? column : ctx->alias[column];
}

void
msh_invert_cdf( const double* cdf, size_t n_vals, double* invcdf, size_t n_invcdf_bins )
{
  size_t prev_x = 0;
  for( size_t i = 0 ; i < n_vals; ++i )
  {
    size_t cur_x = (size_t)(cdf[i] * (n_invcdf_bins-1));
    for( size_t x = prev_x; x <= cur_x; ++x )
    {
      invcdf[ x ] = i;
    }
    prev_x = cur_x;
  }
}

int
msh_pdfsample_invcdf( const double* invcdf, double prob, size_t n_vals )
{
  int cdf_idx = prob * n_vals;
  double sample_idx = invcdf[cdf_idx];
  return (int)sample_idx;
}

int
msh_pdfsample_linear( const double* pdf, double prob, size_t n_vals)
{
  size_t sample_idx = 0;
  while ( sample_idx < n_vals && prob > pdf[sample_idx])
  {
    prob -= pdf[sample_idx];
    sample_idx++;
  }
  return sample_idx;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// HEAP
// Modelled after 'Matters Computational' by Joerg Arndt, but interface mirrors stl
////////////////////////////////////////////////////////////////////////////////////////////////////

void msh__heapify( float *vals, size_t vals_len, size_t cur )
{
  size_t max = cur;
  const size_t left  = (cur<<1) + 1;
  const size_t right = (cur<<1) + 2;

  if( (left < vals_len) && (vals[left] > vals[cur]) )   { max = left; }
  if( (right < vals_len) && (vals[right] > vals[max]) ) { max = right; }

  if( max != cur ) // need to swap
  {
    float tmp = vals[cur];
    vals[cur] = vals[max];
    vals[max] = tmp;
    msh__heapify( vals, vals_len, max );
  }
}

void msh_heap_make( real32_t* vals, size_t vals_len )
{
  int64_t i = vals_len >> 1;
  while ( i >= 0 ) { msh__heapify( vals, vals_len, i-- ); }
}

void msh_heap_pop( real32_t* vals, size_t vals_len )
{
  float max = vals[0];
  vals[0] = vals[vals_len-1];
  vals[vals_len-1] = max;
  vals_len--;

  if( vals_len > 0 ){ msh__heapify( vals, vals_len, 0 ); }
}

void msh_heap_push( real32_t* vals, size_t vals_len )
{
  int64_t i = vals_len-1;
  float v = vals[i];

  while( i > 0 )
  {
    int64_t j = (i-1) >> 1;
    if( vals[j] >= v ) break;
    vals[i] = vals[j];
    i = j;
  }
  vals[i] = v;
}

bool msh_heap_isvalid( real32_t* vals, size_t vals_len )
{
  for( int i = vals_len - 1; i > 0; --i )
  {
    size_t parent = (i-1) >> 1;
    if( vals[i] > vals[parent] ) { return false; }
  }
  return true;
}

#endif /*MSH_STD_IMPLEMENTATION*/
