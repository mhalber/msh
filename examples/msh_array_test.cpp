#define MSH_IMPLEMENTATION
#include "msh.h"
#include <vector>


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
    msh_array_sprintf(str, "One: %d\n", 1);
    assert(strcmp(str, "One: 1\n") == 0);
    msh_array_sprintf(str, "Hex: 0x%x\n", 0x12345678);
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
  printf("time to push %d elements onto msh_array: %fus\n", msh_array_len(buf_a), msh_time_diff(MSHT_MICROSECONDS, t2, t1));
  for (int32_t i = 0; i < msh_array_len(buf_a); i++) {
    assert(buf_a[i] == i);
  }

  std::vector<int> buf_b;
  t1 = msh_time_now();
  for( int i = 0; i < n; i++ )
  {
    buf_b.push_back(i);
  }
  t2 = msh_time_now();
  printf("time to push %lu elements onto msh_array: %fus\n", buf_b.size(), msh_time_diff(MSHT_MICROSECONDS, t2, t1));
  for (int32_t i = 0; i < buf_b.size(); i++) {
    assert(buf_b[i] == i);
  }
  return 0;
}