
/*
TODOs
[x] Fix speckles
    [x] Clamping
    [x] Better vertex normal calculation - weight by area etc.
[ ] Normal calculation
  [ ] Load a mesh with vertex normals and see how it looks
  [ ] Speed up the normal calculation with enkiTS
[ ] Move viewport rather than resizing model
[x] Stutter issue
  [x] Check tearing.c
  [x] Update glfw
*/

/*------------------------------------------------------------------------------
//  meshview.c
------------------------------------------------------------------------------*/
#define MSH_IMPLEMENTATION
#define MSH_CAM_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#include "msh.h"
#include "msh_vec_math.h"
#include "msh_argparse.h"
#include "msh_cam.h"
#include "experimental/msh_ply.h"

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol/sokol_gfx.h"


/*------------------------------------------------------------------------------
Command line opts 
------------------------------------------------------------------------------*/
typedef struct options
{
  char* input_filename;
  bool verbose;
  int width;
  int height;
} opts_t;

int parse_arguments( int argc, char** argv, opts_t* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->verbose         = 0;
  opts->width           = 800;
  opts->height          = 600;

  msh_init_argparse( "mshview",
                     "Simple utility for viewing meshes", 
                     &parser );
  msh_add_string_argument("input_filename", NULL, "Name of mesh file to read",
                           &opts->input_filename, 1, &parser );
  msh_add_bool_argument("--verbose", "-v", "Print verbose information",
                        &opts->verbose, 0, &parser );

  if( !msh_parse_arguments(argc, argv, &parser) )
  {
    msh_display_help( &parser );
    return 1;
  }
  return 0;
}

/*------------------------------------------------------------------------------
 Keyboard & Mouse
------------------------------------------------------------------------------*/
typedef struct mouse_state
{
  msh_vec2_t prev_pos;
  msh_vec2_t cur_pos;
  msh_scalar_t x_scroll_state;
  msh_scalar_t y_scroll_state;
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


static mouse_state_t mouse;
static keyboard_state_t keyboard;

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
  mouse.x_scroll_state = 0.5*xoffset;
  mouse.y_scroll_state = 0.5*yoffset;
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

/*------------------------------------------------------------------------------
 Mesh data structure
------------------------------------------------------------------------------*/
typedef struct face
{
  int i0, i1, i2;
} face_t;

typedef struct trimesh
{
  msh_vec3_t* positions;
  msh_vec3_t* normals;
  face_t* faces;
  int n_vertices;
  int n_faces;
} trimesh_t;

void 
trimesh_read_ply( trimesh_t* tm, const char* filename )
{
  ply_file_t* pf = ply_file_open( filename, "rb");
  ply_hint_t indices_size_hint = {.property_name = "vertex_indices", 
                                  .expected_size = 3};
  ply_file_add_hint(pf, indices_size_hint);
  if( pf )
  {
    ply_file_parse(pf);
    const char* positions_names[] = {"x","y","z"};
    const char* vertex_indices_names[] = {"vertex_indices"};
    ply_file_get_property_from_element(pf, "vertex", positions_names, 3, PLY_FLOAT, PLY_INVALID, 
                                      (void**)&tm->positions, NULL, &tm->n_vertices );
    ply_file_get_property_from_element(pf, "face", vertex_indices_names, 1, PLY_INT32, PLY_UINT8, 
                                       (void**)&tm->faces, NULL, &tm->n_faces);
  }
  ply_file_close(pf);
}

// TODO(maciej): This should just be zero mean
msh_mat4_t 
trimesh_calculate_normalizing_transform( const trimesh_t* tm )
{
  msh_vec3_t min_pt = msh_vec3_zeros();
  msh_vec3_t max_pt = msh_vec3_zeros();
  msh_vec3_t avg_pt = msh_vec3_zeros();
  for( int i = 0; i < tm->n_vertices; ++i )
  {
    msh_vec3_t pt = tm->positions[i];
    if( i == 0 ) { min_pt = pt; max_pt = pt; }
    min_pt.x = msh_min(pt.x, min_pt.x); max_pt.x = msh_max(pt.x, max_pt.x);
    min_pt.y = msh_min(pt.y, min_pt.y); max_pt.y = msh_max(pt.y, max_pt.y);
    min_pt.z = msh_min(pt.z, min_pt.z); max_pt.z = msh_max(pt.z, max_pt.z);
    avg_pt = msh_vec3_add(pt, avg_pt);
  }
  avg_pt = msh_vec3_scalar_div( avg_pt, tm->n_vertices );
  double sx = 1.0 / msh_abs(min_pt.x-max_pt.x);
  double sy = 1.0 / msh_abs(min_pt.y-max_pt.y);
  double sz = 1.0 / msh_abs(min_pt.z-max_pt.z);
  double scale = msh_max3(sx, sy, sz);

  msh_mat4_t xform = msh_scale( msh_mat4_identity(), 
                                msh_vec3(scale, scale, scale));
  xform = msh_translate( xform, msh_vec3_invert(avg_pt) );
  return xform;
}

void
trimesh_apply_transform( trimesh_t* tm, const msh_mat4_t xform )
{
  msh_mat4_t normal_matrix = msh_mat4_transpose(msh_mat4_inverse(xform));
  for( int i = 0; i < tm->n_vertices; ++i )
  {
    tm->positions[i] = msh_mat4_vec3_mul( xform, tm->positions[i], 1 );
  }
  if( tm->normals )
  {
    for( int i = 0 ; i < tm->n_vertices; ++i )
    {
      tm->normals[i] = msh_mat4_vec3_mul( normal_matrix, tm->normals[i], 0 );
    }
  }
}

void
trimesh_calculate_normals( trimesh_t* tm )
{
  
  if( !tm->normals )
  {
    for( int i = 0; i < tm->n_vertices; ++i ) { msh_array_push(tm->normals, msh_vec3_zeros()); }
  }
  else
  {
    for( int i = 0; i < tm->n_vertices; ++i ) { tm->normals[i] = msh_vec3_zeros(); }
  }

  for( int i = 0 ; i < tm->n_faces; ++i )
  {
    msh_vec3_t p0 = tm->positions[tm->faces[i].i0];
    msh_vec3_t p1 = tm->positions[tm->faces[i].i1];
    msh_vec3_t p2 = tm->positions[tm->faces[i].i2];

    msh_vec3_t v0 = msh_vec3_sub(p2, p0);
    msh_vec3_t v1 = msh_vec3_sub(p1, p0);

    msh_vec3_t normal = msh_vec3_cross(v0, v1);
    
    tm->normals[tm->faces[i].i0] = msh_vec3_add(tm->normals[tm->faces[i].i0], normal);
    tm->normals[tm->faces[i].i1] = msh_vec3_add(tm->normals[tm->faces[i].i1], normal);
    tm->normals[tm->faces[i].i2] = msh_vec3_add(tm->normals[tm->faces[i].i2], normal);
  }

  for( int i = 0; i < tm->n_vertices; ++i ) { tm->normals[i] = msh_vec3_normalize(tm->normals[i]); }

}




int main( int argc, char** argv ) {
  opts_t opts = {0};
  int parse_err = parse_arguments( argc, argv, &opts );
  if( parse_err ) { return 1; }


  /* create GLFW window and initialize GL */
  glfwInit();
  glfwWindowHint(GLFW_SAMPLES, 1);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* w = glfwCreateWindow(opts.width, opts.height, "MeshView", 0, 0);
  glfwMakeContextCurrent( w );
  glfwSetMouseButtonCallback( w, mouse_button_callback );
  glfwSetScrollCallback( w, mouse_scroll_callback );
  glfwSetKeyCallback( w, keyboard_callback );
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);
  /* read in and prep the mesh */
  trimesh_t mesh = {0};
  trimesh_read_ply( &mesh, opts.input_filename );
  msh_mat4_t center_scale = trimesh_calculate_normalizing_transform( &mesh );
  trimesh_apply_transform( &mesh, center_scale );
  trimesh_calculate_normals( &mesh );

  /* setup sokol_gfx */
  sg_desc desc = {0};
  sg_setup(&desc);
  assert(sg_isvalid());

  /* cube vertex buffer */
  sg_buffer pbuf = sg_make_buffer(&(sg_buffer_desc){
    .size = mesh.n_vertices * sizeof(msh_vec3_t),
    .content = mesh.positions,
  });

  sg_buffer nbuf = sg_make_buffer(&(sg_buffer_desc){
    .size = mesh.n_vertices * sizeof(msh_vec3_t),
    .content = mesh.normals,
  });

  sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
    .type = SG_BUFFERTYPE_INDEXBUFFER,
    .size = mesh.n_faces * sizeof(face_t),
    .content = mesh.faces,
  });

  /* create shader */
  typedef struct params {
    msh_mat4_t mvp;
    msh_mat4_t modelview;
    msh_vec3_t light_pos;
  } params_t;

  sg_shader shd = sg_make_shader(&(sg_shader_desc){
    .vs.uniform_blocks[0] = { 
      .size = sizeof(params_t),
      .uniforms = {
        [0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 },
        [1] = { .name="modelview", .type=SG_UNIFORMTYPE_MAT4 },
        [2] = { .name="light_pos", .type=SG_UNIFORMTYPE_FLOAT3 }
      },
    },

    .vs.source =
      "#version 330\n"
      "uniform mat4 mvp;\n"
      "uniform mat4 modelview;\n"
      "uniform vec3 light_pos;"
      "layout(location = 0) in vec3 position;\n"
      "layout(location = 1) in vec3 normal;\n"
      "out vec3 p;\n"
      "out vec3 n;\n"
      "out vec3 l;\n"
      "void main() {\n"
      "  p = vec3(modelview * vec4(position, 1.0));\n"
      "  n = vec3(modelview * vec4(normal, 0.0));\n"
      "  l = vec3(modelview * vec4(light_pos, 1.0));\n"
      "  gl_Position = mvp * vec4(position, 1.0);\n"
      "}\n",  
    .fs.source =
      "#version 330\n"
      "out vec4 frag_color;\n"
      "in vec3 p;"
      "in vec3 n;"
      "in vec3 l;"
      "void main() {\n"
      "  vec3 ld = normalize(p-l);\n"
      "  float intensity = clamp(dot(n,ld), 0.0, 1.0);\n"
      // "  frag_color = vec4(0.5*(n+1.0), 1.0);\n"
      "  frag_color = vec4(vec3(intensity), 1.0);\n"
      "}\n"
  });

  // TODO(maciej): Figure out how to do the operation with stride
  /* create pipeline object */
  sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
      .layout = {
          .attrs = {
              [0] = { .name="position", .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index = 0 },
              [1] = { .name="normal",   .format=SG_VERTEXFORMAT_FLOAT3, .buffer_index = 1 },
          }
      },
      .shader = shd,
      .index_type = SG_INDEXTYPE_UINT32,
      .depth_stencil = {
          .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
          .depth_write_enabled = true
      },
      .rasterizer.cull_mode = SG_CULLMODE_FRONT
  });

  /* draw state struct with resource bindings */
  sg_draw_state draw_state = {
      .pipeline = pip,
      .vertex_buffers[0] = pbuf,
      .vertex_buffers[1] = nbuf,
      .index_buffer = ibuf,
  };

  /* default pass action */
  sg_pass_action pass_action = { 0 };

  /* setup camera */
  msh_camera_t camera;
  msh_camera_init( &camera,
                    msh_vec3(0.0, 0.0, 5.0),
                    msh_vec3(0.0, 0.0, 0.0),
                    msh_vec3(0.0, 1.0, 0.0),
                    0.75,
                    1.0, 
                    0.1, 100.0 );

  params_t vs_params;
  float rx = 0.0f;
  while( !glfwWindowShouldClose(w) )
  {
    glfwPollEvents();

    int cur_width, cur_height;
    glfwGetFramebufferSize(w, &cur_width, &cur_height);
    float aspect_ratio = (float)cur_width/(float)cur_height;
    msh_arcball_camera_update( &camera, 
                                mouse.prev_pos, mouse.cur_pos,
                                mouse.lmb_state,
                                mouse.rmb_state,
                                mouse.y_scroll_state,
                                msh_vec4(0, 0, cur_width, cur_height));
    mouse_refresh( w ); 

      /* model-view-projection matrix for vertex shader */
    msh_mat4_t proj = msh_perspective(msh_deg2rad(60.0), aspect_ratio, 0.01f, 10.0f);
    msh_mat4_t view_proj = msh_mat4_mul(proj, camera.view);
    msh_mat4_t model = msh_rotate(msh_mat4_identity(), rx, msh_vec3(0.0f, 1.0f, 0.0f));
    rx+=0.1;
    vs_params.mvp = msh_mat4_mul(view_proj, model);
    vs_params.modelview = msh_mat4_mul(camera.view, model);
    vs_params.light_pos = msh_vec3(0.0f, 0.0f, 5.0f);
    sg_begin_default_pass(&pass_action, cur_width, cur_height);
    sg_apply_draw_state(&draw_state);
    sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
    sg_draw(0, mesh.n_faces*3, 1);
    sg_end_pass();
    sg_commit();

    glfwSwapBuffers(w);
  }

  sg_shutdown();
  glfwDestroyWindow(w);
  glfwTerminate();
}