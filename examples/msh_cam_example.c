#include <stdio.h>
#include <float.h>
#include <math.h>

#define MSH_VEC_MATH_IMPLEMENTATION
#define MSH_GFX_IMPLEMENTATION
#define MSH_CAM_IMPLEMENTATION
#define FLYTHROUGH_CAMERA_IMPLEMENTATION
#include "msh_vec_math.h"
#include "msh_gfx.h"
#include "msh_cam.h"

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


/* simple i/o state */
typedef struct mouse_state
{
  msh_vec2_t prev_pos;
  msh_vec2_t cur_pos;
  int x_scroll_state;
  int y_scroll_state;
  int lmb_state;
  int rmb_state;
  int mmb_state;
  int shift_key_state;
  int super_key_state;
  int alt_key_state;
  int ctrl_key_state;
} mouse_state_t;

typedef struct keyboard_state
{
  int pressed[GLFW_KEY_LAST];
} keyboard_state_t;

/* Global data */
static mshgfx_window_t *win; 
static mshgfx_geometry_t cube_geo;
static mshgfx_shader_prog_t cube_shader;
static msh_camera_t camera;
static mouse_state_t mouse;
static keyboard_state_t keyboard;
static float offsets[11][11];

static void 
mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if( !glfwGetWindowAttrib(window, GLFW_FOCUSED) ) return;
  mouse.lmb_state       = 0;
  mouse.rmb_state       = 0;
  mouse.mmb_state       = 0;
  mouse.shift_key_state = 0;
  mouse.super_key_state = 0;
  mouse.alt_key_state   = 0;
  mouse.ctrl_key_state  = 0;

  if( action == GLFW_PRESS )
  {
    if( button == GLFW_MOUSE_BUTTON_LEFT )   mouse.lmb_state = 1;
    if( button == GLFW_MOUSE_BUTTON_RIGHT )  mouse.rmb_state = 1;
    if( button == GLFW_MOUSE_BUTTON_MIDDLE ) mouse.mmb_state = 1;
    if( mods & GLFW_MOD_SHIFT )              mouse.shift_key_state = 1;
    if( mods & GLFW_MOD_SUPER )              mouse.super_key_state = 1;
    if( mods & GLFW_MOD_ALT )                mouse.alt_key_state   = 1;
    if( mods & GLFW_MOD_CONTROL )            mouse.ctrl_key_state  = 1;
  }
}

static void 
mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if( !glfwGetWindowAttrib(window, GLFW_FOCUSED) ) return;
  mouse.x_scroll_state = xoffset;
  mouse.y_scroll_state = yoffset;
}

static void
keyboard_callback( GLFWwindow *window, int key, int scancode, int action, int mods)
{
  keyboard.pressed[ key ] = 0;

  if( action == GLFW_PRESS || action == GLFW_REPEAT )
  {
    keyboard.pressed[ key ] = 1;
  }

  if( keyboard.pressed[GLFW_KEY_ESCAPE] ) exit(1);

}

static void
mouse_refresh( GLFWwindow *window )
{
  mouse.prev_pos  = mouse.cur_pos;
  double x, y;
  glfwGetCursorPos( window, &x, &y );
  mouse.cur_pos.x = x;
  mouse.cur_pos.y = y;
  
  mouse.x_scroll_state = 0;
  mouse.y_scroll_state = 0;
}

/* Actual code */

int init()
{
  /* setup view matrix */
  msh_camera_init( &camera,
                           msh_vec3(0.0, 0.0, 5.0),
                           msh_vec3(0.0, 0.0, 0.0),
                           msh_vec3(0.0, 1.0, 0.0),
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
  cube_data.positions  = &(positions[0]);
  cube_data.colors_a   = &(colors[0]);
  cube_data.indices    = &(indices[0]);
  cube_data.n_vertices = vertex_count;
  cube_data.n_elements = indices_count;

  mshgfx_geometry_init( &cube_geo, &cube_data, 
                        POSITION | COLOR_A | STRUCTURED );  

  /* compile shader */
  mshgfx_shader_prog_create_from_source_vf( &cube_shader, 
                                            cube_vs_src, cube_fs_src );
  for(int i = -5 ; i < 6 ; ++i )
  {
    for(int j = -5 ; j < 6 ; ++j )
    {
      offsets[i+5][j+5] = sin((i-5)*0.5) * cos((j-5)*0.5);
    }
  }

  return 1;
}


int display()
{
  int w, h;
  glfwGetFramebufferSize( win, &w, &h );
  glViewport( 0, 0, w, h);
  float aspect_ratio = (float)w/h;

  /*
  msh_arcball_camera_update( &camera, 
                              mouse.prev_pos, mouse.cur_pos,
                              mouse.lmb_state,
                              mouse.rmb_state,
                              mouse.y_scroll_state,
                              msh_vec4(0, 0, w, h));
  */

  msh_flythrough_camera( &camera, 
                         mouse.prev_pos, mouse.cur_pos,
                         mouse.lmb_state,
                         keyboard.pressed[GLFW_KEY_W], 
                         keyboard.pressed[GLFW_KEY_S],
                         keyboard.pressed[GLFW_KEY_A],
                         keyboard.pressed[GLFW_KEY_D] );

  mouse_refresh( win ); 


  mshgfx_background_gradient4fv( msh_vec4( 0.194f, 0.587f, 0.843f, 1.0f ), 
                                 msh_vec4( 0.067f, 0.265f, 0.394f, 1.0f ) );
  static float near = 0.1f;
  static float far  = 100.0f;
  static float fovy = 35.0f * (M_PI/180.0f);

  msh_mat4_t projection = msh_perspective( fovy, 
                                           aspect_ratio, 
                                           near, 
                                           far );

  mshgfx_shader_prog_use( &cube_shader );

  for(int i = -5 ; i < 6 ; ++i )
  {
    for(int j = -5 ; j < 6 ; ++j )
    {
      msh_mat4_t model = msh_translate( msh_mat4_identity(), 
                                        msh_vec3( i, offsets[i+5][j+5] , j) );
      msh_mat4_t mvp = msh_mat4_mul( msh_mat4_mul( projection, camera.view ), model );
      mshgfx_shader_prog_set_uniform_4fm( &cube_shader, "mvp", &mvp );
      mshgfx_geometry_draw( &cube_geo, GL_TRIANGLES );
    }
  }
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
  glfwSetMouseButtonCallback( win, mouse_button_callback );
  glfwSetScrollCallback( win, mouse_scroll_callback );
  glfwSetKeyCallback( win, keyboard_callback );
  
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

