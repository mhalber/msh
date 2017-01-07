/*
A simple wrapper around glfw, simplifying common actions like window creation. 
This is done by simply bundling togheter some functions that are commonly called
together.
Author: Maciej Halber

NOTE: Merge this with gfx??
*/

#ifndef MSH_WINDOW_H
#define MSH_WINDOW_H

// Does not seem like apple needs anything, so let's not bloat.
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

// TODO: Do not render if minimized / unfocused
// TODO: Callbacks
// TODO: Resizing
// TODO: Nuklear integration
// TODO: Make window handles opaque

// Interface

// TODO: USE ONLY IF UNDEFINED!
typedef struct msh_point
{
  float x;
  float y;
} msh_point_t;

typedef GLFWwindow msh_window_t ;

// TODO -> FREE!!!

msh_window_t * msh_window_create( const char * title, 
                                  const int pos_x, const int pos_y, 
                                  const int res_x, const int res_y );
int  msh_window_display( msh_window_t * window, 
                         int (*display_function)(void) );
int  msh_window_destroy( msh_window_t * window );
int  msh_window_is_any_open( msh_window_t ** windows, const int n_windows );
void msh_window_poll_events(void);
void msh_window_terminate(void);

#endif //MSH_WINDOW_H

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
  // NOTE: This is a contract, probably needs to be specified once. bit like a 
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
