
#include <stdio.h>
#include "generic.h"

int
main()
{
  mh_vec3f_t a = mh_vec3f_init( 1, 2, 3 );
  mh_vec3d_t b = mh_vec3d_init( 1, 2, 3 );
  mh_vec3i_t c = mh_vec3i_ones();
  printf("%f %f %f\n", a.x, a.y, a.z );
  printf("%f %f %f\n", b.x, b.y, b.z );
  printf("%d %d %d\n", c.x, c.y, c.z );

  mh_vec3i_t v1 = mh_vec3i_init( 2, 2, 2 );
  mh_vec3i_t v2 = mh_vec3i_init( 4, 1, 3 );
  mh_vec3f_t v3 = mh_vec3f_init( 4.0, 1.0, 3.0 );
  mh_vec3f_t v4 = mh_vec3f_init( 2.0, 2.0, 2.0 );

  int v1dotv2 = mh_vec3i_dot( v1, v2 );
  float v3dotv4 = mh_vec3f_dot( v3, v4 );

  printf("%d %f\n", v1dotv2, v3dotv4 );

  return 0;
}