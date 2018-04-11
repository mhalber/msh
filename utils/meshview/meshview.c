//------------------------------------------------------------------------------
//  texcube-glfw.c
//------------------------------------------------------------------------------
#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol/sokol_gfx.h"
#define MSH_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#include "msh.h"
#include "msh_vec_math.h"
#include "experimental/msh_ply.h"

/* a uniform block with a model-view-projection matrix */
typedef struct {
    msh_mat4_t mvp;
} params_t;

int main( int argc, char** argv ) {
    const int WIDTH = 800;
    const int HEIGHT = 600;

    /* create GLFW window and initialize GL */
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* w = glfwCreateWindow(WIDTH, HEIGHT, "Sokol Textured Cube GLFW", 0, 0);
    glfwMakeContextCurrent(w);
    // glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

//// READ MESH
double t1, t2;
  t1 = msh_get_time(MSHT_MILLISECONDS);
  float* positions = NULL; int n_verts = -1;
  int* indices = NULL; int n_faces = -1;
  ply_file_t* pf = ply_file_open( argv[1] , "rb");
  ply_hint_t indices_size_hint = {.property_name = "vertex_indices", 
                                  .expected_size = 3};
  ply_file_add_hint(pf, indices_size_hint);
  if( pf )
  {
    ply_file_parse(pf);
    const char* positions_names[] = {"x","y","z"};
    const char* vertex_indices_names[] = {"vertex_indices"};
    ply_file_get_property_from_element(pf, "vertex", positions_names, 3, PLY_FLOAT, PLY_INVALID, 
                                      (void**)&positions, NULL, &n_verts );
    ply_file_get_property_from_element(pf, "face", vertex_indices_names, 1, PLY_INT32, PLY_UINT8, 
                                       (void**)&indices, NULL, &n_faces);
    msh_vec3_t min_pt = msh_vec3_zeros();
    msh_vec3_t max_pt = msh_vec3_zeros();
    msh_vec3_t avg_pt = msh_vec3_zeros();
    for( int i = 0; i < n_verts; ++i )
    {
      msh_vec3_t* pt = (msh_vec3_t*)&positions[3*i];
      if( i == 0 ) { min_pt = *pt; max_pt = *pt; }
      min_pt.x = msh_min(pt->x, min_pt.x); max_pt.x = msh_max(pt->x, max_pt.x);
      min_pt.y = msh_min(pt->y, min_pt.y); max_pt.y = msh_max(pt->y, max_pt.y);
      min_pt.z = msh_min(pt->z, min_pt.z); max_pt.z = msh_max(pt->z, max_pt.z);
      avg_pt = msh_vec3_add(*pt, avg_pt);
    }
    avg_pt = msh_vec3_scalar_div( avg_pt, n_verts );
    double sx = 1.0 / msh_abs(min_pt.x-max_pt.x);
    double sy = 1.0 / msh_abs(min_pt.y-max_pt.y);
    double sz = 1.0 / msh_abs(min_pt.z-max_pt.z);
    double s = msh_max3(sx, sy, sz);
    printf("%g %g %g | %g\n", sx, sy, sz, s);
    // msh_mat4_t xform = msh_translate( msh_mat4_identity(), msh_vec3_invert(avg_pt) );
    // xform = msh_scale(xform, msh_vec3(sx, sy, sz));
    msh_mat4_t xform = msh_scale(msh_mat4_identity(), msh_vec3(s, s, s));
    xform = msh_translate( xform, msh_vec3_invert(avg_pt) );
    avg_pt = msh_vec3_zeros();
        for( int i = 0; i < n_verts; ++i )
    {
      msh_vec3_t* pt = (msh_vec3_t*)&positions[3*i];
      *pt = msh_mat4_vec3_mul( xform, *pt, 1);
      if( i == 0 ) { min_pt = *pt; max_pt = *pt; }
      min_pt.x = msh_min(pt->x, min_pt.x); max_pt.x = msh_max(pt->x, max_pt.x);
      min_pt.y = msh_min(pt->y, min_pt.y); max_pt.y = msh_max(pt->y, max_pt.y);
      min_pt.z = msh_min(pt->z, min_pt.z); max_pt.z = msh_max(pt->z, max_pt.z);
      avg_pt = msh_vec3_add(*pt, avg_pt);
    }
    avg_pt = msh_vec3_scalar_div( avg_pt, n_verts );
    msh_vec3_print(avg_pt);
    msh_vec3_print(min_pt);
    msh_vec3_print(max_pt);

    // msh_cprintf( opts.verbose, "Vertex count: %d\n", n_verts );
    // msh_cprintf( opts.verbose, "Face Count: %d\n", n_faces );
  }
  // ply_file_print_header(pf);
  t2 = msh_get_time(MSHT_MILLISECONDS);
  // msh_cprintf(opts.verbose, "Mesh reading: %fms.\n", t2-t1);
  ply_file_close(pf);
  // printf( "Mesh reading: %fms.\n", t2-t1);

    /* setup sokol_gfx */
    sg_desc desc = {0};
    sg_setup(&desc);
    assert(sg_isvalid());

/* cube vertex buffer */
    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
        .size = n_verts * 3 * sizeof(float),
        .content = positions,
    });

    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = n_faces * 3 * sizeof(int),
        .content = indices,
    });

    /* create shader */
    sg_shader shd = sg_make_shader(&(sg_shader_desc){
        .vs.uniform_blocks[0] = { 
            .size = sizeof(params_t),
            .uniforms = {
                [0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 }
            },
        },
        .vs.source =
            "#version 330\n"
            "uniform mat4 mvp;\n"
            "layout(location = 0) in vec4 position;\n"
            "void main() {\n"
            "  gl_Position = mvp * position;\n"
            "}\n",
        .fs.source =
            "#version 330\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "  frag_color = vec4(1.0, 0.5, 0.0, 1.0);\n"
            "}\n"
    });

    /* create pipeline object */
    sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
        .layout = {
            /* on GL3.3 we can ommit the vertex attribute name if the 
               vertex shader explicitely defines the attribute location
               via layout(location = xx), and since the vertex layout
               has no gaps, we don't need to give the vertex stride
               or attribute offsets
            */
            .attrs = {
                [0] = { .format=SG_VERTEXFORMAT_FLOAT3 },
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
        .vertex_buffers[0] = vbuf,
        .index_buffer = ibuf,
    };

    /* default pass action */
    sg_pass_action pass_action = { 0 };

    /* view-projection matrix */
    msh_mat4_t proj = msh_perspective(msh_deg2rad(60.0), 640.0f/480.0f, 0.001f, 10.0f);
    msh_mat4_t view = msh_look_at(msh_vec3(2.0f, 2.0f, 2.0f), msh_vec3(0.0f, 0.0f, 0.0f), msh_vec3(0.0f, 1.0f, 0.0f));
    msh_mat4_t view_proj = msh_mat4_mul(proj, view);

    params_t vs_params;
    float rx = 0.0f, ry = 0.0f;
    while( !glfwWindowShouldClose(w) )
    {
        // rx += 0.01f; ry += 0.02f;
        // msh_mat4_t rxm = msh_rotate( msh_mat4_identity(), rx, msh_vec3(1.0f, 0.0f, 0.0f));
        msh_mat4_t rym = msh_rotate( msh_mat4_identity(), ry, msh_vec3(0.0f, 1.0f, 0.0f));
        msh_mat4_t model = msh_mat4_identity();

        /* model-view-projection matrix for vertex shader */
        vs_params.mvp = msh_mat4_mul(view_proj, model);

        int cur_width, cur_height;
        glfwGetFramebufferSize(w, &cur_width, &cur_height);
        sg_begin_default_pass(&pass_action, cur_width, cur_height);
        sg_apply_draw_state(&draw_state);
        sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
        sg_draw(0, n_faces * 3, 1);
        sg_end_pass();
        sg_commit();

        glfwSwapBuffers(w);
        glfwPollEvents();
    }

    sg_shutdown();
    glfwTerminate();
}


// #define MSH_IMPLEMENTATION
// #define MSH_ARGPARSE_IMPLEMENTATION
// #define MSH_VEC_MATH_IMPLEMENTATION
// #define MSH_CAM_IMPLEMENTATION

// #include "msh.h"
// #include "msh_argparse.h"
// #include "msh_vec_math.h"
// #include "experimental/msh_ply.h"
// #define GLFW_INCLUDE_NONE
// #include "GLFW/glfw3.h"
// #include "glad/glad.h"
// #define SOKOL_IMPL
// #define SOKOL_GLCORE33
// #include "sokol/sokol_gfx.h"


// typedef struct options
// {
//   char* input_filename;
//   bool verbose;
// }opts_t;

// typedef struct {
//     msh_mat4_t mvp;
// } params_t;


// int parse_arguments( int argc, char** argv, opts_t* opts)
// {
//   msh_argparse_t parser;
//   opts->input_filename  = NULL;
//   opts->verbose         = 0;

//   msh_init_argparse( "mshview",
//                      "Simple utility for viewing meshes", 
//                      &parser );
//   msh_add_string_argument("input_filename", NULL, "Name of mesh file to read",
//                            &opts->input_filename, 1, &parser );
//   msh_add_bool_argument("--verbose", "-v", "Print verbose information",
//                         &opts->verbose, 0, &parser );

//   if( !msh_parse_arguments(argc, argv, &parser) )
//   {
//     msh_display_help( &parser );
//     return 1;
//   }
//   return 0;
// }

// int main( int argc, char** argv )
// {
//   opts_t opts;
//   int parse_err = parse_arguments( argc, argv, &opts );
//   if( parse_err ) { return 1; }

//   double t1,t2;
//   t1 = msh_get_time(MSHT_MILLISECONDS);
//   glfwInit();
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//   GLFWwindow* w = glfwCreateWindow(640, 480, "meshview", 0, 0);
//   glfwMakeContextCurrent(w);
//   t2 = msh_get_time(MSHT_MILLISECONDS);
//   msh_cprintf(opts.verbose, "Window creation: %fms.\n", t2-t1);

//   gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
//   glfwSwapInterval(1);

// //// READ MESH
//   // t1 = msh_get_time(MSHT_MILLISECONDS);
//   // float* positions = NULL; int n_verts = -1;
//   // int* indices = NULL; int n_faces = -1;
//   // ply_file_t* pf = ply_file_open( opts.input_filename , "r");
//   // ply_hint_t indices_size_hint = {.property_name = "vertex_indices", 
//   //                                 .expected_size = 3};
//   // ply_file_add_hint(pf, indices_size_hint);
//   // if( pf )
//   // {
//   //   ply_file_parse(pf);
//   //   const char* positions_names[] = {"x","y","z"};
//   //   const char* vertex_indices_names[] = {"vertex_indices"};
//   //   ply_file_get_property_from_element(pf, "vertex", positions_names, 3, PLY_FLOAT, PLY_INVALID, 
//   //                                     (void**)&positions, NULL, &n_verts );
//   //   ply_file_get_property_from_element(pf, "face", vertex_indices_names, 1, PLY_INT32, PLY_UINT8, 
//   //                                      (void**)&indices, NULL, &n_faces);

//   //   msh_cprintf( opts.verbose, "Vertex count: %d\n", n_verts );
//   //   msh_cprintf( opts.verbose, "Face Count: %d\n", n_faces );
//   // }
//   // ply_file_print_header(pf);
//   // t2 = msh_get_time(MSHT_MILLISECONDS);
//   // msh_cprintf(opts.verbose, "Mesh reading: %fms.\n", t2-t1);
//   // ply_file_close(pf);
// //// SOKOL STUFF

//  /* cube vertex buffer */
//     float vertices[] = {
//         /* pos                  color                       uvs */
//         -1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 0.0f,
//          1.0f, -1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 0.0f,
//          1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
//         -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.0f, 1.0f,     0.0f, 1.0f,

//         -1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 0.0f, 
//          1.0f, -1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 0.0f,
//          1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     1.0f, 1.0f,
//         -1.0f,  1.0f,  1.0f,    0.0f, 1.0f, 0.0f, 1.0f,     0.0f, 1.0f,

//         -1.0f, -1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 0.0f,
//         -1.0f,  1.0f, -1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 0.0f,
//         -1.0f,  1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     1.0f, 1.0f,
//         -1.0f, -1.0f,  1.0f,    0.0f, 0.0f, 1.0f, 1.0f,     0.0f, 1.0f,

//          1.0f, -1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 0.0f,
//          1.0f,  1.0f, -1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 0.0f,
//          1.0f,  1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     1.0f, 1.0f,
//          1.0f, -1.0f,  1.0f,    1.0f, 0.5f, 0.0f, 1.0f,     0.0f, 1.0f,

//         -1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 0.0f,
//         -1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 0.0f,
//          1.0f, -1.0f,  1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     1.0f, 1.0f,
//          1.0f, -1.0f, -1.0f,    0.0f, 0.5f, 1.0f, 1.0f,     0.0f, 1.0f,

//         -1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 0.0f,
//         -1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 0.0f,
//          1.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     1.0f, 1.0f,
//          1.0f,  1.0f, -1.0f,    1.0f, 0.0f, 0.5f, 1.0f,     0.0f, 1.0f
//     };
//     sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
//         .size = sizeof(vertices),
//         .content = vertices,
//     });

//     /* create an index buffer for the cube */
//     uint16_t indices2[] = {
//         0, 1, 2,  0, 2, 3,
//         6, 5, 4,  7, 6, 4,
//         8, 9, 10,  8, 10, 11,
//         14, 13, 12,  15, 14, 12,
//         16, 17, 18,  16, 18, 19,
//         22, 21, 20,  23, 22, 20
//     };
//     sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
//         .type = SG_BUFFERTYPE_INDEXBUFFER,
//         .size = sizeof(indices2),
//         .content = indices2,
//     });

//     /* create a checkerboard texture */
//     uint32_t pixels[4*4] = {
//         0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
//         0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
//         0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
//         0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
//     };
//     sg_image img = sg_make_image(&(sg_image_desc){
//         .width = 4,
//         .height = 4,
//         .pixel_format = SG_PIXELFORMAT_RGBA8,
//         .min_filter = SG_FILTER_LINEAR,
//         .mag_filter = SG_FILTER_LINEAR,
//         .content.subimage[0][0] = {
//             .ptr = pixels,
//             .size = sizeof(pixels)
//         }
//     });

//     /* create shader */
//     sg_shader shd = sg_make_shader(&(sg_shader_desc){
//         .vs.uniform_blocks[0] = { 
//             .size = sizeof(params_t),
//             .uniforms = {
//                 [0] = { .name="mvp", .type=SG_UNIFORMTYPE_MAT4 }
//             },
//         },
//         .fs.images[0] = { .name="tex", .type=SG_IMAGETYPE_2D },
//         .vs.source =
//             "#version 330\n"
//             "uniform mat4 mvp;\n"
//             "layout(location = 0) in vec4 position;\n"
//             "layout(location = 1) in vec4 color0;\n"
//             "layout(location = 2) in vec2 texcoord0;\n"
//             "out vec4 color;\n"
//             "out vec2 uv;"
//             "void main() {\n"
//             "  gl_Position = mvp * position;\n"
//             "  color = color0;\n"
//             "  uv = texcoord0 * 5.0;\n"
//             "}\n",
//         .fs.source =
//             "#version 330\n"
//             "uniform sampler2D tex;"
//             "in vec4 color;\n"
//             "in vec2 uv;\n"
//             "out vec4 frag_color;\n"
//             "void main() {\n"
//             "  frag_color = texture(tex, uv) * color;\n"
//             "}\n"
//     });

//     /* create pipeline object */
//     sg_pipeline pip = sg_make_pipeline(&(sg_pipeline_desc){
//         .layout = {
//             /* on GL3.3 we can ommit the vertex attribute name if the 
//                vertex shader explicitely defines the attribute location
//                via layout(location = xx), and since the vertex layout
//                has no gaps, we don't need to give the vertex stride
//                or attribute offsets
//             */
//             .attrs = {
//                 [0] = { .format=SG_VERTEXFORMAT_FLOAT3 },
//                 [1] = { .format=SG_VERTEXFORMAT_FLOAT4 },
//                 [2] = { .format=SG_VERTEXFORMAT_FLOAT2 }
//             }
//         },
//         .shader = shd,
//         .index_type = SG_INDEXTYPE_UINT16,
//         .depth_stencil = {
//             .depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL,
//             .depth_write_enabled = true
//         },
//         .rasterizer.cull_mode = SG_CULLMODE_BACK
//     });

//     /* draw state struct with resource bindings */
//     sg_draw_state draw_state = {
//         .pipeline = pip,
//         .vertex_buffers[0] = vbuf,
//         .index_buffer = ibuf,
//         .fs_images[0] = img
//     };

//     /* default pass action */
//     sg_pass_action pass_action = { 0 };
// ////////////
// printf("TEST\n");
//   msh_mat4_t proj = msh_perspective(msh_deg2rad(60.0), 640.0f/480.0f, 0.01f, 10.0f);
//   msh_mat4_t view = msh_look_at(msh_vec3(0.0f, 1.5f, 6.0f), msh_vec3(0.0f, 0.0f, 0.0f), msh_vec3(0.0f, 1.0f, 0.0f));
//   msh_mat4_t view_proj = msh_mat4_mul(proj, view);

//   params_t vs_params;
//   float rx = 0.0f, ry = 0.0f;
//   while( !glfwWindowShouldClose(w) )
//   {
//     rx += 1.0f; ry += 2.0f;
//     msh_mat4_t rxm = msh_rotate( msh_mat4_identity(), rx, msh_vec3(1.0f, 0.0f, 0.0f));
//     msh_mat4_t rym = msh_rotate( msh_mat4_identity(), ry, msh_vec3(0.0f, 1.0f, 0.0f));
//     msh_mat4_t model = msh_mat4_mul(rxm, rym);

//     /* model-view-projection matrix for vertex shader */
//     vs_params.mvp = msh_mat4_mul(view_proj, model);

//     int cur_width, cur_height;
//     glfwGetFramebufferSize(w, &cur_width, &cur_height);
//     sg_begin_default_pass(&pass_action, cur_width, cur_height);
//     sg_apply_draw_state(&draw_state);
//     sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
//     sg_draw(0, 36, 1);
//     sg_end_pass();
//     sg_commit();
    
//     glfwSwapBuffers(w);
//     glfwPollEvents();
//   }
//   glfwDestroyWindow(w);
//   glfwTerminate();



//   return 0;
// }