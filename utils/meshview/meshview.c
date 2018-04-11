#define MSH_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#define MSH_CAM_IMPLEMENTATION

#include "msh.h"
#include "msh_argparse.h"
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol/sokol_gfx.h"


typedef struct options
{
  char* input_filename;
  bool verbose;
}opts_t;

int parse_arguments( int argc, char** argv, opts_t* opts)
{
  msh_argparse_t parser;
  opts->input_filename  = NULL;
  opts->verbose         = 0;

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

int main( int argc, char** argv )
{
  opts_t opts;
  int parse_err = parse_arguments( argc, argv, &opts );
  if( parse_err ) { return 1; }

  double t1,t2;
  t1 = msh_get_time(MSHT_MILLISECONDS);
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow* w = glfwCreateWindow(640, 480, "meshview", 0, 0);
  glfwMakeContextCurrent(w);
  t2 = msh_get_time(MSHT_MILLISECONDS);
  msh_cprintf(opts.verbose, "Window creation: %fms.\n", t2-t1);

  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  glfwSwapInterval(1);

//// SOKOL STUFF

/* setup sokol_gfx */
    sg_setup(&(sg_desc){0});

    /* a vertex buffer */
    const float vertices[] = {
        // positions            // colors
         0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f 
    };
    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
        .size = sizeof(vertices),
        .content = vertices, 
    });

    /* a shader */
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vs.source = 
            "#version 330\n"
            "in vec4 position;\n"
            "in vec4 color0;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "  gl_Position = position;\n"
            "  color = color0;\n"
            "}\n",
        .fs.source =
            "#version 330\n"
            "in vec4 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = color;\n"
            "}\n"
    });

    /* a pipeline state object */
    sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        /* if the vertex layout doesn't have gaps, don't need to provide strides and offsets */
        .layout = {
            .attrs = {
                [0] = { .name="position", .format=SG_VERTEXFORMAT_FLOAT3 },
                [1] = { .name="color0", .format=SG_VERTEXFORMAT_FLOAT4 }
            }
        }
    });

    /* a draw state with all the resource binding */
    sg_draw_state draw_state = {
        .pipeline = pip,
        .vertex_buffers[0] = vbuf
    };

    /* default pass action (clear to grey) */
    sg_pass_action pass_action = {0};
////////////



  while( !glfwWindowShouldClose(w) )
  {
     int cur_width, cur_height;
    glfwGetFramebufferSize(w, &cur_width, &cur_height);
    sg_begin_default_pass(&pass_action, cur_width, cur_height);
    sg_apply_draw_state(&draw_state);
    sg_draw(0, 3, 1);
    sg_end_pass();
    sg_commit();
    
    glfwSwapBuffers(w);
    glfwPollEvents();
  }
  glfwDestroyWindow(w);
  glfwTerminate();



  return 0;
}