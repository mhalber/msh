#include <stdio.h>
#include <float.h>
#include <math.h>

#define MSH_VEC_MATH_IMPLEMENTATION
#define MSH_GFX_IMPLEMENTATION
#define MSH_CAM_IMPLEMENTATION
#define ARCBALL_CAMERA_IMPLEMENTATION
#include "msh_vec_math.h"
#include "msh_gfx.h"
#include "msh_cam.h"

/* Global data */
/* NOTE: Types should be in msh_ namespace and the functions should have the 
  identifier like mshgfx or mshcam */
static mshgfx_window_t *win; 
static mshgfx_geometry_t cube_geo;
static mshgfx_shader_prog_t cube_shader;
static msh_camera_t camera;
static msh_vec2_t prev_pos;
static msh_vec2_t cur_pos;

/* Shaders */
char* cube_vs_src = (char*) MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY
(
  layout (location = 0) in vec3 position;
  layout (location = 4) in vec4 color;
  uniform mat4 mvp;
  out vec4 v_color;
  void main()
  {
    gl_Position = mvp * vec4( position, 1.0 );
    v_color = color;
  }
);

char* cube_fs_src = (char*) MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY
(
  in vec4 v_color;
  out vec4 frag_color;
  void main()
  {
    frag_color = v_color;
  }
);

static msh_vec3_t eye = msh_vec3( 0.0f, 0.0f, 5.0f );
static msh_vec3_t target = msh_vec3( 0.0f, 0.0f, 0.0f );
static msh_vec3_t up = msh_vec3( 0.0f, 1.0f, 0.0f );

/* Actual code */
int init()
{
  /* setup view matrix */
  msh_arcball_camera_init( &camera,
                           eye,
                           target,
                           up,
                           0.75,
                           1.0, 
                           0.1, 100.0 );

  /* setup cube geometry and send it to gpu */
  const int vertex_count  = 8;
  const int indices_count = 36;

  float positions[] =
  {
    -0.5, -0.5, -0.5, // 0
    -0.5, -0.5,  0.5, // 1
     0.5, -0.5,  0.5, // 2
     0.5, -0.5, -0.5, // 3
    -0.5,  0.5, -0.5, // 4
    -0.5,  0.5,  0.5, // 5
     0.5,  0.5,  0.5, // 6
     0.5,  0.5, -0.5, // 7
  };

  unsigned char colors[] =
  {
    255, 255, 255, 255, // 0
    255, 255,   0, 255, // 1
    255,   0, 255, 255, // 2
      0, 255, 255, 255, // 3
    255,   0,   0, 255, // 4
      0, 255,   0, 255, // 5
      0,   0, 255, 255, // 6
      0,   0,   0, 255, // 7
  };

  unsigned int indices[] = 
  {
      0, 2, 1, 0, 3, 2, // BOTTOM
      4, 5, 6, 4, 6, 7, // TOP
      1, 6, 5, 1, 2, 6, // FRONT
      2, 3, 6, 3, 7, 6, // RIGHT 
      3, 4, 7, 3, 0, 4, // BACK
      0, 5, 4, 0, 1, 5  // LEFT
  };
  
  mshgfx_geometry_data_t cube_data;
  cube_data.positions   = &(positions[0]);
  cube_data.colors_a    = &(colors[0]);
  cube_data.indices     = &(indices[0]);
  cube_data.n_vertices  = vertex_count;
  cube_data.n_elements  = indices_count;

  mshgfx_geometry_init( &cube_geo, &cube_data, 
                        POSITION | COLOR_A | STRUCTURED );  

  /* compile shader */
  mshgfx_shader_prog_create_from_source_vf( &cube_shader, 
                                            cube_vs_src, cube_fs_src );


  return 1;
}

msh_scalar_t scroll_state = 0.0f;

void scroll_callback(GLFWwindow* win, double xoffset, double yoffset)
{
  scroll_state = yoffset;
}

int display()
{
  int w, h;
  glfwGetFramebufferSize( win, &w, &h );
  glViewport( 0, 0, w, h);
  float aspect_ratio = (float)w/h;

  /* NOTE: How to push controls only to the camera to allow custom controls */
  int left_press = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT);
  int right_press = glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT);
  double x, y;
  double t = 0;
  glfwGetCursorPos( win, &x, &y );

  if( left_press == GLFW_PRESS || right_press == GLFW_PRESS || scroll_state != 0 )
  {
    cur_pos = (msh_vec2_t){.x = (float)x, 
                           .y = (float)y};

    msh_arcball_camera_update( &camera, 
                               prev_pos, cur_pos,
                               left_press,
                               right_press,
                               scroll_state,
                               msh_vec4(0, 0, w, h));
    scroll_state = 0.0f;
    t = glfwGetTime();
    prev_pos = cur_pos;
  }
  else
  {
    cur_pos = (msh_vec2_t){.x = (float)x, .y = (float)y}; 
    prev_pos = (msh_vec2_t){.x = (float)x, .y = (float)y}; 
    scroll_state = 0.0f;
  }
  mshgfx_background_gradient4fv( msh_vec4( 0.194f, 0.587f, 0.843f, 1.0f ), 
                                 msh_vec4( 0.067f, 0.265f, 0.394f, 1.0f ) );
  static float near = 0.1f;
  static float far  = 100.0f;
  static float fovy = 35.0f * (M_PI/180.0f);

  msh_mat4_t projection = msh_perspective( fovy, 
                                           aspect_ratio, 
                                           near, 
                                           far );
  static msh_mat4_t model = msh_mat4_identity();

  msh_mat4_t mvp = msh_mat4_mul( msh_mat4_mul( projection, camera.view ), model );

  mshgfx_shader_prog_use( &cube_shader );
  mshgfx_shader_prog_set_uniform_4fm( &cube_shader, "mvp", &mvp );
  mshgfx_geometry_draw( &cube_geo, GL_TRIANGLES );
 
  return 1;
}

void refresh( mshgfx_window_t * window )
{
  mshgfx_window_display( window, display );
}

int main()
{
  win = mshgfx_window_create( "Camera Controls", -1, -1, 1024, 1024 );
  
  mshgfx_window_activate( win );
  mshgfx_window_set_callback_refresh( win, refresh );
  glfwSetScrollCallback( win, scroll_callback );
  
  init();

  while( mshgfx_window_is_any_open( &win, 1 ) ) 
  {
    if( win )
    {
      mshgfx_window_display( win, display ); 
    }
    mshgfx_window_poll_events();
  }

  mshgfx_window_terminate();

  return 1;
}

