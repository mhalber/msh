#include "msh.h"

// #ifdef __APPLE__
  // #include <OpenGL/gl3.h>
// #else
#include "glad/glad.h"
// #endif

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

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
  // glfwWindowHint(GLFW_SAMPLES, 8);
  // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  window = glfwCreateWindow( 512, 512, "Simple example", NULL, NULL );

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
  GLuint64 elapsed_time;
  glGenQueries(1, &query);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glEnable(GL_FRAMEBUFFER_SRGB);
  while( !glfwWindowShouldClose(window) )
  {
    int done = 0;
    int fb_w, fb_h;
    double mx, my;
    glBeginQuery(GL_TIME_ELAPSED,query);
    glfwGetFramebufferSize( window, &fb_w, &fb_h );
    glfwGetCursorPos(window, &mx, &my);
    
    glViewport( 0, 0, fb_w, fb_h );
    glClearColor( 1.0f, 0.5f, 0.5f, 1.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    msh_draw_new_frame( &draw_ctx, fb_w, fb_h );
    ttTime();

    static float t = 0.0f;
    // t += 0.01f;
    float size = 256.0f;
    // msh_draw_gradient( &draw_ctx, 0.91f, 0.02f, 0.25f, 0.21f, 0.84f, 0.32f);
    msh_draw_gradient( &draw_ctx, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    msh_draw_rectangle( &draw_ctx, 100.0f, 100.0f, 250.0f, 400.0f );

    msh_draw_gradient( &draw_ctx, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    msh_draw_rectangle( &draw_ctx, 220.0f, 100.0f, 400.0f, 400.0f );

    msh_draw_gradient( &draw_ctx, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f);
    msh_draw_rectangle( &draw_ctx, 450.0f, 100.0f, 514.0f, 164.0f );
    // msh_draw_gradient( &draw_ctx, 0.91f, 0.02f, 0.25f, 0.21f, 0.84f, 0.32f);
    // msh_draw_circle( &draw_ctx, 384.0f, 256.00f, size + 32 * sinf(t) );
    
    // msh_draw_triangle( &draw_ctx, 256.0f, 256.0f, size - 100 * sinf(t) );

    msh_draw_render( &draw_ctx );

    glEndQuery(GL_TIME_ELAPSED);
    while (!done) {
      glGetQueryObjectiv(query, 
                        GL_QUERY_RESULT_AVAILABLE, 
                        &done);
            }
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
    
    glfwSwapBuffers(window);
    glfwPollEvents();
    printf("Time Elapsed: %fs, %fms \n", ttTime(), elapsed_time/1000000.0f );
   
  }
  glfwDestroyWindow(window);
  glfwTerminate();

  return 1;
}
