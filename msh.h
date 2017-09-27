/*
  ==============================================================================
  
  MSH.H 
  
  A single header library for some standard library functionality, sadly not
  present in C. This file is very strongly based on Sean T. Barrets stb.h and
  Ginger's Bill gb.h files. I simply wanted to create mine for educational
  purposes and to have something that meshes well with the rest of my code.

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

  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)
  ==============================================================================
  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  ==============================================================================
  TODOs:
  [x] Dynamic array (std::vector replacement)
  [ ] Hashtable
  [x] Static keyword disentanglement
  [x] Macros
  [ ] Bit operations
  [ ] Queue / Stack
  [ ] Custom prints
  [x] Asserts
     [ ] Enable turning assertions of
  [ ] Memory allocation
  [ ] Sorting and Searching
  [ ] Math constants? (Maybe should create msh_math.h)
  [ ] Multithreading

  ==============================================================================
  REFERENCES:
  [1] stb.h           https://github.com/nothings/stb/blob/master/stb.h
  [2] gb.h            https://github.com/gingerBill/gb/blob/master/gb.h
  [3] stretchy_buffer https://github.com/nothings/stb/blob/master/stretchy_buffer.h
*/

#ifndef MSH
#define MSH

#ifndef MSH_NO_C_HEADERS
#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#endif


////////////////////////////////////////////////////////////////////////////////
// Useful constants and macros
////////////////////////////////////////////////////////////////////////////////
#ifndef MSH_PI
#define MSH_PI          3.1415926535897932384626433832
#define MSH_TWO_PI      6.2831853072
#define MSH_INV_PI      0.3183098862
#define MSH_PI_OVER_TWO 1.5707963268
#endif


#ifndef msh_rad2deg
#define msh_rad2deg(x) ((x) * 180.0 * MSH_INV_PI)
#endif

#ifndef msh_deg2rad
#define msh_deg2rad(x) ((x) * 0.005555555556 * MSH_PI)
#endif

#ifndef msh_size_of
#define msh_size_of(x) (int64_t)(sizeof(x))
#endif

#ifndef msh_count_of
#define msh_count_of(x) ( ( msh_size_of(x) / msh_size_of( *x ) ) )
#endif

#ifndef msh_offset_of
#define msh_offset_of(Type, element) ((int64_t)&(((Type *)NULL)->element))
#endif

#ifndef msh_max 
#define msh_max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef msh_min 
#define msh_min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef msh_max3
#define msh_max3(a, b, c) msh_max(msh_max(a, b), c)
#endif

#ifndef msh_min3
#define msh_min3(a, b, c) msh_min(msh_min(a,b), c)
#endif

#ifndef msh_clamp
#define msh_clamp(x, lower, upper) msh_min( msh_max((x), (lower)), (upper))
#endif

#ifndef msh_clamp01
#define msh_clamp01(x) msh_clamp((x), 0, 1)
#endif

#ifndef msh_within
#define msh_within(x, lower, upper) ( ((x) >= (lower)) && ((x) <= (upper)) )
#endif

#ifndef msh_abs
#define msh_abs(x) ((x) < 0 ? -(x) : (x))
#endif

////////////////////////////////////////////////////////////////////////////////
// Static keyword
////////////////////////////////////////////////////////////////////////////////
#ifndef msh_local_persist
#define msh_local_persitent static // Local variables with persisting values
#define msh_global          static // Global variables
#define msh_internal        static // Internal linkage
#endif

#ifdef MSH_STATIC
#define MSHDEF static
#else
#define MSHDEF extern
#endif

////////////////////////////////////////////////////////////////////////////////
// Printing
////////////////////////////////////////////////////////////////////////////////

#define msh_cprintf(cond, fmt, ...) do { if(cond){ printf (fmt, ##__VA_ARGS__);} } while (0)
#define msh_eprintf(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); } while (0)
#define msh_panic_eprintf(fmt, ...) do { fprintf(stderr, fmt, ##__VA_ARGS__); exit(EXIT_FAILURE); } while (0)
#define msh_panic_ceprintf(cond, fmt, ...) do { if(cond){ fprintf(stderr, fmt, ##__VA_ARGS__); exit(EXIT_FAILURE);} } while (0)

////////////////////////////////////////////////////////////////////////////////
// Debug
////////////////////////////////////////////////////////////////////////////////

// NOTE(maciej): This is shameslessly ripped out from gb.h
#ifndef MSH_DEBUG_TRAP
  #if defined(_MSC_VER)
    #if _MSC_VER < 1300
      #define MSH_DEBUG_TRAP() __asm int 3 /* Trap to debugger! */
    #else
      #define MSH_DEBUG_TRAP() __debugbreak()
    #endif
  #else
    #define MSH_DEBUG_TRAP() __builtin_trap()
  #endif
#endif

// NOTE(maciej): Extend this when we get custom printfs.
#ifndef MSH_ASSERT_MSG
#define MSH_ASSERT_MSG(cond, msg) do {                                         \
  if (!(cond)) {                                                               \
    msh__assert_handler(#cond, __FILE__, (int64_t)__LINE__, msg );             \
    MSH_DEBUG_TRAP();                                                          \
  }                                                                            \
} while (0)
#endif

#ifndef MSH_ASSERT
#define MSH_ASSERT(cond) MSH_ASSERT_MSG(cond, NULL)
#endif

#ifndef MSH_ASSERT_NOT_NULL
#define MSH_ASSERT_NOT_NULL(ptr) MSH_ASSERT_MSG((ptr) != NULL,                   \
                                                       #ptr " must not be NULL")
#endif

// NOTE(bill): Things that shouldn't happen with a message!
#ifndef MSH_PANIC
#define MSH_PANIC(msg, ...) MSH_ASSERT_MSG(0, msg, ##__VA_ARGS__)
#endif


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

////////////////////////////////////////////////////////////////////////////////
// Array
////////////////////////////////////////////////////////////////////////////////

// TODO(maciej): Are we messing up alignement?
// TODO(maciej): Better docs
// msh_array_init(a, n)     // <- Initialize array of size n with unitialized values
// msh_array_grow(a, n)     // <- Grow array to size n
// msh_array_push(a, v)     // <- Push value v onto array 
// msh_array_pop(a)         // <- Pop value from array
// msh_array_last(a)        // <- Access the last element
// msh_array_count(a)       // <- Size of an array
// msh_array_size(a)       // <- Size of an array
// msh_array_capacity(a)    // <- Capacity of an array
// msh_array_clear(a)       // <- Clear elements but leave array allocated
// msh_array_free(a)        // <- Free memory

typedef struct msh_array_header
{
  int32_t count;
  int32_t capacity;
} msh_array_header_t;

#define msh_array(T) T *

#ifndef MSH_ARRAY_GROW_FORMULA
#define MSH_ARRAY_GROW_FORMULA(x) ( 1.5*(x) + 2 )
#endif

#define msh__array_header(a)     ((msh_array_header_t*)(a) - 1)
#define msh_array_count(a)       ((a) ? msh__array_header(a)->count : 0)
#define msh_array_size(a)        ((a) ? msh__array_header(a)->count : 0)
#define msh_array_capacity(a)    ((a) ? msh__array_header(a)->capacity : 0)
#define msh_array_empty(a)       ((a) ? (msh__array_header(a)->count > 0) : 0)
#define msh_array_front(a)       ((a) ? &a[0] : NULL)
#define msh_array_back(a)        ((a) ? &a[msh__array_header(a)->count - 1] : NULL)

#define msh_array_init(a, n) do                                                \
{                                                                              \
  MSH_ASSERT(!a);                                                              \
  msh_array_header_t *msh__ah =                                                \
   (msh_array_header_t *)malloc(sizeof(msh_array_header_t) + sizeof(*(a)) * n);\
  msh__ah->capacity = n;                                                       \
  msh__ah->count = 0;                                                          \
  (a) = (void*)(msh__ah+1);                                                    \
} while( 0 )

#define msh_array_free(a) do                                                   \
{                                                                              \
  MSH_ASSERT( a!=NULL ); \
  msh_array_header_t *msh__ah = msh__array_header(a);                          \
  free( msh__ah );                                                             \
  a = NULL;                                                                    \
} while( 0 )

#define msh__array_grow(a) do                                                  \
{                                                                              \
  int64_t new_capacity = MSH_ARRAY_GROW_FORMULA( msh_array_capacity(a) );      \
  void** msh__array = (void**)&(a);                                            \
  *msh__array = msh__array_reserve( (void*)a, new_capacity, sizeof(*(a)) );    \
} while( 0 )

#define msh_array_reserve( a, n ) do \
{ \
  (a) = msh__array_reserve( (void*)a, n, sizeof(*(a)) ); \
} while( 0 )

#define msh_array_resize( a, n ) do                                            \
{                                                                              \
  void** msh__array = (void**)&(a);                                            \
  *msh_array = msh__array_reserve( (void*)a, n, sizeof(*(a)) );                \
  msh__array_header(a)->count = n; \
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


#endif /* MSH */

#ifdef MSH_IMPLEMENTATION

#endif