
#define MSH_WINDOW_IMPLEMENTATION
#define MAX_WINDOWS 10

#include <stdio.h>
#include <msh_window.h>

int window_A_display( void )
{
  glClearColor( 0.9f, 0.5f, 0.5f, 1.0f );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return 1;
}

int main( int argc, char ** argv )
{
  // setup the functions
  msh_window_t *windows[MAX_WINDOWS];
  int (*display_functions[MAX_WINDOWS]) (void);
  windows[0] = msh_window_create( "", -1, -1, 640, 480 );
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