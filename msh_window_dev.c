/* A simple experiment to get a just_gl! functionality to the mac os family. 
 * We want minimal code to create an application that put the window to the
 * screen, and is able to draw, as well as get the user input, like mouse and 
 * keystrokes */

// to compile : cc -framework OpenGL msh_window_dev.c -lglfw3 -O3 -o msh_window
// 

// NOTE: We will use a single library that is not a single file - glfw. I do not
// want to spend time implementing window handling. This was productive enough,

#ifndef MSH_WINDOW_H
#define MSH_WINDOW_H

#define MSH_WINDOW_IMPLEMENTATION

#include <stdio.h>

// Does not seem like apple needs anything, so let's not bloat.
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define MAX_WINDOWS 10

// helpers
void 
hex2rgba( int hex, float *r, float *g, float *b, float *a )
{
  *r = (unsigned char)((hex & 0xFF000000) >> 24) / 255.0f;
  *g = (unsigned char)((hex & 0x00FF0000) >> 16) / 255.0f;
  *b = (unsigned char)((hex & 0x0000FF00) >> 8) / 255.0f; 
  *a = (unsigned char)((hex & 0x000000FF)) / 255.0f; 
}

// Interface
// TODO: USE ONLY IF UNDEFINED!
typedef struct msh_point
{
  int x;
  int y;
} msh_point_t;

typedef GLFWwindow msh_window_t ;

msh_window_t * msh_window_create( const char * title, 
                                  const int pos_x, const int pos_y, 
                                  const int res_x, const int res_y );
int  msh_window_display( msh_window_t * window, 
                         int (*display_function)(void) );
int  msh_window_destroy( msh_window_t * window );
int  msh_window_is_any_open( msh_window_t ** windows, const int n_windows );
void msh_window_poll_events(void);
void msh_window_terminate(void);

// TODO: Do not render if minimized / unfocused
// TODO: Callbacks
// TODO: Resizing
// TODO: Nuklear integration
// TODO: Make window handles opaque
// TODO: git version control

// IMPLEMENTATION
#ifdef MSH_WINDOW_IMPLEMENTATION

msh_window_t * msh_window_create( const char * title,
                                  const int pos_x, const int pos_y,
                                  const int res_x, const int res_y )
{
  msh_window_t * window = NULL;

  // attempt to initialize glfw library
  if (!glfwInit())
  {
    return 0;
  }

  // setup some hints - this is osx specific
  // NOTE: This is contract, probably needs to be specified once. bit like a 
  // state machine
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
  
  // actually create window
  window = glfwCreateWindow( res_x, res_y, title, NULL, NULL );

  if (!window)
  {
    return NULL;
  }

  // change position
  if ( pos_x >= 0 && pos_y >= 0 )
  {
    glfwSetWindowPos( window, pos_x, pos_y );
  }
  
  return window;
}

int msh_window_destroy( msh_window_t * window )
{
  if ( window )
  {
    glfwDestroyWindow( window );
    return 1;
  }
  return 0;
}

int msh_window_display( msh_window_t * window,
                        int (*display_function)(void) )
{
  glfwMakeContextCurrent(window);
  if ( !display_function() )
  {
    return 0;
  }
  glfwSwapBuffers(window);
  return 1;
}

int msh_window_is_any_open( msh_window_t ** windows, const int n_windows )
{
  int is_any_open = 0;
  for ( int i = 0 ; i < n_windows ; ++i )
  {
    int is_valid = windows[i] ? 1 : 0;
    if ( is_valid )
    {
      int should_close = glfwWindowShouldClose( windows[i] );
      if ( should_close )
      {
         msh_window_destroy( windows[i] );
         windows[i] = NULL;
      }
    }
    is_any_open |= is_valid;
   }
  return is_any_open;
 }
 
void msh_window_poll_events( void )
{
  glfwPollEvents();
}

void msh_window_terminate( void )
{
  glfwTerminate();
}

#endif //MSH_WINDOW_IMPLEMENTATION

// TEST

int window_A_display( void )
{
  glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return 1;
}



int main( int argc, char ** argv )
{
  // setup the functions
  msh_window_t *windows[MAX_WINDOWS];
  int (*display_functions[MAX_WINDOWS]) (void);
  windows[0] = msh_window_create( "TEST_A", -1, -1, 640, 480 );
  display_functions[0] = window_A_display;
  int n_windows = 1;

  // iterate over the windows
  while ( msh_window_is_any_open( windows, n_windows ) ) 
  {
    for ( int i = 0 ; i < n_windows ; ++i )
    {
      if ( windows[i] )
      {
        msh_window_display( windows[i], display_functions[i]  );
      }
    }
    msh_window_poll_events();
  }

  msh_window_terminate();

  return 0;
}

#endif //MSH_WINDOW_H