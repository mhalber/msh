// TODO(maciej): Test hashtable.h
#define MSH_IMPLEMENTATION
#define HASHTABLE_IMPLEMENTATION
#include "msh.h"
#include "hashtable/hashtable.h"
// #include <vector>
// #include <unordered_map>

///-------------------------------------------------
// STB hashtable
//   hash table implementation
typedef uint32_t uint32;

static int hashword(uint32 c)
{
   // Bob Jenkin's mix function, possibly overkill for only 32 bits?
   // but a simpler one was no faster, so what the heck
   uint32 a,b;
   a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
   a -= b; a -= c; a ^= (c>>13);
   b -= c; b -= a; b ^= (a<<8);
   c -= a; c -= b; c ^= (b>>13);
   a -= b; a -= c; a ^= (c>>12);
   b -= c; b -= a; b ^= (a<<16);
   c -= a; c -= b; c ^= (b>>5);
   a -= b; a -= c; a ^= (c>>3);
   b -= c; b -= a; b ^= (a<<10);
   c -= a; c -= b; c ^= (b>>15);
   return c;
}

#define KEY_NULL 0
#define KEY_DELETED 1

int rehashes;
float hash_table_max_full = 0.65f;
float hash_table_empty = 0.030f;
float hash_delete_rehash = 0.8f;

typedef struct
{
   uint32 key, value;
} Item;

typedef struct
{
   int mask;
   Item *data;
   int has_key[2];
   uint32 value[2];  // handle out of bound values
   int data_len;
   int population, deletes;
   int grow_size, shrink_size, delete_size;  // cache for speed
   void *free_ptr;
} Hash;

#define OVERAGE 1
  // allocate extra because of either weird memory errors in win98
  // or a bug in the hash table code itself

void *hashCreate(int size);
void hashFree(void *hash);
uint32 *hashFind(void *hash, uint32 key);
uint32 *hashInsert(void *hash, uint32 key);
int hashDelete(void *hash, uint32 key);
int hashCount(void *hash);
inline int hashMem(void *hash);

void *hashCreate(int size)
{
   Hash *h = (Hash*)malloc(sizeof(*h));
   if (size < 8) size = 8;
  //  assert(nongreater_power_of_two(size) == size);
   h->data_len = size;
   h->mask = size-1;
   h->population = h->deletes = 0;
   h->has_key[0] = h->has_key[1] = 0;
   h->grow_size = (int) (size * hash_table_max_full) - 1;
   if (h->grow_size < 2) h->grow_size = 2;
   h->shrink_size = (int) (size * hash_table_empty);
   if (h->data_len == 8) h->shrink_size = 0;
   h->delete_size = (int) (size * hash_delete_rehash) - 1;
   h->data = (Item*)calloc(sizeof(h->data[0]), size+OVERAGE*2);
   h->free_ptr = h->data;
   h->data += OVERAGE;
   return h;
}

void hashFree(void *h)
{
   free(((Hash *) h)->free_ptr);
   free(h);
}

uint32 *hashFind(void *hash, uint32 key)
{
   Hash *d = (Hash *) hash;
   Item *z = d->data;
   uint32 mask = d->mask;

   uint32 h = hashword(key);
   uint32 p = h & mask;
   uint32 s;

   if (key <= KEY_DELETED) {
      if (d->has_key[key]) return &d->value[key];
      return NULL;
   }

   if (z[p].key == key) return &z[p].value;
   if (z[p].key == KEY_NULL) return NULL;

   s = (((h >> 16) | (h << 16)) & mask) | 1;
   assert(d->population + d->deletes != d->data_len);
   do {
      p = (p + s) & mask;
      if (z[p].key == key) return &z[p].value;
   } while (z[p].key != KEY_NULL);

   return NULL;
}

static void rehash(Hash *h, int len)
{
   Hash *d = (Hash*)hashCreate(len);
   int i;
   
   ++rehashes;

   for (i=0; i < h->data_len; ++i) {
      if (h->data[i].key > KEY_DELETED) {
         uint32 *p = hashInsert(d, h->data[i].key);
         assert(p != NULL);
         *p = h->data[i].value;
      }
   }
   assert(h->population == d->population);

   free(h->free_ptr);
   for (i=0; i < 2; ++i)
      d->has_key[i] = h->has_key[i],
      d->value[i] = h->value[i];
   memcpy(h, d, sizeof(*h));
   free(d);
}

uint32 *hashInsert(void *hash, uint32 key)
{
   Hash *d = (Hash *) hash;
   Item *z = d->data;
   uint32 mask = d->mask;
   int f = -1;

   uint32 h = hashword(key);
   uint32 p = h & mask;
   uint32 s;

   if (key <= KEY_DELETED) {
      d->has_key[key] = 1;
      return &d->value[key];
   }

   if (z[p].key == key) return &z[p].value;
   if (z[p].key == KEY_DELETED) f = p;
   if (z[p].key > KEY_NULL) {
      s = (((h >> 16) | (h << 16)) & mask) | 1;
      assert(d->population + d->deletes != d->data_len);
      do {
         p = (p + s) & mask;
         if (z[p].key == key) return &z[p].value;
         if (z[p].key == KEY_DELETED && f == -1) f = p;
      } while (z[p].key > KEY_NULL);
   }

   if (z[p].key == KEY_NULL && d->population >= d->grow_size) {
      rehash(d, d->data_len * 2);
      return hashInsert(hash, key);
   } else if (d->population + d->deletes > d->delete_size) {
      rehash(d, d->data_len);
      return hashInsert(hash, key);
   } else {
      if (f >= 0) p = f;
      if (z[p].key == KEY_DELETED) --d->deletes;
      z[p].key = key;
      ++d->population;
      return &z[p].value;
   }
}

int hashDelete(void *hash, uint32 key)
{
   Hash *h = (Hash *) hash;
   Item *p;
   uint32 *value = hashFind(hash, key);
   if (value == NULL) return 0;
   if (key <= KEY_DELETED) {
      h->has_key[key] = 0;
      return 1;
   }
   p = (Item *) (value-1);
   p->key = KEY_DELETED;
   --h->population;
   ++h->deletes;
   if (h->population < h->shrink_size)
      rehash(h, h->data_len >> 1);
   else if (h->population + h->deletes >= h->delete_size)
      rehash(h, h->data_len);
   return 1;
}

int hashCount(void *hash)
{
   Hash *h = (Hash*)hash;
   return h->population + h->has_key[KEY_NULL] + h->has_key[KEY_DELETED];
}

inline int hashMem(void *hash)
{ 
    return sizeof(Hash) + sizeof(Item) * ((Hash *) hash)->data_len;
}
//--------------------------------------------------



void msh_array_test(void) {
    msh_array(int) buf = {0};
    assert(msh_array_len(buf)==0);
    int n = 1024;
    for( int i = 0; i < n; i++ )
    {
      msh_array_push(buf, i);
    }
    assert(msh_array_len(buf) == n);
    for (int32_t i = 0; i < msh_array_len(buf); i++) {
        assert(buf[i] == i);
    }
    msh_array_free(buf);
    assert(buf == NULL);
    assert(msh_array_len(buf) == 0);

    //This is like a string builder!
    char *str = NULL;
    msh_array_printf(str, "One: %d\n", 1);
    assert(strcmp(str, "One: 1\n") == 0);
    msh_array_printf(str, "Hex: 0x%x\n", 0x12345678);
    assert(strcmp(str, "One: 1\nHex: 0x12345678\n") == 0);
}


int 
main( int argc, char** argv )
{
  msh_array_test();
  uint64_t t1, t2;
  int32_t n = pow(2, 19);
  t1 = msh_time_now();
  msh_array(int) buf_a = 0;
  for( int i = 0; i < n; i++ )
  {
    msh_array_push(buf_a, i);
  }
  t2 = msh_time_now();
  printf("time to push %lu elements onto msh_array: %fus\n", msh_array_len(buf_a), msh_time_diff(MSHT_MICROSECONDS, t2, t1));
  for (int32_t i = 0; i < msh_array_len(buf_a); i++) {
    assert(buf_a[i] == i);
  }

  // std::vector<int> buf_b;
  // t1 = msh_time_now();
  // for( int i = 0; i < n; i++ )
  // {
  //   buf_b.push_back(i);
  // }
  // t2 = msh_time_now();
  // printf("time to push %lu elements onto std::vector: %fus\n", buf_b.size(), msh_time_diff(MSHT_MICROSECONDS, t2, t1));
  // for (int32_t i = 0; i < buf_b.size(); i++) {
  //   assert(buf_b[i] == i);
  // }
  ///===============================================================================================
  int m = pow(2, 19);
  uint64_t* keys = (uint64_t*)malloc(m*sizeof(uint64_t));
  uint64_t* vals = (uint64_t*)malloc(m*sizeof(uint64_t));
  for( uint64_t i = 0; i < m; ++i )
  {
    keys[i] = 6*i + rand() % 6;
    vals[i] = rand() % m;
  }

  // std::unordered_map<uint64_t, uint64_t> std_map;
  // t1 = msh_time_now();
  // for( uint64_t i = 0; i < m; ++i )
  // {
  //   std_map.insert( {keys[i], vals[i]} );
  // }
  // t2 = msh_time_now();
  // printf("time to insert %lu elements onto std::unordered_map: %fus\n", std_map.size(), msh_time_diff(MSHT_MICROSECONDS, t2, t1));

  // t1 = msh_time_now();
  // for( uint64_t i = 0; i < std_map.size(); ++i )
  // {
  //   auto search = std_map.find( keys[i] );
  //   assert( search->second == vals[i] );
  // }
  // t2 = msh_time_now();
  // printf("time to find  %lu elements in std::unordered_map: %fus\n", std_map.size(), msh_time_diff(MSHT_MICROSECONDS, t2, t1));

  //==================================
  msh_map_t map = {0};
  t1 = msh_time_now();;
  for( uint64_t i = 0; i < m; ++i )
  {
    msh_map_insert(&map, keys[i], vals[i]);
  }
  t2 = msh_time_now();
  printf("time to insert %lu elements onto msh_map: %fus\n", msh_map_len(&map), msh_time_diff(MSHT_MICROSECONDS, t2, t1));

  t1 = msh_time_now();
  for( uint64_t i = 0; i < m; ++i )
  {
    uint64_t val = *msh_map_get( &map, keys[i] );
    assert( val == vals[i] );
  }
  t2 = msh_time_now();
  printf("time to find %lu elements in msh_map: %fus\n", msh_map_len(&map), msh_time_diff(MSHT_MICROSECONDS, t2, t1));
  //==================================
  
  hashtable_t ht_map = {0};
  hashtable_init( &ht_map, sizeof(uint64_t), 16, NULL );
  t1 = msh_time_now();;
  for( uint64_t i = 0; i < m; ++i )
  {
    hashtable_insert( &ht_map, keys[i], &vals[i]);
  }
  t2 = msh_time_now();
  printf("time to insert %d elements onto hashtable_t: %fus\n", hashtable_count(&ht_map), msh_time_diff(MSHT_MICROSECONDS, t2, t1));

  t1 = msh_time_now();
  for( uint64_t i = 0; i < m; ++i )
  {
    uint64_t* val = (uint64_t*)hashtable_find( &ht_map, keys[i] );
    assert( *val == vals[i] );
  }
  t2 = msh_time_now();
  printf("time to find  %d elements in hashtable_t: %fus\n", hashtable_count(&ht_map), msh_time_diff(MSHT_MICROSECONDS, t2, t1));

  //==================================
  void* stb_map = hashCreate(16);
  t1 = msh_time_now();
  for( uint64_t i = 0; i < m; ++i )
  {
    uint32_t* val = hashInsert( stb_map, (uint32_t)(keys[i]) );
    *val = (uint32_t)vals[i];
  }
  t2 = msh_time_now();
  printf("time to insert %d elements onto stb_map: %fus\n", hashCount(stb_map), msh_time_diff(MSHT_MICROSECONDS, t2, t1));

  t1 = msh_time_now();
  for( uint64_t i = 0; i < m; ++i )
  {
    uint32_t* val = hashFind( stb_map, (uint64_t)(keys[i]) );
    assert( *val ==  (uint32_t)vals[i] );
  }
  t2 = msh_time_now();
  printf("time to find  %d elements in stb_map: %fus\n", hashCount(stb_map), msh_time_diff(MSHT_MICROSECONDS, t2, t1));

  return 0;
  


}

