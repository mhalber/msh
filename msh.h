/*
  ==============================================================================
  
  MSH_STB.H 
  
  A single header library for some standard library functionality, that is not
  present in C. This file is partially written by myself, but also includes a lot
  of code copied/modified from other libraries. Please see credits for details.

  To use the library you simply add:
  
  #define MSH_IMPLEMENTATION
  #include "msh_math.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh.h"

  All functions can be made static by definining:

  #ifdef MSH_STATIC

  before including the "msh.h"

  ==============================================================================
  DEPENDENCIES

    This file includes some c stdlib headers.

  ==============================================================================
  AUTHORS:
    Maciej Halber

  CREDITS:
    Dynamic array based on                   stb.h      by Sean T. Barrett
    Random number generation based on        rnd.h      by Mattias Gustavsson
    Time measurements based on               tinytime.h by Randy Gaul
    Assert handling based on                 gb.h       by Ginger Bill

  ==============================================================================
  TODOs:
  [x] Dynamic array (std::vector replacement)
  [ ] Separate into header / implementation
  [ ] Add switch flags
  [ ] Hashtable
  [x] Static keyword disentanglement
  [x] Macros
  [ ] Change some macros to inline functions (force inline trickery is required)
  [ ] Bit operations
  [ ] Queue / Stack
  [ ] Custom prints
  [x] Asserts
     [ ] Enable turning assertions at compile time
  [ ] Memory allocation
  [ ] Sorting and Searching
  [ ] Math constants? (Maybe should create msh_math.h)
  [ ] Multithreading / Scheduling

  ==============================================================================
  REFERENCES:
  [1] stb.h           https://github.com/nothings/stb/blob/master/stb.h
  [2] gb.h            https://github.com/gingerBill/gb/blob/master/gb.h
  [3] stretchy_buffer https://github.com/nothings/stb/blob/master/stretchy_buffer.h
  [4] tinyheaders     https://github.com/RandyGaul/tinyheaders
  [5] gb.h            https://github.com/gingerBill/gb
*/

#ifndef MSH
#define MSH

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MSH_NO_HEADERS

// c stdlib
#include <assert.h>
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

// system specific
#ifdef __linux__ 
#include <sys/stat.h>
#endif

#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// Useful macros
////////////////////////////////////////////////////////////////////////////////////////////////////
#define MSH_SIZE_OF(x) (int64_t)(sizeof(x))
#define MSH_COUNT_OF(x) ( ( MSH_SIZE_OF(x) / MSH_SIZE_OF( *x ) ) )
#define MSH_OFFSET_OF(Type, element) ((int64_t)&(((Type *)NULL)->element))

#ifdef MSH_STATIC
#define MSHDEF static
#else
#define MSHDEF extern
#endif

#define msh_local_persitent static // Local variables with persisting values
#define msh_global          static // Global variables
#define msh_internal        static // Internal linkage


////////////////////////////////////////////////////////////////////////////////////////////////////
// Maths & stats helpers
////////////////////////////////////////////////////////////////////////////////////////////////////

#define MSH_PI          3.1415926535897932384626433832
#define MSH_TWO_PI      6.2831853072
#define MSH_INV_PI      0.3183098862
#define MSH_PI_OVER_TWO 1.5707963268

#define msh_rad2deg(x) ((x) * 180.0 * MSH_INV_PI)
#define msh_deg2rad(x) ((x) * 0.005555555556 * MSH_PI)
#define msh_max(a, b) ((a) > (b) ? (a) : (b))
#define msh_min(a, b) ((a) < (b) ? (a) : (b))
#define msh_max3(a, b, c) msh_max(msh_max(a, b), c)
#define msh_min3(a, b, c) msh_min(msh_min(a,b), c)
#define msh_clamp(x, lower, upper) msh_min( msh_max((x), (lower)), (upper))
#define msh_clamp01(x) msh_clamp((x), 0, 1)
#define msh_within(x, lower, upper) ( ((x) >= (lower)) && ((x) <= (upper)) )
#define msh_abs(x) ((x) < 0 ? -(x) : (x))

static inline int    msh_sqi(int a)    { return a*a; }
static inline float  msh_sqf(float a)  { return a*a; }
static inline double msh_sqd(double a) { return a*a; }

int32_t msh_accumulatei( const int32_t* vals, const int32_t n_vals );
float msh_accumulatef( const float *vals, const int32_t n_vals );
float msh_inner_product( const float *vals, const int n_vals );
float msh_compute_mean( const float *vals, const int n_vals );
float msh_compute_stddev( float mean, float *vals, int n_vals );
float msh_gauss1d( float x, float mu, float sigma );
void  msh_distrib2pdf( const float* dist, float* pdf, int n_vals );
void  msh_pdf2cdf( const float* pdf, float* cdf, int n_vals );
void  msh_invert_cdf( const float* cdf, float* invcdf, int n_vals);
float msh_pdfsample( const float* pdf, float prob, int n_vals);
float msh_gausspdf1d( float x, float mu, float sigma );


////////////////////////////////////////////////////////////////////////////////////////////////////
// Printing
////////////////////////////////////////////////////////////////////////////////////////////////////

#define msh_cprintf(cond, fmt, ...) do {                      \
    if(cond)                                                  \
    {                                                         \
      printf (fmt, ##__VA_ARGS__);                            \
    }                                                         \
  }                                                           \
  while (0)

#define msh_eprintf(fmt, ...) do {                            \
    fprintf(stderr, fmt, ##__VA_ARGS__);                      \
  }                                                           \
  while (0)
#define msh_panic_eprintf(fmt, ...) do {                      \
    fprintf(stderr, fmt, ##__VA_ARGS__);                      \
    exit(EXIT_FAILURE);                                       \
  }                                                           \
  while (0)
#define msh_panic_ceprintf(cond, fmt, ...) do {               \
    if(cond)                                                  \
    {                                                         \
      fprintf(stderr, fmt, ##__VA_ARGS__);                    \
      exit(EXIT_FAILURE);                                     \
    }                                                         \
  }                                                           \
  while (0)


////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug
//
// Credit
//  This is taken from gb.h by Ginger Bill.
////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(_MSC_VER)
  #if _MSC_VER < 1300
    #define MSH_DEBUG_TRAP() __asm int 3 /* Trap to debugger! */
  #else
    #define MSH_DEBUG_TRAP() __debugbreak()
  #endif
#else
  #define MSH_DEBUG_TRAP() __builtin_trap()
#endif

#ifndef MSH_NDEBUG
  #define MSH_ASSERT_MSG(cond, msg) do {                                         \
    if (!(cond)) {                                                               \
      msh__assert_handler(#cond, __FILE__, (int64_t)__LINE__, msg );             \
      MSH_DEBUG_TRAP();                                                          \
    }                                                                            \
  } while (0)
#else
  #define MSH_ASSERT_MSG(cond, msg) /* Expands to nothing */
#endif /*MSH_NDEBUG*/


#define MSH_ASSERT(cond) MSH_ASSERT_MSG(cond, NULL)

#define MSH_ASSERT_NOT_NULL(ptr) MSH_ASSERT_MSG((ptr) != NULL, #ptr " must not be NULL")

#define MSH_PANIC(msg, ...) MSH_ASSERT_MSG(0, msg, ##__VA_ARGS__)

void msh__assert_handler( char const *condition, char const *file, int32_t line, char const *msg );

////////////////////////////////////////////////////////////////////////////////////////////////////
// Array
////////////////////////////////////////////////////////////////////////////////////////////////////
// TODO(maciej): Test bug with m_back / last
// TODO(maciej): Should these be changed to functions instead of macros?
// TODO(maciej): Small array optimization
// TODO(maciej): Prepare Better docs
// TODO(maciej): Test efficiency against std::vector/regular arrays. Investigate what might be 
//               causing the slowdown. 

// msh_array_init(a, n)     // <- Initialize array of size n with unitialized values
// msh_array_grow(a, n)     // <- Grow array to size n
// msh_array_push(a, v)     // <- Push value v onto array 
// msh_array_pop(a)         // <- Pop value from array
// msh_array_back(a)        // <- Access the last element
// msh_array_count(a)       // <- Size of an array
// msh_array_size(a)        // <- Size of an array
// msh_array_capacity(a)    // <- Capacity of an array
// msh_array_clear(a)       // <- Clear elements but leave array allocated
// msh_array_free(a)        // <- Free memory

typedef struct msh_array_header
{
  int32_t count;
  int32_t capacity;
} msh_array_header_t;

#define msh_array(T) T*

#define MSH_ARRAY_GROW_FORMULA(x) ( 1.5*(x) + 2 )

#define msh__array_header(a)     ((msh_array_header_t*)(a) - 1)
#define msh_array_count(a)       ((a) ? msh__array_header(a)->count : 0)
#define msh_array_size(a)        ((a) ? msh__array_header(a)->count : 0)
#define msh_array_capacity(a)    ((a) ? msh__array_header(a)->capacity : 0)
#define msh_array_empty(a)       ((a) ? (msh__array_header(a)->count <= 0) : 0)
#define msh_array_front(a)       ((a) ? &a[0] : NULL)
#define msh_array_back(a)        ((a) ? &a[((msh__array_header(a)->count) - 1)] : NULL)

#define msh_array_init(a, n) do                                                \
{                                                                              \
  MSH_ASSERT(a == NULL);                                                       \
  msh_array_header_t *msh__ah =                                                \
   (msh_array_header_t *)malloc(sizeof(msh_array_header_t) + sizeof(*(a)) * n);\
  msh__ah->capacity = n;                                                       \
  msh__ah->count = 0;                                                          \
  void** msh__array = (void**)&(a);                                            \
  (*msh__array) = (void*)(msh__ah+1);                                          \
} while( 0 )

#define msh_array_free(a) do                                                   \
{                                                                              \
  MSH_ASSERT( a!=NULL );                                                       \
  msh_array_header_t *msh__ah = msh__array_header(a);                          \
  free( msh__ah );                                                             \
  a = NULL;                                                                    \
} while( 0 )

#define msh__array_grow(a) do                                                  \
{                                                                              \
  int32_t new_capacity=(int32_t)MSH_ARRAY_GROW_FORMULA(msh_array_capacity(a)); \
  void** msh__array = (void**)&(a);                                            \
  (*msh__array) = msh__array_reserve( (void*)a, new_capacity, sizeof(*(a)) );  \
} while( 0 )

#define msh_array_reserve( a, n ) do                                  \
{                                                                     \
  if( msh__array_header(a)->capacity < n )                            \
  {                                                                   \
  void** msh__array = (void**)&(a);                                   \
  (*msh__array) = msh__array_reserve( (void*)a, n, sizeof(*(a)) );    \
  }                                                                   \
} while( 0 )

#define msh_array_push( a, v ) do \
{ \
  if( !a || msh__array_header(a)->capacity < msh__array_header(a)->count + 1 )     \
  { \
    msh__array_grow(a); \
  }\
  (a)[msh__array_header(a)->count++] = v; \
} while( 0 )

#define msh_array_pop(a)   do\
{ \
  MSH_ASSERT(a!=NULL); \
  MSH_ASSERT(msh__array_header(a)->count > 0); \
  msh__array_header(a)->count--; \
} while (0)

#define msh_array_clear(a) do \
{\
  msh__array_header(a)->count = 0; \
} while (0)

#define msh_array_copy( dest, src, n ) do \
{ \
  if( !dest || msh__array_header(dest)->capacity < n ) \
  { \
    void** msh__array = (void**)&(dest);                                \
    (*msh__array) = msh__array_reserve( (void*)dest, n, sizeof(*(dest)) );   \
  } \
  memcpy( (void*)dest, (void*)src, n*sizeof(*(dest)) ); \
  msh__array_header(dest)->count = n; \
} while(0)

MSHDEF void* msh__array_reserve( void* array, int32_t capacity, int32_t item_size );


////////////////////////////////////////////////////////////////////////////////////////////////////
// String and path manipulation
////////////////////////////////////////////////////////////////////////////////////////////////////

char* msh_strdup(const char *src);

// inline void
// msh__asprintf(char *str)
// {
//   // TODO(check book for that)
//   return;
// }

// inline void
// msh__path_concat(char* buf, va_list ap)
// {
//   // TODO(do cross platform path concatenation using variable arguments)
//   return ;
// }

////////////////////////////////////////////////////////////////////////////////
// Time
// 
// Credits
//   Based on Randy Gauls tinyheaders https://github.com/RandyGaul/tinyheaders
////////////////////////////////////////////////////////////////////////////////

enum msh__time_units
{
  MSHT_SECONDS,
  MSHT_MILLISECONDS,
  MSHT_MICROSECONDS,
  MSHT_NANOSECONDS
};

uint64_t msh_time_now();
double msh_time_diff(int32_t unit, uint64_t new_time, uint64_t old_time);

////////////////////////////////////////////////////////////////////////////////////////////////////
// PCG-based random number generation 
//
// Credits:
//   Mattias Gustavsson(internals of pcg seed generator)
//   Jonatan Hedborg: unsigned int to normalized float conversion
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef MSH_RND_U32
    #define MSH_RND_U32 unsigned int
#endif
#ifndef MSH_RND_U64
    #define MSH_RND_U64 unsigned long long
#endif

typedef struct msh_rand_ctx { 
  MSH_RND_U64 state[ 2 ]; 
} msh_rand_ctx_t;

void        msh_rand_init( msh_rand_ctx_t* pcg, MSH_RND_U32 seed );
MSH_RND_U32 msh_rand_next( msh_rand_ctx_t* pcg );
float       msh_rand_nextf( msh_rand_ctx_t* pcg );
int         msh_rand_range( msh_rand_ctx_t* pcg, int min, int max );

#ifdef __cplusplus
}
#endif

#endif /* MSH */









#ifdef MSH_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////////////////////////
// ARRAY
////////////////////////////////////////////////////////////////////////////////////////////////////
MSHDEF void*
msh__array_reserve( void* array, int32_t capacity, int32_t item_size )
{
  MSH_ASSERT( item_size > 0 );

  msh_array_header_t * ah = msh__array_header( array );
  if( array && capacity == ah->capacity ) 
  { 
    return array;
  }

  int32_t prev_count = array ? ah->count : 0;
  int32_t new_size = item_size * capacity + sizeof(msh_array_header_t);
  void *p = (void*)realloc( (array ? ah : 0), new_size );
  MSH_ASSERT(p);

  ah = (msh_array_header_t*)p;
  ah->capacity = capacity;
  ah->count    = prev_count;
  if( ah->capacity < ah->count ) ah->count = ah->capacity;
  
  return (void*)(ah + 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// ASSERT
////////////////////////////////////////////////////////////////////////////////////////////////////
void msh__assert_handler( char const *condition, char const *file, 
                         int32_t line, char const *msg ) {
  fprintf( stderr, "%s:%4d: Assert Failure: ", file, line );
  if( condition ) fprintf( stderr, "`%s` ", condition);
  if( msg ) 
  {
    fprintf( stderr, "-> %s", msg );
  }
  fprintf(stderr, "\n");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// STRINGS
////////////////////////////////////////////////////////////////////////////////////////////////////

char* 
msh_strdup(const char *src)
{
  size_t len = strlen(src);
  char* cpy = (char*)malloc(len+1);
  strncpy(cpy, src, len);
  cpy[len] = 0;
  return cpy;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME
////////////////////////////////////////////////////////////////////////////////////////////////////

// Convert a randomized MSH_RND_U32 value to a float value x in the range 0.0f <= x < 1.0f. Contributed by Jonatan Hedborg
MSHDEF float 
msh_rand__float_normalized_from_u32( MSH_RND_U32 value )
{
  MSH_RND_U32 exponent = 127;
  MSH_RND_U32 mantissa = value >> 9;
  MSH_RND_U32 result   = ( exponent << 23 ) | mantissa;
  float fresult        = 0.0f;
  memcpy(&fresult, &result, sizeof(float));
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


int 
msh_rand_range( msh_rand_ctx_t* pcg, int min, int max )
{
  int const range = ( max - min ) + 1;
  if( range <= 0 ) return min;
  int const value = (int) ( msh_rand_nextf( pcg ) * range );
  return min + value; 
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// TIME
////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE(maciej): This time measurement might actually be bad. Model it after sokol_time.h

double msh_time_diff( int32_t unit, uint64_t new_time, uint64_t old_time )
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

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

uint64_t msh_time_now()
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
uint64_t msh_time_now()
{
  struct timespec now;
  clock_gettime( CLOCK_MONOTONIC, &now );
  double nano_time = ((now.tv_sec * 1000000000) + now.tv_nsec);
  return nano_time;
}

#elif defined(__APPLE__)

#include <mach/mach_time.h>
uint64_t msh_time_now()
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

inline int32_t
msh_accumulatei( const int32_t* vals, const int32_t n_vals )
{
  int32_t accum = 0;
  for( int32_t i = 0; i < n_vals; ++i )
  {
    accum += vals[i];
  }
  return accum;
}

float 
msh_accumulatef( const float *vals, const int32_t n_vals )
{
  float accum = 0;
  for ( int32_t i = 0 ; i < n_vals ; ++i )
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

float 
msh_compute_stddev( float mean, float *vals, int n_vals )
{
  float sq_sum = msh_inner_product( vals, n_vals );
  return sqrtf( sq_sum / (float)n_vals - mean * mean );
}

float 
msh_gauss1d( float x, float mu, float sigma )
{
  float exponential = expf(-0.5f * msh_sqf((x-mu)/sigma));
  return exponential;
}


void 
msh_distrib2pdf( const float* dist, float* pdf, int n_vals )
{ 
  float sum = msh_accumulatef(dist, n_vals);
  if( sum <= 0.00000001f ) return;
  float inv_sum = 1.0f / sum;
  for( int32_t i = 0 ; i < n_vals; ++i ) { pdf[i] = (dist[i] * inv_sum); }
}

void
msh_pdf2cdf( const float* pdf, float* cdf, int n_vals )
{
  float accum = 0.0;
  for( int32_t i = 0; i < n_vals; ++i ) { accum += pdf[i]; cdf[i] = accum;  };
}

// void
// msh_invert_cdf( const float* cdf, float* invcdf, int n_vals)
// {
  // int prev_idx = 0;
  // for(int i = 0 ; i < n_vals; ++i)
  // {
  //   float likelihood = cdf[i];
  //   int idx = (int)floorf(likelihood*(n_vals)); 
  //   for( int j = prev_idx; j < idx; j++ )
  //   {
  //     invcdf[j] = (float)floorf(i);
  //   }
  //   prev_idx = idx;
  // }
// }


float
msh_pdfsample( const float* pdf, float prob, int n_vals)
{
  int sample_idx = 0;
  while ( sample_idx < n_vals && prob > pdf[sample_idx])
  {
    prob -= pdf[sample_idx];
    sample_idx++;
  }
  return (float)sample_idx;
}

float 
msh_gausspdf1d( float x, float mu, float sigma )
{
  float scale = 1.0f * sigma * sqrtf(2.0f*MSH_PI);
  float exponential = expf(-0.5f * msh_sqf((x-mu)/sigma));
  return scale*exponential;
}


#endif