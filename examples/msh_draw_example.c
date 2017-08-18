#include "msh.h"

#define MSH_DRAW_IMPLEMENTATION
#include "msh_draw.h"

#include "stdio.h"
#define TT_IMPLEMENTATION
#include "tt/tiny_time.h"
int main( int argc, char** argv )
{
  GLFWwindow* window;
  msh_draw_ctx_t draw_ctx;

  // Initialize GLFW and create window
  if( !glfwInit() )
  {
    msh_eprintf("Could not initialize glfw!\n");
    exit(EXIT_FAILURE);
  } 

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  window = glfwCreateWindow( 640, 480, "Simple example", NULL, NULL );

  if( !window )
  {
    msh_eprintf("Could not open window :<!\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

  // Initialize context
  if( !msh_draw_init_ctx( &draw_ctx ) )
  {
    msh_eprintf("Could not initialize draw context!\n");
  }

  // Draw loop
  float y1 = 0.0f;
  float y2 = 0.0f;
  GLuint query;
  // GLuint64 elapsed_time;
 
  // glGenQueries(1, &query);

  while( !glfwWindowShouldClose(window) )
  {
    int done = 0;
    // glBeginQuery(GL_TIME_ELAPSED,query);
    int fb_w, fb_h;
    glfwGetFramebufferSize( window, &fb_w, &fb_h );
    glViewport( 0, 0, fb_w, fb_h );
    glClearColor( 0.8f, 0.8f, 0.8f, 1.0f );
    glClear(GL_COLOR_BUFFER_BIT);

    msh_draw_new_frame( &draw_ctx );
    ttTime();
    msh_draw_fill_color( &draw_ctx, 0.01f, 0.02f, 0.25f);
    msh_draw_triangle( &draw_ctx, 0.0f, 0.0f, 0.5f );
    msh_draw_fill_color( &draw_ctx, 0.91f, 0.02f, 0.25f);
    msh_draw_triangle( &draw_ctx, 0.0f, -0.25f, 0.5f );
    msh_draw_render( &draw_ctx );
    printf("Time Elapsed: %f s\n", ttTime() );

    // glEndQuery(GL_TIME_ELAPSED);
    // while (!done) {
    //   glGetQueryObjectiv(query, 
    //                     GL_QUERY_RESULT_AVAILABLE, 
    //                     &done);
    //         }
    // glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
   
  }
  glfwDestroyWindow(window);
  glfwTerminate();

  return 1;
}
