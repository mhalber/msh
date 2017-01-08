/* 
 * TODO: Modify this to use more modern glProgramPipeline.
 * TODO: ifndef this?
 * TODO: Move text_file_read to a dedication file loading location
 * TODO: Framebuffer/Renderbuffer stuff
 * TODO: Add integer typedefs?
 * TODO: Need to experiment with glMapBufferRange.
 * TODO: Custom asserts?
 * TODO: Check basics error function, maybe add one here?
 */


#ifndef MSH_OGL_H
#define MSH_OGL_H

/*
 * =============================================================================
 *       INCLUDES
 * =============================================================================
 */

#include <stdlib.h>
#include <assert.h>

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#endif

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"


#ifndef MSH_VEC_MATH
typedef union vec2
{
  struct { float x; float y; };
  struct { float r; float g; };
  struct { float p; float q; };
  float data[2];
} msh_vec2_t;
#endif

/*
 * =============================================================================
 *       HELPERS
 * =============================================================================
 */

int
text_file_read ( const char * filename, char ** file_contents )
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

/*
 * =============================================================================
 *       WINDOWING/CONTEXT
 * =============================================================================
 */

/* 
  NOTE: Currently windowing is a thin wrapper over GLFW. There is some 
  functionality added for multiple windows
*/

typedef GLFWwindow msh_window_t ;

msh_window_t * msh_window_create( const char * title, 
                                  const int pos_x, const int pos_y, 
                                  const int res_x, const int res_y );
int msh_window_destroy( msh_window_t * window );

int  msh_window_display( msh_window_t * window, 
                         int (*display_function)(void) );

int  msh_window_is_any_open( msh_window_t ** windows, const int n_windows );

void msh_window_poll_events(void);

void msh_window_terminate(void);

/*
 * =============================================================================
 *       SHADERS
 * =============================================================================
 */

#define SHADER_HEAD "#version 330 core\n"

enum msh_shader_type
{
  VERTEX_SHADER,
  FRAGMENT_SHADER,
  GEOMETRY_SHADER,
  TESS_CONTROL_SHADER,
  TESS_EVALUATION_SHADER
};

typedef struct msh_shader
{
  GLuint id;
  enum msh_shader_type type;
  GLint compiled;
} msh_shader_t;

typedef struct msh_shader_prog
{
  GLuint id;
  GLint linked;
} msh_shader_prog_t;

int msh_shader_prog_create_from_source_vf( msh_shader_prog_t *p,
                                           const char *vs_source,
                                           const char *fs_source );

int msh_shader_prog_create_from_source_vgf( msh_shader_prog_t *p,
                                            const char *vs_source,
                                            const char *gs_source,
                                            const char *fs_source );

int msh_shader_prog_create_from_files_vf( msh_shader_prog_t *p,
                                          const char * vs_filename,
                                          const char * fs_filename );

int msh_shader_prog_create_from_files_vgf( msh_shader_prog_t *p,
                                           const char * vs_filename,
                                           const char * gs_filename,
                                           const char * fs_filename );

int msh_shader_compile( msh_shader_t *s,
                        const char * source );

int msh_shader_prog_link_vf( msh_shader_prog_t *p,
                             const msh_shader_t *vs, 
                             const msh_shader_t *fs );

int msh_shader_prog_link_vgf( msh_shader_prog_t *p,
                              const msh_shader_t *vs, 
                              const msh_shader_t *gs, 
                              const msh_shader_t *fs );

int msh_shader_prog_use( const msh_shader_prog_t *p );


void msh_shader_prog_set_uniform_1f( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x );
void msh_shader_prog_set_uniform_1i( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x );
void msh_shader_prog_set_uniform_1u( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x );

void msh_shader_prog_set_uniform_2f( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y );
void msh_shader_prog_set_uniform_2i( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y );
void msh_shader_prog_set_uniform_2u( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x, unsigned int y );

void msh_shader_prog_set_uniform_3f( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y, float z );
void msh_shader_prog_set_uniform_3i( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y, int z );
void msh_shader_prog_set_uniform_3u( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x, unsigned int y, 
                                     unsigned int z );

void msh_shader_prog_set_uniform_4f( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y, float z, float w );
void msh_shader_prog_set_uniform_4i( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y, int z, int w );
void msh_shader_prog_set_uniform_4u( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned int x, unsigned int y, 
                                     unsigned int z, unsigned int w );

void msh_shader_prog_set_uniform_fv( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     const float *val, 
                                     const unsigned int count );
void msh_shader_prog_set_uniform_iv( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     const int *val, 
                                     const unsigned int count );
void msh_shader_prog_set_uniform_uv( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     const unsigned int *val, 
                                     const unsigned int count );

/*
#ifdef MSH_VEC_MATH
void set_uniform ( const msh_shader_prog_t *p, 
                   const char *attrib_name, const vec2f &v );
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const vec2f *v, 
                    const unsigned int count = 1 );
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const vec3f &v );
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const vec3f *v, 
                    const unsigned int count = 1 );
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const vec4f &v);
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const vec4f *v, 
                    const unsigned int count = 1 );

void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const mat3f &m );
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const mat3f *m,
                    const unsigned int count = 1 );
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const mat4f &m );
void set_uniform ( const msh_shader_prog_t *p, 
                    const char *attrib_name, const mat4f *m,
                    const unsigned int count = 1 );
#endif
*/

/*
 * =============================================================================
 *       GPU GEOMETRY
 * =============================================================================
 */

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

typedef int msh_internal_data_type;
typedef int msh_geometry_properties_flags;

typedef struct msh_geometry_data
{
  float * positions;
  float * normals;
  float * tangents;
  float * texcoords;
  unsigned char  * colors_a;
  unsigned char  * colors_b;
  unsigned char  * colors_c;
  unsigned char  * colors_d;
  unsigned int * indices;
  int n_vertices;
  int n_elements;
} msh_geometry_data_t;

typedef struct msh_gpu_geometry
{
  unsigned int vao;
  unsigned int vbo;
  unsigned int ebo;
  int n_indices;
  int n_elements;
  int buffer_size;
  msh_geometry_properties_flags flags;
} msh_gpu_geometry_t;

int msh_gpu_geo_update( const msh_geometry_data_t * host_data, 
                        const msh_gpu_geometry_t * geo, 
                        const int flags );

int msh_gpu_geo_init( const msh_geometry_data_t * host_data,   
                      msh_gpu_geometry_t * geo, 
                      const int flags );

int msh_gpu_geo_free( msh_gpu_geometry_t * geo );

void msh_gpu_geo_draw( msh_gpu_geometry_t * geo, GLenum draw_mode );

/*
 * =============================================================================
 *       TEXTURES
 * =============================================================================
 */

typedef struct msh_gpu_texture
{
  unsigned int id;
  int width;
  int height;
  int n_comp;
  GLenum type;
  unsigned int unit;
} msh_gpu_texture_t;

void msh_gpu_tex_init_u8( const unsigned char * data, 
                          const int w, 
                          const int h, 
                          const int n_comp, 
                          msh_gpu_texture_t * tex, 
                          const unsigned int unit );

void msh_gpu_tex_init_u16( const unsigned short * data, 
                           const int w, 
                           const int h, 
                           const int n_comp, 
                           msh_gpu_texture_t * tex, 
                           const unsigned int unit );

void msh_gpu_tex_update_u8( const unsigned char * data, 
                            msh_gpu_texture_t * tex );

void msh_gpu_tex_update_u16( const unsigned short * data, 
                             msh_gpu_texture_t * tex );

void msh_gpu_tex_use( const msh_gpu_texture_t * tex );

void msh_gpu_tex_free( msh_gpu_texture_t * tex );

/*
 * =============================================================================
 *       FRAMEBUFFER 
 * =============================================================================
 */

enum msh_framebuffer_type_
{
  DEFAULT,
  RGB,
  RGBA
};

typedef int msh_framebuffer_type;

#endif /* MSH_OGL_H */


/*
================================================================================
================================================================================
================================================================================
================================================================================
*/

#ifdef MSH_OGL_IMPLEMENTATION

/*
 * =============================================================================
 *       WINDOWING/CONTEXT IMPLEMENTATION
 * =============================================================================
 */

msh_window_t * 
msh_window_create( const char * title,
                   const int pos_x, const int pos_y,
                   const int res_x, const int res_y )
{
  msh_window_t * window = NULL;

  /* attempt to initialize glfw library */
  if (!glfwInit())
  {
    return 0;
  }

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

  if (!window)
  {
    return NULL;
  }

  /* change position */
  if ( pos_x >= 0 && pos_y >= 0 )
  {
    glfwSetWindowPos( window, pos_x, pos_y );
  }
  
  return window;
}

int 
msh_window_destroy( msh_window_t * window )
{
  if ( window )
  {
    glfwDestroyWindow( window );
    return 1;
  }
  return 0;
}

int 
msh_window_display( msh_window_t * window,
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
msh_window_is_any_open( msh_window_t ** windows, const int n_windows )
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
         msh_window_destroy( windows[i] );
         windows[i] = NULL;
      }
    }
    is_any_open |= is_valid;
   }
  return is_any_open;
 }
 
void 
msh_window_poll_events( void )
{
  glfwPollEvents();
}

void 
msh_window_terminate( void )
{
  glfwTerminate();
}



/*
 * =============================================================================
 *       SHADERS IMPLEMENTATION
 * =============================================================================
 */

int
msh_shader_compile( msh_shader_t *s, const char * source )
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
      char * msh_shader_type_str = NULL;
      switch(s->type)
      {
        case VERTEX_SHADER:
          msh_shader_type_str = (char*)"Vertex Shader";
          break;
        case FRAGMENT_SHADER:
          msh_shader_type_str = (char*)"Fragment Shader";
          break;
        case GEOMETRY_SHADER:
          msh_shader_type_str = (char*)"Geometry Shader";
          break;
        case TESS_CONTROL_SHADER:
          msh_shader_type_str = (char*)"Tess Control Shader";
          break;
        case TESS_EVALUATION_SHADER:
          msh_shader_type_str = (char*)"Tess Evaluation Shader";
          break;
      }
      printf( "%s Compilation Failure :\n%s\n", msh_shader_type_str, log_str );
      free( log_str );
    }
    return 0;
  }

  return s->compiled;
}

int 
msh_shader_prog_link_vf( msh_shader_prog_t *p,
                         const msh_shader_t *vs, 
                         const msh_shader_t *fs )
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
msh_shader_prog_link_vgf( msh_shader_prog_t *p,
                          const msh_shader_t *vs, 
                          const msh_shader_t *gs, 
                          const msh_shader_t *fs )
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
msh_shader_prog_use( const msh_shader_prog_t *p )
{
  if ( p->linked ) {
    glUseProgram( p->id );
    return 1;
  }
  return 0;
}

int
msh_shader_prog_create_from_source_vf( msh_shader_prog_t *p,
                                       const char *vs_src,
                                       const char *fs_src )
{

  msh_shader_t vs, fs;
  vs.type = VERTEX_SHADER;
  fs.type = FRAGMENT_SHADER;

  if ( !msh_shader_compile( &vs, vs_src ) )        return 0;
  if ( !msh_shader_compile( &fs, fs_src ) )        return 0;
  if ( !msh_shader_prog_link_vf( p, &vs, &fs ) )   return 0;

  return 1;
}

int
msh_shader_prog_create_from_source_vgf( msh_shader_prog_t *p,
                                        const char * vs_src,
                                        const char * gs_src,
                                        const char * fs_src )
{
  msh_shader_t vs, fs, gs;
  vs.type = VERTEX_SHADER;
  fs.type = FRAGMENT_SHADER;
  gs.type = GEOMETRY_SHADER;

  if ( !msh_shader_compile( &vs, vs_src ) )            return 0;
  if ( !msh_shader_compile( &gs, gs_src ) )            return 0;
  if ( !msh_shader_compile( &fs, fs_src ) )            return 0;
  if ( !msh_shader_prog_link_vgf( p, &vs, &gs, &fs ) ) return 0;

  return 1;
}

int
msh_shader_prog_create_from_files_vf( msh_shader_prog_t *p,
                                      const char * vs_filename,
                                      const char * fs_filename )
{
  char *vs_source, *fs_source;
  if( !text_file_read( vs_filename, &vs_source ) ||
      !text_file_read( fs_filename, &fs_source ) )
  {
    return 0;
  }

  if( !msh_shader_prog_create_from_source_vf( p, vs_source, fs_source ))
  {
    return 0;
  }

  free( vs_source );
  free( fs_source );

  return 1;
}

int
msh_shader_prog_create_from_files_vgf( msh_shader_prog_t *p,
                                       const char * vs_filename,
                                       const char * gs_filename,
                                       const char * fs_filename )
{
  char *vs_source, *fs_source, *gs_source;
  if( !text_file_read( vs_filename, &vs_source ) ||
      !text_file_read( gs_filename, &gs_source ) ||
      !text_file_read( fs_filename, &fs_source ) )
  {
    return 0;
  }

  if( !msh_shader_prog_create_from_source_vgf( p, 
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
msh_shader_prog_set_uniform_1f( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                float x )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1f( location, (GLfloat)x );
}

void
msh_shader_prog_set_uniform_1i( const msh_shader_prog_t *p, 
                               const char *attrib_name, 
                               int x )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1i( location, (GLint)x );
}

void
msh_shader_prog_set_uniform_1u( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                unsigned int x )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1ui( location, (GLuint)x );
}

void
msh_shader_prog_set_uniform_2f( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                float x, float y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2f( location, x, y );
}

void
msh_shader_prog_set_uniform_2i( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                int x, int y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2i( location, x, y );
}

void
msh_shader_prog_set_uniform_2u( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                unsigned int x, unsigned int y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2ui( location, x, y );
}


void
msh_shader_prog_set_uniform_3f( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                float x, float y, float z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3f( location, x, y, z );
}

void
msh_shader_prog_set_uniform_3i( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                int x, int y, int z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3i( location, x, y, z );
}

void
msh_shader_prog_set_uniform_3u( const msh_shader_prog_t *p, 
                                const char *attrib_name, 
                                unsigned int x, unsigned int y, unsigned int z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3ui( location, x, y, z );
}



/*

NOTE: These should only be active if specific header is already included
void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const vec2f & v)
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2fv( location, 1, &(v.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const vec2f * v, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2fv( location, count, &( v->data[0] ) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const vec3f & v)
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3fv( location, 1, &(v.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const vec3f * v, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3fv( location, count, &( v->data[0] ) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const vec4f &v )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4fv( location, 1, &(v.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const vec4f *v, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4fv( location, count, &(v->data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const mat3f &m )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix3fv( location, 1, GL_FALSE, &(m.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const mat3f *m, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix3fv( location, count, GL_FALSE, &(m->data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const mat4f &m )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix4fv( location, 1, GL_FALSE, &(m.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_prog_t *p, 
              const char *attrib_name, const mat4f *m, const unsigned int count  )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix4fv( location, 1, GL_FALSE, &(m->data[0]) );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_prog_t *p, 
               const char *attrib_name, bool val )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1i( location, (GLint)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_prog_t *p, 
               const char *attrib_name, const bool * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1iv( location, count, (GLint*)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_prog_t *p, 
               const char *attrib_name, const float * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1fv( location, count, (GLfloat*)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_prog_t *p, 
               const char *attrib_name, const int * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1iv( location, count, (GLint*)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_prog_t *p, 
               const char *attrib_name, const unsigned int * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1uiv( location, count, (GLuint*)val );
}
*/



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
msh__gpu_geo_get_offset( const msh_gpu_geometry_t * geo, 
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
msh_gpu_geo_update( const msh_geometry_data_t * host_data, 
                    const msh_gpu_geometry_t * geo,
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
msh_gpu_geo_init( const msh_geometry_data_t * host_data, 
                  msh_gpu_geometry_t * geo, 
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

  if ( !msh_gpu_geo_update( host_data, geo, flags ) )
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
msh_gpu_geo_free( msh_gpu_geometry_t * geo )
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
msh_gpu_geo_draw( msh_gpu_geometry_t * geo, 
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
 *       TEXTURES
 * =============================================================================
 */

void 
msh_gpu_tex_update_u8( const unsigned char * data, 
                       msh_gpu_texture_t * tex )
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
msh_gpu_tex_update_u16( const unsigned short * data, 
                        msh_gpu_texture_t * tex )
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
msh_gpu_tex_init_u8( const unsigned char * data, 
                     const int w, 
                     const int h, 
                     const int n_comp, 
                     msh_gpu_texture_t * tex, 
                     const unsigned int unit )
{
  tex->width  = w;
  tex->height = h;
  tex->n_comp = n_comp; 
  tex->type   = GL_UNSIGNED_BYTE;
  tex->unit   = unit;
  glGenTextures( 1, &tex->id );
  
  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  msh_gpu_tex_update_u8( data, tex );

  glBindTexture( GL_TEXTURE_2D, 0 );
}


void 
init_gpu_tex( const unsigned short * data, 
              const int w, 
              const int h, 
              const int n_comp, 
              msh_gpu_texture_t * tex, 
              const unsigned int unit )
{
  tex->width  = w;
  tex->height = h;
  tex->n_comp = n_comp; 
  tex->type   = GL_UNSIGNED_SHORT;
  tex->unit   = unit;
  glGenTextures( 1, &tex->id );
  
  glActiveTexture ( GL_TEXTURE0 + tex->unit );
  glBindTexture( GL_TEXTURE_2D, tex->id );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  msh_gpu_tex_update_u16( data, tex );

  glBindTexture( GL_TEXTURE_2D, 0 );
}

void 
msh_gpu_tex_use( const msh_gpu_texture_t * tex)
{
    glActiveTexture( GL_TEXTURE0 + tex->unit );
    glBindTexture( GL_TEXTURE_2D, tex->id );
}

void 
msh_gpu_tex_free( msh_gpu_texture_t * tex )
{
    glDeleteTextures( 1, &tex->id );
    tex->id = 0;
}



/*
 * =============================================================================
 *       FRAMEBUFFERS
 * =============================================================================
 */



#endif /* MSH_OGL_IMPLEMENTATION */
