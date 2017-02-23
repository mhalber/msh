/*
  ==============================================================================
  
  MSH_GFX.H - WIP!
  
  A single header library that wraps OpenGL. This library should be providing two
  APISs, with two purposes:
    - High Level API - Streamline common OpenGL operation
    - Low Level API - Layer of abstraction for different OpenGL context 
                      (3.3+, 2.1, ES), or even different rendering APIs

  What the low level api aims to be is close to Branimir Karadzic bgfx.
  What the high level api aims to be is close to Ricardo Cabello(mrdoob) three.js
  But in C11... ¯\_(ツ)_/¯ We'll see how it goes. Currently it is a bit of a
  hodgepodge and I would not recommend using it.

  To use the library you simply add:
  
  #define MSH_GFX_IMPLEMENTATION
  #include "msh_gfx.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh_gfx.h"

  ==============================================================================
  DEPENDENCIES

  This library depends on following standard headers:
    <stdlib.h>
    <assert.h>

  By default this library does not import these headers. Please see 
  docs/no_headers.md for explanation. Importing heades is enabled by:

  #define MSH_GFX_INCLUDE_HEADES

  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)

  ==============================================================================
  LICENSE

  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.

  ==============================================================================
  NOTES 

  1. This currently is a wrapper around the opengl. What it should be
     is a API agnostic renderer -> Similar to bgfx / nanovg.
  2. Currently windowing is a thin wrapper over GLFW. There is some 
     functionality added for handling multiple windows. This will be removed. 

  ==============================================================================

  TODO: Modify shader code to enable use of glProgramPipeline
  TODO: Add texture format declaration
  TODO: For geometry, need to experiment with glMapBufferRange
  TODO: Add informative error messages. Make them optional
  TODO: Add lower-level wrapper for geometry handling
  TODO: Add higher level stuff for wrapping materials
  TODO: Remove the mshgfx_window stuff and replace it with glfw
  TODO: Think more about the viewport design

  ==============================================================================
  REFERENCES:

 */


/*
 * =============================================================================
 *       INCLUDES, TYPES AND DEFINES
 * =============================================================================
 */

#ifndef MSH_GFX_H
#define MSH_GFX_H

#include <stdlib.h>
#include <assert.h>

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#ifdef __linux__
  #include "glad/glad.h"
#endif

//#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#ifndef MSH_VEC_MATH

typedef union vec2
{
  struct { float x; float y; };
  float data[2];
} msh_vec2_t;

#endif


enum mshgfx_shader_type
{
  VERTEX_SHADER,
  FRAGMENT_SHADER,
  GEOMETRY_SHADER,
  TESS_CONTROL_SHADER,
  TESS_EVALUATION_SHADER
};

enum msh_geometry_properties_flags_
{
  POSITION  = 1 << 0, /* layout( location = 0 ) */
  NORMAL    = 1 << 1, /* layout( location = 1 ) */
  TANGENT   = 1 << 2, /* layout( location = 2 ) */
  TEX_COORD = 1 << 3, /* layout( location = 3 ) */
  COLOR_A   = 1 << 4, /* layout( location = 4 ) */
  COLOR_B   = 1 << 5, /* layout( location = 5 ) */
  COLOR_C   = 1 << 6, /* layout( location = 6 ) */
  COLOR_D   = 1 << 7, /* layout( location = 7 ) */
  STRUCTURED = 1 << 8,

  SIMPLE_MESH   = POSITION | NORMAL | STRUCTURED,
  POINTCLOUD    = POSITION
};


typedef GLFWwindow mshgfx_window_t ;

typedef struct msh_viewport
{
  msh_vec2_t p1;
  msh_vec2_t p2;
} msh_viewport_t;

typedef struct mshgfx_shader
{
  GLuint id;
  enum mshgfx_shader_type type;
  GLint compiled;
} mshgfx_shader_t;


typedef struct mshgfx_shader_prog
{
  GLuint id;
  GLint linked;
} mshgfx_shader_prog_t;

typedef struct mshgfx_texture
{
  unsigned int id;
  int width;
  int height;
  int n_comp;
  GLenum type;
  unsigned int unit;
} mshgfx_texture_t;

typedef int msh_internal_data_type;
typedef int msh_geometry_properties_flags;

typedef struct msh_framebuffer
{
  GLuint id;
  GLuint depthrenderbuffer;
  int width;
  int height;
} mshgfx_framebuffer_t;


typedef struct mshgfx_geometry_data
{
  float * positions;
  float * normals;
  float * tangents;
  float *texcoords;
  unsigned char  * colors_a;
  unsigned char  * colors_b;
  unsigned char  * colors_c;
  unsigned char  * colors_d;
  unsigned int * indices;
  int n_vertices;
  int n_elements;
} mshgfx_geometry_data_t;

typedef struct mshgfx_geometry
{
  unsigned int vao;
  unsigned int vbo;
  unsigned int ebo;
  int n_indices;
  int n_elements;
  int buffer_size;
  msh_geometry_properties_flags flags;
} mshgfx_geometry_t;

/*
 * =============================================================================
 *       WINDOWING/CONTEXT
 * =============================================================================
 */

mshgfx_window_t * mshgfx_window_create( const char *title, int pos_x, int pos_y, 
                                                      int res_x, int res_y );
int mshgfx_window_destroy( mshgfx_window_t *window );
int mshgfx_window_display( mshgfx_window_t *window, int (*display_function)(void) );
int mshgfx_window_is_any_open( mshgfx_window_t **windows, const int n_windows );
void mshgfx_window_poll_events(void);
void mshgfx_window_terminate(void);

void mshgfx_window_activate( mshgfx_window_t *window )
{
  glfwMakeContextCurrent( window );
}

void mshgfx_window_set_callback_window_size( mshgfx_window_t *window, 
              void (*callback)( mshgfx_window_t * window, int width, int height ) )
{
  glfwSetWindowSizeCallback( window, callback );
}

void mshgfx_window_set_callback_framebuffer_size( mshgfx_window_t *window, 
              void (*callback)( mshgfx_window_t * window, int width, int height ) )
{
  glfwSetFramebufferSizeCallback( window, callback );
}

void mshgfx_window_set_callback_refresh( mshgfx_window_t *window, 
                                     void (*callback)( mshgfx_window_t * window ) )
{
  glfwSetWindowRefreshCallback( window, callback );
}



/*
 * =============================================================================
 *       FRAMEBUFFER 
 * =============================================================================
 */

int mshgfx_framebuffer_init( mshgfx_framebuffer_t * fb, int width, int height );
int mshgfx_framebuffer_attach( mshgfx_texture_t *tex, unsigned int attachements,
                            int n_textures );
int mshgfx_framebuffer_bind( mshgfx_framebuffer_t * fb );
int mshgfx_framebuffer_attach_color_texture( mshgfx_framebuffer_t *fb, 
                                          mshgfx_texture_t *tex,
                                          GLuint *attachement, int n );
int mshgfx_framebuffer_attach_depth_texture( mshgfx_framebuffer_t *fb, 
                                          mshgfx_texture_t *tex );
int mshgfx_framebuffer_add_color_renderbuffer( mshgfx_framebuffer_t *fb );
int mshgfx_framebuffer_add_depth_renderbuffer( mshgfx_framebuffer_t *fb );
int mshgfx_framebuffer_check_status( mshgfx_framebuffer_t * fb );
int mshgfx_framebuffer_terminate( mshgfx_framebuffer_t *fb );


/*
 * =============================================================================
 *       VIEWPORTS 
 * =============================================================================
 */

int  msh_viewport_init( msh_viewport_t *v, msh_vec2_t p1, msh_vec2_t p2 );
void msh_viewport_begin( const msh_viewport_t *v);
void msh_viewport_end();


void mshgfx_background_flat4f( float r, float g, float b, float a);
void mshgfx_background_gradient4f( float r1, float g1, float b1, float a1,
                                   float r2, float g2, float b2, float a2 );
void mshgfx_background_tex( mshgfx_texture_t *tex );

#ifdef MSH_VEC_MATH
void mshgfx_background_flat4v( msh_vec4_t col );
void mshgfx_background_gradient4fv( msh_vec4_t col1, msh_vec4_t col2 );
#endif /*MSH_VEC_MATH*/


/*
 * =============================================================================
 *       SHADERS
 * =============================================================================
 */

#define MSHGFX_SHADER_HEAD "#version 330 core\n"
#define MSHGFX_SHADER_STRINGIFY(x) #x

int mshgfx_shader_prog_create_from_source_vf( mshgfx_shader_prog_t *p,
                                           const char *vs_source,
                                           const char *fs_source );

int mshgfx_shader_prog_create_from_source_vgf( mshgfx_shader_prog_t *p,
                                            const char *vs_source,
                                            const char *gs_source,
                                            const char *fs_source );

int mshgfx_shader_prog_create_from_files_vf( mshgfx_shader_prog_t *p,
                                          const char * vs_filename,
                                          const char * fs_filename );

int mshgfx_shader_prog_create_from_files_vgf( mshgfx_shader_prog_t *p,
                                           const char * vs_filename,
                                           const char * gs_filename,
                                           const char * fs_filename );

int mshgfx_shader_compile( mshgfx_shader_t *s, const char * source );

int mshgfx_shader_prog_link_vf( mshgfx_shader_prog_t *p,
                             const mshgfx_shader_t *vs, 
                             const mshgfx_shader_t *fs );

int mshgfx_shader_prog_link_vgf( mshgfx_shader_prog_t *p,
                              const mshgfx_shader_t *vs, 
                              const mshgfx_shader_t *gs, 
                              const mshgfx_shader_t *fs );

int mshgfx_shader_prog_use( const mshgfx_shader_prog_t *p );


void mshgfx_shader_prog_set_uniform_1f( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x );
void mshgfx_shader_prog_set_uniform_1i( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x );
void mshgfx_shader_prog_set_uniform_1u( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x );

void mshgfx_shader_prog_set_uniform_2f( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y );
void mshgfx_shader_prog_set_uniform_2i( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y );
void mshgfx_shader_prog_set_uniform_2u( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x, unsigned int y );

void mshgfx_shader_prog_set_uniform_3f( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y, float z );
void mshgfx_shader_prog_set_uniform_3i( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y, int z );
void mshgfx_shader_prog_set_uniform_3u( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x, unsigned int y, 
                                     unsigned int z );

void mshgfx_shader_prog_set_uniform_4f( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y, float z, float w );
void mshgfx_shader_prog_set_uniform_4i( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y, int z, int w );
void mshgfx_shader_prog_set_uniform_4u( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x, unsigned int y, 
                                     unsigned int z, unsigned int w );

void mshgfx_shader_prog_set_uniform_fv( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     const float *val, 
                                     const unsigned int count );
void mshgfx_shader_prog_set_uniform_iv( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     const int *val, 
                                     const unsigned int count );
void mshgfx_shader_prog_set_uniform_uv( const mshgfx_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     const unsigned int *val, 
                                     const unsigned int count );

#ifdef MSH_VEC_MATH
void mshgfx_shader_prog_set_uniform_2fv( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_vec2_t *v );
void mshgfx_shader_prog_set_uniform_2fvc( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_vec2_t *v, 
                                 const unsigned int count );
void mshgfx_shader_prog_set_uniform_3fv( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_vec3_t *v );
void mshgfx_shader_prog_set_uniform_3fvc( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_vec3_t *v, 
                                 const unsigned int count );
void mshgfx_shader_prog_set_uniform_4fv( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_vec4_t *v);
void mshgfx_shader_prog_set_uniform_4fvc( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_vec4_t *v, 
                                  const unsigned int count );

void mshgfx_shader_prog_set_uniform_3fm( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_mat3_t *m );
void mshgfx_shader_prog_set_uniform_3fmc( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_mat3_t *m,
                                 const unsigned int count );
void mshgfx_shader_prog_set_uniform_4fm( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_mat4_t *m );
void mshgfx_shader_prog_set_uniform_4fmc( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_mat4_t *m,
                                 const unsigned int count );
#endif


/*
 * =============================================================================
 *       GPU GEOMETRY
 * =============================================================================
 */

int mshgfx_geometry_update( const mshgfx_geometry_t * geo, 
                            const mshgfx_geometry_data_t * host_data, 
                            const int flags );

int mshgfx_geometry_init( mshgfx_geometry_t * geo, 
                          const mshgfx_geometry_data_t * host_data,   
                          const int flags );

int mshgfx_geometry_free( mshgfx_geometry_t * geo );

void mshgfx_geometry_draw( mshgfx_geometry_t * geo, GLenum draw_mode );

/*
 * =============================================================================
 *       TEXTURES
 * =============================================================================
 */

void mshgfx_texture_init_u8( mshgfx_texture_t *tex,                              
                             const unsigned char *data, 
                             const int w, 
                             const int h, 
                             const int n_comp, 
                             const unsigned int unit );

void mshgfx_texture_init_u16( mshgfx_texture_t *tex,
                              const unsigned short *data, 
                              const int w, 
                              const int h, 
                              const int n_comp, 
                              const unsigned int unit );

void mshgfx_texture_init_u32( mshgfx_texture_t *tex,
                              const unsigned int *data, 
                              const int w, 
                              const int h, 
                              const int n_comp, 
                              const unsigned int unit );


void mshgfx_texture_init_r32( mshgfx_texture_t *tex,
                              const float *data, 
                              const int w, 
                              const int h, 
                              const int n_comp, 
                              const unsigned int unit );


void mshgfx_texture_update_u8(  mshgfx_texture_t *tex, const unsigned char *data );
void mshgfx_texture_update_u16( mshgfx_texture_t *tex, const unsigned short *data );
void mshgfx_texture_update_u32( mshgfx_texture_t *tex, const unsigned int *data );
void mshgfx_texture_update_r32( mshgfx_texture_t *tex, const float *data );

void mshgfx_texture_use( const mshgfx_texture_t *tex );

void mshgfx_texture_free( mshgfx_texture_t *tex );

#endif /* MSH_GFX_H */




#ifdef MSH_GFX_IMPLEMENTATION

/*
 * =============================================================================
 *       HELPERS
 * =============================================================================
 */

static void 
msh__check_gl_error(const char* str)
{
	GLenum err;
	err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "Error %08x after %s\n", err, str);
		return;
	}
}

/*
 * =============================================================================
 *       WINDOWING/CONTEXT IMPLEMENTATION
 * =============================================================================
 */


mshgfx_window_t * 
mshgfx_window_create( const char * title, int pos_x, int pos_y, 
                                       int res_x, int res_y )
{
  mshgfx_window_t * window = NULL;

  /* attempt to initialize glfw library */
  if(!glfwInit()) return 0;

  /* setup some hints - this is osx specific */
  /* 
    NOTE: This is a contract, probably needs to be specified once. bit like a 
    state machine
   */
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
  
  /* actually create window */
  window = glfwCreateWindow( res_x, res_y, title, NULL, NULL );
  if(!window) return NULL;

#ifdef __linux__
  glfwMakeContextCurrent(window);
  if(!gladLoadGL()) {
    printf("Something went wrong!\n");
    exit(-1);
  }
#endif

  /* change position */
  if( pos_x >= 0 && pos_y >= 0 )
  {
    glfwSetWindowPos( window, pos_x, pos_y );
  }
  
  return window;
}

int 
mshgfx_window_destroy( mshgfx_window_t * window )
{
  if ( window )
  {
    glfwDestroyWindow( window );
    return 1;
  }
  return 0;
}

int 
mshgfx_window_display( mshgfx_window_t * window,
                    int (*display_function)(void) )
{
  glfwMakeContextCurrent(window);
  if ( !display_function() )
  {
    return 0;
  }
  glfwSwapBuffers(window);
  return 1;
}

int 
mshgfx_window_is_any_open( mshgfx_window_t ** windows, const int n_windows )
{
  int is_any_open = 0;
  for ( int i = 0 ; i < n_windows ; ++i )
  {
    int is_valid = windows[i] ? 1 : 0;
    if ( is_valid )
    {
      int should_close = glfwWindowShouldClose( windows[i] );
      if ( should_close )
      {
         mshgfx_window_destroy( windows[i] );
         windows[i] = NULL;
      }
    }
    is_any_open |= is_valid;
   }
  return is_any_open;
 }
 
void 
mshgfx_window_poll_events( void )
{
  glfwPollEvents();
}

void 
mshgfx_window_terminate( void )
{
  glfwTerminate();
}



/*
 * =============================================================================
 *       FRAMEBUFFERS IMPLEMENTATION
 * =============================================================================
 */

int 
mshgfx_framebuffer_init( mshgfx_framebuffer_t *fb, int width, int height )
{
  fb->width = width;
  fb->height = height;
  glGenFramebuffers(1, &fb->id);
  glBindFramebuffer(GL_FRAMEBUFFER, fb->id);

  return 1;
}

int 
mshgfx_framebuffer_bind( mshgfx_framebuffer_t *fb )
{
  if (fb) glBindFramebuffer(GL_FRAMEBUFFER, fb->id);
  else    glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return 1;
}

int
mshgfx_framebuffer_attach_color_texture( mshgfx_framebuffer_t *fb, 
                                      mshgfx_texture_t *tex,
                                      GLuint *attachement, int n )
{
  if(!fb) return 0;
  // NOTE: Research what is the difference between glFramebufferTexture2D and glFramebufferTexture
  for( int i = 0 ; i < n ; ++i )
  {
    // NOTE: We should test if texture sizes are the same.
    glFramebufferTexture2D( GL_FRAMEBUFFER, attachement[i], 
                            GL_TEXTURE_2D, tex[i].id, 0);
  }
  glDrawBuffers( n, attachement );
  return 1;
}

int mshgfx_framebuffer_attach_depth_texture( mshgfx_framebuffer_t *fb, 
                                          mshgfx_texture_t *tex )
{
  if(!fb) return 0;
  // NOTE: We should test if texture sizes are the same
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->id, 0 );
  return 1;
}

int
mshgfx_framebuffer_add_color_renderbuffer(mshgfx_framebuffer_t *fb)
{
  if(!fb) return 0;
  // NOTE: TO BE IMPLEMENTED
  return 0;
}

int
mshgfx_framebuffer_add_depth_renderbuffer( mshgfx_framebuffer_t *fb )
{
  if(!fb) return 0;
  glGenRenderbuffers(1, &fb->depthrenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, fb->depthrenderbuffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
                                                         fb->width, fb->height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                                       GL_RENDERBUFFER, fb->depthrenderbuffer);
  return 1;
}

int 
mshgfx_framebuffer_check_status( mshgfx_framebuffer_t * fb )
{
  if( !fb ) return 0;
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    printf( "%s\n", "Framebuffer not complete!" );
  }
  return 0;
}

int 
mshgfx_framebuffer_resize( mshgfx_framebuffer_t *fb, int width, int height )
{
  if(!fb) return 0;
  
  // delete old
  if( fb->depthrenderbuffer ) glDeleteRenderbuffers(1, &fb->depthrenderbuffer );
  glDeleteFramebuffers( 1, &fb->id );

  msh__check_gl_error("framebuffer resize"); 

  // reinitialize
  return mshgfx_framebuffer_init( fb, width, height );
}

int
mshgfx_framebuffer_terminate(mshgfx_framebuffer_t *fb)
{
  if(!fb) return 0;
  if( fb->depthrenderbuffer ) glDeleteRenderbuffers(1, &fb->depthrenderbuffer );
  glDeleteFramebuffers( 1, &fb->id );
  msh__check_gl_error("framebuffer termination");
  return 1;
}


/*
 * =============================================================================
 *       VIEWPORTS IMPLEMENTATION
 * =============================================================================
 */

int msh_viewport_init(msh_viewport_t *v, msh_point2_t p1, msh_point2_t p2)
{
  glEnable(GL_SCISSOR_TEST);
  v->p1 = p1;
  v->p2 = p2;
  return 1;
}

void msh_viewport_begin( const msh_viewport_t *v)
{
  glScissor( v->p1.x, v->p1.y, v->p2.x, v->p2.y );
  glViewport( v->p1.x, v->p1.y, v->p2.x, v->p2.y );
}

void msh_viewport_end()
{
  glScissor( 0, 0, 0, 0 );
  glViewport( 0, 0, 0, 0 );
}

void 
mshgfx_background_flat4f( float r, float g, float b, float a )
{
  glClearColor( r, g, b, a );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void 
mshgfx_background_gradient4f( float r1, float g1, float b1, float a1,
                              float r2, float g2, float b2, float a2 )
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  
  static GLuint background_vao = 0;
  static mshgfx_shader_prog_t background_shader;
  if (background_vao == 0)
  {
    glGenVertexArrays(1, &background_vao);
  
    // vertex shader ( full screen quad by Morgan McGuire)
    char* vs_src = (char*) MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY
    (
      out vec2 v_texcoords;
      void main()
      {
        uint idx = uint( gl_VertexID % 3 );
        gl_Position = vec4(
            (float(   idx        & 1U ) ) * 4.0 - 1.0,
            (float( ( idx >> 1U )& 1U ) ) * 4.0 - 1.0,
            0.0, 1.0);
        v_texcoords = gl_Position.xy * 0.5 + 0.5;
      }
    );

    char* fs_src = (char*) MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY
    (
      in vec2 v_texcoords;
      uniform vec4 col_top;
      uniform vec4 col_bot;
      out vec4 frag_color;
      void main()
      {
        frag_color = v_texcoords.y * col_top + (1.0 - v_texcoords.y) * col_bot;
      }
    );
    mshgfx_shader_prog_create_from_source_vf( &background_shader, vs_src, fs_src );
  }

  mshgfx_shader_prog_use( &background_shader );
  mshgfx_shader_prog_set_uniform_4f( &background_shader, "col_top", r1, g1, b1, a1 );
  mshgfx_shader_prog_set_uniform_4f( &background_shader, "col_bot", r2, g2, b2, a2 );
  glBindVertexArray( background_vao );
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

  glEnable(GL_DEPTH_TEST);
}

void 
mshgfx_background_tex( mshgfx_texture_t *tex )
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  static GLuint background_vao = 0;
  static mshgfx_shader_prog_t background_shader;

  if (background_vao == 0)
  {
    glGenVertexArrays(1, &background_vao);
  
    // vertex shader (full screen quad by Morgan McGuire)
    char* vs_src = (char*) MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY
    (
      out vec2 v_texcoords;
      void main()
      {
        uint idx = uint( gl_VertexID % 3 );
        gl_Position = vec4(
            (float(   idx        & 1U ) ) * 4.0 - 1.0,
            (float( ( idx >> 1U )& 1U ) ) * 4.0 - 1.0,
            0.0, 1.0);
        v_texcoords = gl_Position.xy * 0.5 + 0.5;
      }
    );

    char* fs_src = (char*) MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY
    (
      in vec2 v_texcoords;
      uniform sampler2D fb_tex;
      out vec4 frag_color;
      void main()
      {
        vec3 tex_color = texture( fb_tex, v_texcoords ).rgb;
        frag_color = vec4( tex_color.r, tex_color.g, tex_color.b, 1.0f );
      }
    );
    mshgfx_shader_prog_create_from_source_vf( &background_shader, vs_src, fs_src );
  }
  
  mshgfx_shader_prog_use( &background_shader );
  mshgfx_texture_use( tex );
  mshgfx_shader_prog_set_uniform_1i( &background_shader, "fb_tex", tex->unit );

  glBindVertexArray( background_vao );
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);
  glEnable(GL_DEPTH_TEST);
}

#ifdef MSH_VEC_MATH
void 
mshgfx_background_flat4fv( msh_vec4_t col )
{
  glClearColor( col.x, col.y, col.z, col.w );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void 
mshgfx_background_gradient4fv( msh_vec4_t col_top, msh_vec4_t col_bot )
{
  mshgfx_background_gradient4f( col_top.x, col_top.y, col_top.z, col_top.w,
                             col_bot.x, col_bot.y, col_bot.z, col_bot.w );
}
#endif

/*
 * =============================================================================
 *       SHADERS IMPLEMENTATION
 * =============================================================================
 */


int
msh__shader_text_file_read( const char * filename, char ** file_contents )
{
  FILE *fp;
  long l_size;

  fp = fopen ( filename , "r" );
  if( !fp ) perror(filename),exit(1);

  fseek( fp , 0L , SEEK_END);
  l_size = ftell( fp );
  rewind( fp );

  /* allocate memory for entire content */ 
  (*file_contents) = (char*)calloc( 1, l_size + 1 );
  if ( !(*file_contents) )
  { 
    fclose( fp ); 
    fputs( "memory alloc fails", stderr ); 
    exit( 1 );
  }

  /* copy the file into the buffer */
  if ( fread( (*file_contents), l_size, 1, fp) != 1 )
  {
    fclose(fp); 
    free( (*file_contents) ); 
    fputs( "entire read fails",stderr );
    exit( 1 );
  }

  fclose(fp);
  return 1;
}


int
mshgfx_shader_compile( mshgfx_shader_t *s, const char * source )
{
  switch ( s->type ) {
      case VERTEX_SHADER:
          s->id = glCreateShader( GL_VERTEX_SHADER );
          break;
      case FRAGMENT_SHADER:
          s->id = glCreateShader( GL_FRAGMENT_SHADER );
          break;

    #ifndef __EMSCRIPTEN__
      case GEOMETRY_SHADER:
          s->id = glCreateShader( GL_GEOMETRY_SHADER );
          break;
      case TESS_CONTROL_SHADER:
          s->id = glCreateShader( GL_TESS_CONTROL_SHADER );
          break;
      case TESS_EVALUATION_SHADER:
          s->id = glCreateShader( GL_TESS_EVALUATION_SHADER );
          break;
    #endif

      default:
          return 0;
  }

  glShaderSource(s->id, 1, &source, NULL);

  glCompileShader(s->id);
  glGetShaderiv(s->id, GL_COMPILE_STATUS, &s->compiled);

  if(!s->compiled)
  {
    GLint log_len;
    glGetShaderiv(s->id, GL_INFO_LOG_LENGTH, &log_len);
    if ( log_len > 0 )
    {
      char* log_str = (char*)malloc(log_len);
      GLsizei written;
      glGetShaderInfoLog(s->id, log_len, &written, log_str);
      char * mshgfx_shader_type_str = NULL;
      switch(s->type)
      {
        case VERTEX_SHADER:
          mshgfx_shader_type_str = (char*)"Vertex Shader";
          break;
        case FRAGMENT_SHADER:
          mshgfx_shader_type_str = (char*)"Fragment Shader";
          break;
        case GEOMETRY_SHADER:
          mshgfx_shader_type_str = (char*)"Geometry Shader";
          break;
        case TESS_CONTROL_SHADER:
          mshgfx_shader_type_str = (char*)"Tess Control Shader";
          break;
        case TESS_EVALUATION_SHADER:
          mshgfx_shader_type_str = (char*)"Tess Evaluation Shader";
          break;
      }
      printf( "%s Compilation Failure :\n%s\n", mshgfx_shader_type_str, log_str );
      free( log_str );
    }
    return 0;
  }

  return s->compiled;
}

int 
mshgfx_shader_prog_link_vf( mshgfx_shader_prog_t *p,
                         const mshgfx_shader_t *vs, 
                         const mshgfx_shader_t *fs )
{
  /* NOTE: change to asserts? */
  if ( !vs->compiled || !fs->compiled )
  {
    printf( "Compile shaders before linking program!\n" );
    return 1;
  }

  p->id = glCreateProgram();
  if ( p->id == 0 )
  {
    printf ( " Failed to create program (%d)!\n", p->id );
    return 0;
  }

  glAttachShader( p->id, vs->id );
  glAttachShader( p->id, fs->id );

  glLinkProgram( p->id );
  glGetProgramiv( p->id, GL_LINK_STATUS, &p->linked );

  if ( !p->linked )
  {
    GLint log_len;
    glGetProgramiv( p->id, GL_INFO_LOG_LENGTH, &log_len );
    if( log_len > 0 )
    {
      char* log_str = (char*) malloc( log_len );
      GLsizei written;
      glGetProgramInfoLog( p->id, log_len, &written, log_str );
      printf( "Program Linking Failure : %s\n", log_str );
      free( log_str );
    }
  }

  glDetachShader( p->id, fs->id );
  glDetachShader( p->id, vs->id );

  glDeleteShader( fs->id );
  glDeleteShader( vs->id );

  return p->linked;
}


int
mshgfx_shader_prog_link_vgf( mshgfx_shader_prog_t *p,
                          const mshgfx_shader_t *vs, 
                          const mshgfx_shader_t *gs, 
                          const mshgfx_shader_t *fs )
{
  /* NOTE: change to asserts? */
  if ( !vs->compiled || !fs->compiled )
  {
    printf( "Compile shaders before linking program!\n" );
    return 1;
  }

  p->id = glCreateProgram();
  if ( p->id == 0 )
  {
    printf ( " Failed to create program!\n" );
    return 0;
  }

  glAttachShader( p->id, vs->id );
  glAttachShader( p->id, gs->id );
  glAttachShader( p->id, fs->id );

  glLinkProgram( p->id );
  glGetProgramiv( p->id, GL_LINK_STATUS, &p->linked );

  if ( !p->linked )
  {
    GLint log_len;
    glGetProgramiv( p->id, GL_INFO_LOG_LENGTH, &log_len );
    if( log_len > 0 )
    {
      char* log_str = (char*) malloc( log_len );
      GLsizei written;
      glGetProgramInfoLog( p->id, log_len, &written, log_str );
      printf( "Program Linking Failure : %s\n", log_str );
      free( log_str );
    }
  }

  glDetachShader( p->id, fs->id );
  glDetachShader( p->id, gs->id );
  glDetachShader( p->id, vs->id );

  glDeleteShader( fs->id );
  glDeleteShader( gs->id );
  glDeleteShader( vs->id );

  return p->linked;
}

int
mshgfx_shader_prog_use( const mshgfx_shader_prog_t *p )
{
  if ( p->linked ) {
    glUseProgram( p->id );
    return 1;
  }
  return 0;
}

int
mshgfx_shader_prog_create_from_source_vf( mshgfx_shader_prog_t *p,
                                       const char *vs_src,
                                       const char *fs_src )
{

  mshgfx_shader_t vs, fs;
  vs.type = VERTEX_SHADER;
  fs.type = FRAGMENT_SHADER;

  if ( !mshgfx_shader_compile( &vs, vs_src ) )        return 0;
  if ( !mshgfx_shader_compile( &fs, fs_src ) )        return 0;
  if ( !mshgfx_shader_prog_link_vf( p, &vs, &fs ) )   return 0;

  return 1;
}

int
mshgfx_shader_prog_create_from_source_vgf( mshgfx_shader_prog_t *p,
                                        const char * vs_src,
                                        const char * gs_src,
                                        const char * fs_src )
{
  mshgfx_shader_t vs, fs, gs;
  vs.type = VERTEX_SHADER;
  fs.type = FRAGMENT_SHADER;
  gs.type = GEOMETRY_SHADER;

  if ( !mshgfx_shader_compile( &vs, vs_src ) )            return 0;
  if ( !mshgfx_shader_compile( &gs, gs_src ) )            return 0;
  if ( !mshgfx_shader_compile( &fs, fs_src ) )            return 0;
  if ( !mshgfx_shader_prog_link_vgf( p, &vs, &gs, &fs ) ) return 0;

  return 1;
}

int
mshgfx_shader_prog_create_from_files_vf( mshgfx_shader_prog_t *p,
                                      const char * vs_filename,
                                      const char * fs_filename )
{
  char *vs_source, *fs_source;
  if( !msh__shader_text_file_read( vs_filename, &vs_source ) ||
      !msh__shader_text_file_read( fs_filename, &fs_source ) )
  {
    return 0;
  }

  if( !mshgfx_shader_prog_create_from_source_vf( p, vs_source, fs_source ))
  {
    return 0;
  }

  free( vs_source );
  free( fs_source );

  return 1;
}

int
mshgfx_shader_prog_create_from_files_vgf( mshgfx_shader_prog_t *p,
                                       const char * vs_filename,
                                       const char * gs_filename,
                                       const char * fs_filename )
{
  char *vs_source, *fs_source, *gs_source;
  if( !msh__shader_text_file_read( vs_filename, &vs_source ) ||
      !msh__shader_text_file_read( gs_filename, &gs_source ) ||
      !msh__shader_text_file_read( fs_filename, &fs_source ) )
  {
    return 0;
  }

  if( !mshgfx_shader_prog_create_from_source_vgf( p, 
                                              vs_source, 
                                              gs_source,
                                              fs_source ) )
  {
    return 0;
  }

  free( vs_source );
  free( fs_source );

  return 1;
}

void
mshgfx_shader_prog_set_uniform_1f( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                float x )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1f( location, (GLfloat)x );
}

void
mshgfx_shader_prog_set_uniform_1i( const mshgfx_shader_prog_t *p, 
                               const char *attrib_name, 
                               int x )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1i( location, (GLint)x );
}

void
mshgfx_shader_prog_set_uniform_1u( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                unsigned int x )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1ui( location, (GLuint)x );
}

void
mshgfx_shader_prog_set_uniform_2f( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                float x, float y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2f( location, x, y );
}

void
mshgfx_shader_prog_set_uniform_2i( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                int x, int y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2i( location, x, y );
}

void
mshgfx_shader_prog_set_uniform_2u( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                unsigned int x, unsigned int y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2ui( location, x, y );
}


void
mshgfx_shader_prog_set_uniform_3f( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                float x, float y, float z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3f( location, x, y, z );
}

void
mshgfx_shader_prog_set_uniform_3i( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                int x, int y, int z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3i( location, x, y, z );
}

void
mshgfx_shader_prog_set_uniform_3u( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                unsigned int x, unsigned int y, unsigned int z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3ui( location, x, y, z );
}

void
mshgfx_shader_prog_set_uniform_4f( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                float x, float y, float z, float w )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4f( location, x, y, z, w );
}

void
mshgfx_shader_prog_set_uniform_4i( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                int x, int y, int z, int w )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4i( location, x, y, z, w );
}

void
mshgfx_shader_prog_set_uniform_4u( const mshgfx_shader_prog_t *p, 
                                const char *attrib_name, 
                                unsigned int x, unsigned int y, 
                                unsigned int z, unsigned int w )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4ui( location, x, y, z, w );
}

#ifdef MSH_VEC_MATH
void
mshgfx_shader_prog_set_uniform_2fv( const mshgfx_shader_prog_t *p, 
                                   const char *attrib_name, const msh_vec2_t* v)
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2fv( location, 1, &(v->data[0]) );
}

void
mshgfx_shader_prog_set_uniform_2fvc( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_vec2_t* v, 
                                  const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2fv( location, count, &( v->data[0] ) );
}

void
mshgfx_shader_prog_set_uniform_3fv( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_vec3_t * v)
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3fv( location, 1, &(v->data[0]) );
}

void
mshgfx_shader_prog_set_uniform_3fvc( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_vec3_t * v, 
                                  const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3fv( location, count, &( v->data[0] ) );
}

void
mshgfx_shader_prog_set_uniform_4fv( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_vec4_t *v )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4fv( location, 1, &(v->data[0]) );
}

void
mshgfx_shader_prog_set_uniform_4fvc( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_vec4_t *v, 
                                  const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4fv( location, count, &(v->data[0]) );
}

void
mshgfx_shader_prog_set_uniform_3fm( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_mat3_t *m )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix3fv( location, 1, GL_FALSE, &(m->data[0]) );
}

void
mshgfx_shader_prog_set_uniform_3fmc( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_mat3_t *m, 
                                  const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix3fv( location, count, GL_FALSE, &(m->data[0]) );
}

void
mshgfx_shader_prog_set_uniform_4fm( const mshgfx_shader_prog_t *p, 
                                 const char *attrib_name, const msh_mat4_t *m )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix4fv( location, 1, GL_FALSE, &(m->data[0]) );
}

void
mshgfx_shader_prog_set_uniform_4fmc( const mshgfx_shader_prog_t *p, 
                                  const char *attrib_name, const msh_mat4_t *m, 
                                  const unsigned int count  )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix4fv( location, count, GL_FALSE, &(m->data[0]) );
}

#endif /* MSH_VEC_MATH */



/*
 * =============================================================================
 *       GPU GEOMETRY IMPLEMENTATION
 * =============================================================================
 */

/* 
   calulates offset of requested flag into buffer stored in geo,
   depending on what has been requested 
*/
unsigned long 
msh__gpu_geo_get_offset( const mshgfx_geometry_t * geo, 
                        const int flag )
{
  unsigned long offset = 0;
  if ( geo->flags & POSITION )
  {
    if ( flag & POSITION ) return offset * geo->n_indices;  
    offset += 3 * sizeof(float); 
  }
  if ( geo->flags & NORMAL )
  {
    if ( flag & NORMAL ) return offset * geo->n_indices;
    offset += 3 * sizeof(float); 
  }
  if ( geo->flags & TANGENT )
  {
    if ( flag & TANGENT ) return offset * geo->n_indices;
    offset += 3 * sizeof(float); 
  }
  if ( geo->flags & TEX_COORD )
  {
    if ( flag & TEX_COORD ) return offset * geo->n_indices;
    offset += 2 * sizeof(float); 
  }
  if ( geo->flags & COLOR_A )
  {
    if ( flag & COLOR_A ) return offset * geo->n_indices;
    offset += 4 * sizeof(unsigned char); 
  }
  if ( geo->flags & COLOR_B )
  {
    if ( flag & COLOR_B ) return offset * geo->n_indices;
    offset += 4 * sizeof(unsigned char); 
  }
  if ( geo->flags & COLOR_C )
  {
    if ( flag & COLOR_C ) return offset * geo->n_indices;
    offset += 4 * sizeof(unsigned char); 
  }
  if ( geo->flags & COLOR_D )
  {
    if ( flag & COLOR_D ) return offset * geo->n_indices;
    offset += 4 * sizeof(unsigned char); 
  }
  return offset * geo->n_indices;
}


int 
mshgfx_geometry_update( const mshgfx_geometry_t * geo,
                        const mshgfx_geometry_data_t * host_data, 
                         const int flags )
{
  
  if( !(flags & geo->flags) )
  {
    /* printf( "Flag mismatch" __LINE__, "Requested flag update was not"
      "part of initialization" ); */
    return 0;
  }

  glBindVertexArray( geo->vao );
  glBindBuffer( GL_ARRAY_BUFFER, geo->vbo );
 
  if ( flags & POSITION )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & POSITION );
    unsigned long current_size = 3 * sizeof(float) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->positions[0]) );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 
                           0, (void*) offset );
  }

  if ( flags & NORMAL )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & NORMAL);
    unsigned long current_size = 3 * sizeof(float) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->normals[0]) );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 
                           0, (void*) offset );
  }

  if ( flags & TANGENT )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & TANGENT);
    unsigned long current_size = 3 * sizeof(float) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->tangents[0]) );
    glEnableVertexAttribArray( 2 );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 
                           0, (void*) offset );
  }

  if ( flags & TEX_COORD )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & TEX_COORD);
    unsigned long current_size = 2 * sizeof(float) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->texcoords[0]) );
    glEnableVertexAttribArray( 3 );
    glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, 
                           0, (void*) offset );
  }

  if ( flags & COLOR_A )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & COLOR_A);
    unsigned long current_size = 4 * sizeof(unsigned char) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_a[0]) );
    glEnableVertexAttribArray( 4 );
    glVertexAttribPointer( 4, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  if ( flags & COLOR_B )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & COLOR_B);
    unsigned long current_size = 4 * sizeof(unsigned char) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_b[0]) );
    glEnableVertexAttribArray( 5 );
    glVertexAttribPointer( 5, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  if ( flags & COLOR_C )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & COLOR_C);
    unsigned long current_size = 4 * sizeof(unsigned char) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_c[0]) );
    glEnableVertexAttribArray( 6 );
    glVertexAttribPointer( 6, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  if ( flags & COLOR_D )
  {
    unsigned long offset = msh__gpu_geo_get_offset( geo, flags & COLOR_D);
    unsigned long current_size = 4 * sizeof(unsigned char) * geo->n_indices;
    glBufferSubData( GL_ARRAY_BUFFER, offset, 
                                      current_size, 
                                      &(host_data->colors_d)[0] );
    glEnableVertexAttribArray( 7 );
    glVertexAttribPointer( 7, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                            0, (void*) offset );
  }

  glBindVertexArray(0);
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

  return 1;
}


/* TODO: Errors if creating data fails!! */
int 
mshgfx_geometry_init( mshgfx_geometry_t * geo, 
                      const mshgfx_geometry_data_t * host_data, 
                      const int flags )
{
  /* Always use position */
  assert( flags & POSITION );

  glGenVertexArrays( 1, &(geo->vao)  );
  glGenBuffers( 1, &(geo->vbo) );
  if ( flags & STRUCTURED )
  {
    glGenBuffers( 1, &(geo->ebo) );
  }

  /* Initialize empty buffer */
  glBindVertexArray( geo->vao );
  glBindBuffer( GL_ARRAY_BUFFER, geo->vbo );

  unsigned long buf_size = 0;
  if ( flags & POSITION )  buf_size += 3 * sizeof(float); 
  if ( flags & NORMAL )    buf_size += 3 * sizeof(float);
  if ( flags & TANGENT )   buf_size += 3 * sizeof(float);
  if ( flags & TEX_COORD ) buf_size += 2 * sizeof(float);
  if ( flags & COLOR_A )   buf_size += 4 * sizeof(unsigned char);
  if ( flags & COLOR_B )   buf_size += 4 * sizeof(unsigned char);
  if ( flags & COLOR_C )   buf_size += 4 * sizeof(unsigned char);
  if ( flags & COLOR_D )   buf_size += 4 * sizeof(unsigned char);
  buf_size *= host_data->n_vertices;
  
  glBufferData( GL_ARRAY_BUFFER, buf_size, NULL, GL_STATIC_DRAW);

  geo->flags       = flags;
  geo->n_indices   = host_data->n_vertices;
  geo->n_elements  = host_data->n_elements;
  geo->buffer_size = buf_size;

  if ( !mshgfx_geometry_update( geo, host_data, flags ) )
  {
    return 0;
  }

  if ( flags & STRUCTURED )
  {  
    glBindVertexArray( geo->vao );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, geo->ebo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, 
                  geo->n_elements * sizeof( unsigned int ), 
                  &(host_data->indices[0]), 
                  GL_STATIC_DRAW ); 
  }

  glBindVertexArray(0);
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ); 
  return 1;
}

int 
mshgfx_geometry_free( mshgfx_geometry_t * geo )
{
  glDeleteBuffers(1, &(geo->vbo) );
  if ( geo->flags & STRUCTURED )
  {
    glDeleteBuffers(1, &(geo->ebo) );
  }
  glDeleteVertexArrays( 1, &(geo->vao) );
  geo->vao = -1;
  geo->vbo = -1;
  geo->ebo = -1;
  geo->n_indices  = -1;
  geo->n_elements = -1;
  return 1;
}

void 
mshgfx_geometry_draw( mshgfx_geometry_t * geo, 
                  GLenum draw_mode ) 
{
  msh_geometry_properties_flags flags = geo->flags;

  glBindVertexArray( geo->vao );
  
  if ( flags & STRUCTURED )
  {
    glDrawElements( draw_mode, geo->n_elements, GL_UNSIGNED_INT, 0 );
  }
  else
  {
    glDrawArrays( draw_mode, 0, geo->n_indices );
  }

  glBindVertexArray( 0 );
}

/*
 * =============================================================================
 *       TEXTURES IMPLEMENTATION
 * =============================================================================
 */

/* TODO (maciej): Format defining macro */

void 
mshgfx_texture_update_u8( mshgfx_texture_t *tex, const unsigned char *data )
{
  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );

  GLint internal_format;
  GLenum format = 0;
  switch( tex->n_comp )
  {
    case 1:
      internal_format = GL_R8;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG;
      break;
    case 3:
      internal_format = GL_RGB;
      format = GL_RGB;
      break;
    case 4:
      internal_format = GL_RGBA;
      format = GL_RGBA;
      break;
    default:
      internal_format = GL_RGB;
      format = GL_RGB;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, 
                tex->width, tex->height, 0, format, GL_UNSIGNED_BYTE, data );
  glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
mshgfx_texture_update_u16( mshgfx_texture_t *tex, const unsigned short *data )
{
  assert( tex->n_comp > 0 && tex->n_comp <= 1 );

  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );

  GLint internal_format;
  GLenum format;
  switch( tex->n_comp )
  {
    case 1:
      internal_format = GL_R16;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG16;
      format = GL_RG;
      break;
    case 3:
      internal_format = GL_RGB16;
      format = GL_RGB;
      break;
    case 4:
      internal_format = GL_RGBA16;
      format = GL_RGBA;
      break;
    default:
      internal_format = GL_RGB;
      format = GL_RGB;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, 
                tex->width, tex->height, 0, format, GL_UNSIGNED_SHORT, data );
  glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
mshgfx_texture_update_u32( mshgfx_texture_t *tex, const unsigned int *data )
{
  assert( tex->n_comp > 0 && tex->n_comp <= 1 );

  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );

  GLint internal_format;
  GLenum format;
  switch( tex->n_comp )
  {
    case 1:
      internal_format = GL_R32UI;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG32UI;
      format = GL_RG;
      break;
    case 3:
      internal_format = GL_RGB32UI;
      format = GL_RGB;
      break;
    case 4:
      internal_format = GL_RGBA32UI;
      format = GL_RGBA;
      break;
    default:
      internal_format = GL_RGB;
      format = GL_RGB;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, 
                tex->width, tex->height, 0, format, GL_UNSIGNED_INT, data );
  glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
mshgfx_texture_update_r32( mshgfx_texture_t *tex, const float *data )
{
  assert( tex->n_comp > 0 && tex->n_comp <= 1 );

  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );

  GLint internal_format;
  GLenum format;
  switch( tex->n_comp )
  {
    case 1:
      internal_format = GL_R32F;
      format = GL_RED;
      break;
    case 2:
      internal_format = GL_RG32F;
      format = GL_RG;
      break;
    case 3:
      internal_format = GL_RGB32F;
      format = GL_RGB;
      break;
    case 4:
      internal_format = GL_RGBA32F;
      format = GL_RGBA;
      break;
    default:
      internal_format = GL_RGB;
      format = GL_RGB;
  }

  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, 
                tex->width, tex->height, 0, format, GL_FLOAT, data );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


static inline void 
mshgfx__texture_init_type( mshgfx_texture_t *tex, const int type, 
                           const int w, const int h, const int n_comp,
                           const unsigned int unit )
{
  tex->width  = w;
  tex->height = h;
  tex->n_comp = n_comp; 
  tex->type   = type;
  tex->unit   = unit;
  glGenTextures( 1, &tex->id );
  
  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );
  /* TODO(maciej): Move the texture parameters out */
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void 
mshgfx_texture_init_u8( mshgfx_texture_t *tex,
                        const unsigned char *data, 
                        const int w, 
                        const int h, 
                        const int n_comp,  
                        const unsigned int unit )
{
  mshgfx__texture_init_type( tex, GL_UNSIGNED_BYTE, w, h, n_comp, unit );
  mshgfx_texture_update_u8( tex, data );

  glBindTexture( GL_TEXTURE_2D, 0 );
}


void 
mshgfx_texture_init_u16( mshgfx_texture_t *tex,
                         const unsigned short *data, 
                         const int w, 
                         const int h, 
                         const int n_comp,  
                         const unsigned int unit )
{
  mshgfx__texture_init_type( tex, GL_UNSIGNED_SHORT, w, h, n_comp, unit );
  mshgfx_texture_update_u16( tex, data );

  glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
mshgfx_texture_init_u32( mshgfx_texture_t *tex,
                         const unsigned int *data, 
                         const int w, 
                         const int h, 
                         const int n_comp,  
                         const unsigned int unit )
{
  mshgfx__texture_init_type( tex, GL_UNSIGNED_INT, w, h, n_comp, unit );
  mshgfx_texture_update_u32( tex, data );

  glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
mshgfx_texture_init_r32( mshgfx_texture_t *tex,
                         const float *data, 
                         const int w, 
                         const int h, 
                         const int n_comp,  
                         const unsigned int unit )
{
  mshgfx__texture_init_type( tex, GL_FLOAT, w, h, n_comp, unit );
  mshgfx_texture_update_r32( tex, data );

  glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
mshgfx_texture_use( const mshgfx_texture_t *tex)
{
    glActiveTexture( GL_TEXTURE0 + tex->unit );
    glBindTexture( GL_TEXTURE_2D, tex->id );
}

void 
mshgfx_texture_free( mshgfx_texture_t *tex )
{
    glDeleteTextures( 1, &tex->id );
    tex->id = 0;
}


#endif /* MSH_GFX_IMPLEMENTATION */
