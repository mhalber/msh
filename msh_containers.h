#ifndef MSH_CONTAINERS
#define MSH_CONTAINERS

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

msh_internal size_t
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
  assert( key < (MSH_U64_MAX - 1) );
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

#endif  /* MSH_CONTAINERS_IMPLEMENTATION*/
