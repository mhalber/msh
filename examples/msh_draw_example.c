#define MSH_DRAW_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#define TT_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION
// #define STB_TRUETYPE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include "msh.h"
#include "glad/glad.h"
#include "tt/tiny_time.h"
#include "stb/stb_image.h"
#include "stb/stb_rect_pack.h"
#include "stb/stb_truetype.h"
#include "stb/stb_image_write.h" //TODO(maciej): This is temp
#include "msh_draw.h"
#include "msh_vec_math.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

#include "stdio.h"

int amod(int x, int m)
{
  return (x % m + m) % m;
}

int main( int argc, char** argv )
{
  ttTime();
  msh_cutouts_path_t path;
  int tris[10024];
  // msh_cutouts_path_begin(&path, 100.0f, 100.0f);
  // msh_cutouts_line_to(&path, 100.0f, 140.0f);
  // msh_cutouts_line_to(&path, 50.0f, 120.0f);
  // msh_cutouts_line_to(&path, 100.0f, 160.0f);
  // msh_cutouts_line_to(&path, 30.0f, 180.0f);
  // msh_cutouts_line_to(&path, 100.0f, 200.0f);
  // msh_cutouts_line_to(&path, 140.0f, 200.0f);
  // msh_cutouts_line_to(&path, 150.0f, 140.0f);
  // msh_cutouts_line_to(&path, 160.0f, 200.0f);
  // msh_cutouts_line_to(&path, 200.0f, 200.0f);
  // msh_cutouts_line_to(&path, 200.0f, 100.0f);
  // msh_cutouts_line_to(&path, 190.0f, 190.0f);
  // msh_cutouts_line_to(&path, 180.0f, 100.0f);
  // msh_cutouts_path_end(&path);

  float cx = 256;
  float cy = 256;
  float r  = 128;
  int c = 0;
  for( float i = 0.0f ; i <= 360.0f; i += 10.0f )
  {
    float theta = (float)msh_deg2rad(i);
    float x = cx + r * sinf(theta);
    float y = cy + r * cosf(theta);
    if(i==0) msh_cutouts_path_begin(&path, x, y);
    else     msh_cutouts_line_to(&path, x, y);
    if(c==1 || c==2) r = 128;
    else r = 100;
    c = (c+1)%3;  
  }
  msh_cutouts_path_end(&path);
  msh_cutouts__path_to_shape(&path, &tris[0]);
  printf("TimeTo Triangulate: %f %d\n", ttTime(), path.idx-2);

  GLFWwindow* window;
  NVGcontext* vg = NULL;
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
  glfwWindowHint( GLFW_SAMPLES, 1 );
  // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  window = glfwCreateWindow( 1024, 512, "Simple example", NULL, NULL );

  if( !window )
  {
    msh_eprintf("Could not open window :<!\n");
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

  vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
  if (vg == NULL) {
    printf("Could not init nanovg.\n");
    return -1;
  }


  // Draw loop
  GLuint query;
  GLuint64 elapsed_time;
  glGenQueries(1, &query);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glBlendEquation(GL_FUNC_ADD);
  // glEnable(GL_FRAMEBUFFER_SRGB);

  while( !glfwWindowShouldClose(window) )
  {
    int done = 0;
    int fb_w, fb_h;
    int win_w, win_h;
    double mx, my;

    glfwGetFramebufferSize( window, &fb_w, &fb_h );
    glfwGetWindowSize( window, &win_w, &win_h );
    glfwGetCursorPos(window, &mx, &my);
    
    glViewport( 0, 0, fb_w, fb_h );
    glClearColor( 0.9f, 0.9f, 0.9f, 1.0f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glBeginQuery(GL_TIME_ELAPSED,query);
    elapsed_time = 0;
    ttTime();

    nvgBeginFrame(vg, win_w, win_h, (float)fb_w / win_w);
    // nvgBeginPath(vg);
    // nvgMoveTo(vg, 128.0f, 128.0f);
    // nvgLineTo(vg, 128.0f, 256.0f);
    // nvgLineTo(vg, 256.0f, 256.0f);
    // nvgLineTo(vg, 256.0f, 226.0f);
    // nvgLineTo(vg, 106.0f, 192.0f);
    // nvgLineTo(vg, 256.0f, 158.0f);
    // nvgLineTo(vg, 256.0f, 128.0f);
    // nvgFillColor(vg, nvgRGBA(128, 128, 243, 255));
    // nvgFill(vg);
    
    nvgBeginPath(vg);
    for( int i = 0 ; i < path.idx-2; ++i)
    {
      int idx1 = tris[3*i+0];
      int idx2 = tris[3*i+1];
      int idx3 = tris[3*i+2];
      nvgMoveTo(vg, path.vertices[2*idx1], path.vertices[2*idx1+1]);
      nvgLineTo(vg, path.vertices[2*idx2], path.vertices[2*idx2+1]);
      nvgLineTo(vg, path.vertices[2*idx3], path.vertices[2*idx3+1]);
      nvgLineTo(vg, path.vertices[2*idx1], path.vertices[2*idx1+1]);
    }
    nvgFillColor(vg, nvgRGBA(128, 128, 143, 255));
    nvgFill(vg);
    nvgEndFrame(vg);



    glEndQuery(GL_TIME_ELAPSED);
    while (!done) {
      glGetQueryObjectiv(query, 
                        GL_QUERY_RESULT_AVAILABLE, 
                        &done);
            }
    glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
    
    // printf("Time Elapsed: %fs, %fms \n", ttTime(), elapsed_time/1000000.0f );
    
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glfwDestroyWindow(window);
  glfwTerminate();

  return 1;
}
