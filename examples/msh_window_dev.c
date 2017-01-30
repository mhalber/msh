
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h> /* What is this header including? */
#include <sys/time.h>
#include <time.h>
#include <float.h>
#include <math.h>


#define MSH_ARGPARSE_IMPLEMENTATION
#include "msh_argparse.h"

#define MSH_VEC_MATH_IMPLEMENTATION
#include "msh_vec_math.h"

#define MSH_OGL_IMPLEMENTATION
#define MAX_WINDOWS 10
#include "msh_ogl.h"


/*
 TO COMPILE
 MAC OSX : time cc -O -std=c11 -Wall -Wextra -Wpedantic -Werror msh_window_dev.c -o bin/msh_window -lglfw3 -framework OpenGL
*/

#ifndef MSH_VIEWPORT
#define MSH_VIEWPORT

typedef struct msh_viewport
{
  msh_point2_t p1;
  msh_point2_t p2;
} msh_viewport_t;


#endif /* MSH_VIEWPORT */

#define MSH_VIEWPORT_IMPLEMENTATION

#ifdef MSH_VIEWPORT_IMPLEMENTATION
/* Note -> do we need manager? Should we have msh_viewports or viewport?? */
int msh_viewport_initialize(msh_viewport_t *v, msh_point2_t p1, msh_point2_t p2)
{
  v->p1 = p1;
  v->p2 = p2;
  return 1;
}

void msh_viewport_begin( const msh_viewport_t *v)
{
  glViewport( v->p1.x, v->p1.y, v->p2.x, v->p2.y );
}

void msh_viewport_end()
{
  glViewport( 0, 0, 0, 0 ); /* no active viewport or make it no-op just for blocking? */
}

#endif /* MSH_VIEWPORT_IMPLEMENTATION */

/******************************************************************************/

#define N_VIEWPORTS 2
static msh_viewport_t viewports[N_VIEWPORTS];


int window_A_display( void )
{
  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  /* msh_viewport_begin( &viewports[0] ); */
  glViewport( 0, 0, 100, 100 );
  glClearColor( 0.9f, 0.5f, 0.5f, 1.0f );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  /* msh_viewport_begin( &viewports[1] ); */
  glViewport( 100, 100, 200, 200 );
  glClearColor( 0.5f, 0.5f, 0.9f, 1.0f );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  return 1;
}

int main( int argc, char** argv )
{
  char *filename_a, *filename_b;
  int test[4] = {};
  bool bool_a = false;

  msh_argparse_t parser;
  msh_init_argparse("Test Program", "Test description", &parser );

  msh_add_string_argument("filename_a", NULL, "Filename for some file that we need",
                           &filename_a, 1, &parser );
  msh_add_string_argument("filename_b", NULL, "Filename for some file that we need",
                           &filename_b, 1, &parser );
  msh_add_bool_argument("--help", "-h", "Print help information", &bool_a, 1, &parser );
  msh_add_int_argument("--vector", "-v", "Some numbers", &test[0], 4, &parser );
  
  if ( !msh_parse_arguments( argc, argv, &parser ) ) msh_display_help( &parser );

  printf("%s\n", filename_a );
  printf("%s\n", filename_b );
  printf("%d %d %d %d\n", test[0], test[1], test[2], test[3] );
  printf("%s\n", bool_a ? "true" : "false" );

  return 0;
  /* setup the functions */
  msh_window_t *windows[MAX_WINDOWS];
  int (*display_functions[MAX_WINDOWS]) (void);
  windows[0] = msh_window_create( "Test1", -1, -1, 640, 480 );
  display_functions[0] = window_A_display;
  int n_windows = 1;

  /* setup the viewports */
  msh_point2_t p1, p2, p3, p4;
  p1.x = 0, p1.y = 0;
  p2.x = 640, p2.y = 240;
  p3.x = 0, p3.y = 240;
  p4.x = 640, p4.y = 480;
  msh_viewport_initialize(&viewports[0], p1, p2);
  msh_viewport_initialize(&viewports[1], p3, p4);

  /* iterate over the windows */
  while ( msh_window_is_any_open( windows, n_windows ) ) 
  {
    for ( int i = 0 ; i < n_windows ; ++i )
    {
      if ( windows[i] )
      {
        /* msh_window_display( windows[i], display_functions[i]  ); */
        /* Seems like I misunderstand something about viewports. Need additional
           functions in ogl to use this effectively... */
        glfwMakeContextCurrent(windows[i]);
        glViewport( 0, 0, 640, 480 );
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glViewport( 100, 100, 640, 480 );
        glClearColor( 1.0f, 0.0f, 0.0f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        glfwSwapBuffers(windows[i]);
      }
    }
    msh_window_poll_events();
  }

  msh_window_terminate();

  return 0;
}
