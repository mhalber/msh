#include "msh.h"

#define MSH_DRAW_IMPLEMENTATION
#include "msh_draw.h"




int main( int argc, char** argv )
{
  GLFWwindow* window;
  msh_draw_ctx_t draw_ctx;

  // Initialize GLFW and create window
  if( !glfwInit() )
  {
    msh_eprintf("Could not open window!\n");
    exit(EXIT_FAILURE);
  } 

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  window = glfwCreateWindow( 640, 480, "Simple example", NULL, NULL );
  if( !window )
  {
    msh_eprintf("Could not open window!\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  // Initialize context
  if( !msh_draw_init_ctx( &draw_ctx ) )
  {
    msh_eprintf("Could not initialize draw context!\n");
  }

  // Draw loop
  while( !glfwWindowShouldClose(window) )
  {
    int fb_w, fb_h;
    glfwGetFramebufferSize( window, &fb_w, &fb_h );
    glViewport( 0, 0, fb_w, fb_h );
    glClearColor( 1.0, 1.0, 1.0, 1.0 );
    glClear(GL_COLOR_BUFFER_BIT);


    msh_draw_triangle( &draw_ctx, 0.5, 0.0, 0.2 );


    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  exit(EXIT_SUCCESS);
}
