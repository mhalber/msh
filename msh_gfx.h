// NOTE: Need to figure out if this even makes sense, or should i simply modify basics...
// Things like the lack of method overriding are annoying in this case...
// NOTE: Also->I am a bit annoyed with the initializer lists, I will need to see how the structs are being copied / passed.

#ifndef MSH_GFX_H // NOTE: Should we start these with '_' - it is said that they should not start like this
#define MSH_GFX_H

#define SHADER_HEAD "#version 330 core\n"

// TODO: Modify this to use more modern glProgramPipeline.
// TODO: Move text_file_read to a dedication file loading location\


#include <stdlib.h>
inline int
text_file_read ( const char * filename, char ** file_contents )
{
  FILE *fp;
  long l_size;

  fp = fopen ( filename , "r" );
  if( !fp ) perror(filename),exit(1);

  fseek( fp , 0L , SEEK_END);
  l_size = ftell( fp );
  rewind( fp );

  // allocate memory for entire content 
  (*file_contents) = (char*)calloc( 1, l_size + 1 );
  if ( !(*file_contents) )
  { 
    fclose( fp ); 
    fputs( "memory alloc fails", stderr ); 
    exit( 1 );
  }

  // copy the file into the buffer 
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



////////////////////////////////////////////////////////////////////////////////
// SHADERS
////////////////////////////////////////////////////////////////////////////////
enum shader_type
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
  enum shader_type type;
  GLint compiled;
} msh_shader_t;

typedef struct msh_shader_prog
{
  GLuint id;
  GLint linked;
} msh_shader_prog_t;

int msh_shader_prog_create_from_source_vf( const char *vs_source,
                                           const char *fs_source,
                                           msh_shader_prog_t *p);

int msh_shader_prog_create_from_source_vgf( const char *vs_source,
                                            const char *gs_source,
                                            const char *fs_source,
                                            msh_shader_prog_t *p);

int msh_shader_prog_create_from_files_vf( const char * vs_filename,
                                          const char * fs_filename,
                                          msh_shader_prog_t *p );

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
                                     unsigned x, unsigned y );

void msh_shader_prog_set_uniform_3f( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y, float z );
void msh_shader_prog_set_uniform_3i( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y, int z );
void msh_shader_prog_set_uniform_3u( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned x, unsigned y, unsigned z );

void msh_shader_prog_set_uniform_4f( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     float x, float y, float z, float w );
void msh_shader_prog_set_uniform_4i( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     int x, int y, int z, int w );
void msh_shader_prog_set_uniform_4u( const msh_shader_prog_t *p, 
                                     const char *attrib_name, 
                                     unsigned x, unsigned y, unsigned z, unsigned w );

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

// void set_uniform ( const msh_shader_prog_t *p, 
//                    const char *attrib_name, const vec2f &v );
// void set_uniform ( const msh_shader_prog_t *p, 
//                     const char *attrib_name, const vec2f *v, 
//                     const unsigned int count = 1 );
// void set_uniform ( const msh_shader_prog_t *p, 
//                     const char *attrib_name, const vec3f &v );
// void set_uniform ( const msh_shader_prog_t *p, 
//                     const char *attrib_name, const vec3f *v, 
//                     const unsigned int count = 1 );
// void set_uniform ( const msh_shader_prog_t *p, 
//                     const char *attrib_name, const vec4f &v);
// void set_uniform ( const msh_shader_prog_t *p, 
//                     const char *attrib_name, const vec4f *v, 
//                     const unsigned int count = 1 );

// void set_uniform ( const msh_shader_prog_t *p, 
//                     const char *attrib_name, const mat3f &m );
// void set_uniform ( const msh_shader_t *p, 
//                     const char *attrib_name, const mat3f *m,
//                     const unsigned int count = 1 );
// void set_uniform ( const msh_shader_t *p, 
//                     const char *attrib_name, const mat4f &m );
// void set_uniform ( const msh_shader_t *p, 
//                     const char *attrib_name, const mat4f *m,
//                     const unsigned int count = 1 );

////////////////////////////////////////////////////////////////////////////////
// ARRAY/ELEMENT BUFFERS
////////////////////////////////////////////////////////////////////////////////

enum geometry_properties_flags_
{
  POSITION  = 1 << 0, // layout( location = 0 )
  NORMAL    = 1 << 1, // layout( location = 1 )
  TANGENT   = 1 << 2, // layout( location = 2 )
  TEX_COORD = 1 << 3, // layout( location = 3 )
  COLOR_A   = 1 << 4, // layout( location = 4 )
  COLOR_B   = 1 << 5, // layout( location = 5 )
  COLOR_C   = 1 << 6, // layout( location = 6 )
  COLOR_D   = 1 << 7, // layout( location = 7 )
  STRUCTURED = 1 << 8,

  SIMPLE_MESH   = POSITION | NORMAL | STRUCTURED,
  POINTCLOUD    = POSITION,
};

typedef int internal_data_type
typedef int geometry_properties_flags
typedef int framebuffer_type

namespace bsc
{
// GEOMETRY
  struct geometry_data
  {
    r32 * positions = NULL;
    r32 * normals   = NULL;
    r32 * tangents  = NULL;
    r32 * texcoords = NULL;
    u8  * colors_a  = NULL;
    u8  * colors_b  = NULL;
    u8  * colors_c  = NULL;
    u8  * colors_d  = NULL;
    u32 * indices   = NULL;
    i32 n_vertices  = -1;
    i32 n_elements  = -1;
  };

  // TODO : Need to experiment with glMapBufferRange.
  struct gpu_geometry
  {
    u32 vao     = -1;
    u32 vbo     = -1;
    u32 ebo     = -1;
    i32 n_indices  = -1;
    i32 n_elements = -1;
    i32 buffer_size = -1;
    GeometryPropertiesFlags flags;
  };

  i32 update_gpu_geo( const geometry_data * host_data, 
                      const gpu_geometry * geo, 
                      const i32 flags = SIMPLE_MESH );
  i32 init_gpu_geo( const geometry_data * host_data,   
                    gpu_geometry * geo, 
                    const i32 flags = SIMPLE_MESH );
  i32 free_gpu_geo( gpu_geometry * geo );
  
  void draw( gpu_geometry * geo, 
             GLenum draw_mode = GL_TRIANGLES );

////////////////////////////////////////////////////////////////////////////////
// TEXTURES
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// FRAMEBUFFER 
////////////////////////////////////////////////////////////////////////////////


enum framebuffer_type_
{
  DEFAULT,
  RGB,
  RGBA,
};


#endif // MSH_GFX_H


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef MSH_GFX_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////
// SHADERS IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////

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
          return false;
  }

  glShaderSource(s->id, 1, &source, NULL);

  glCompileShader(s->id);
  glGetShaderiv(s->id, GL_COMPILE_STATUS, &s.compiled);

  if(!s.compiled)
  {
    GLint log_len;
    glGetShaderiv(s->id, GL_INFO_LOG_LENGTH, &log_len);
    if ( log_len > 0 )
    {
      char* log_str = (char*)malloc(log_len);
      GLsizei written;
      glGetShaderInfoLog(s->id, log_len, &written, log_str);
      char * shader_type_str;
      switch(s.type)
      {
        case VERTEX_SHADER:
          shader_type_str = (char*)"Vertex Shader";
          break;
        case FRAGMENT_SHADER:
          shader_type_str = (char*)"Fragment Shader";
          break;
        case GEOMETRY_SHADER:
          shader_type_str = (char*)"Geometry Shader";
          break;
        case TESS_CONTROL_SHADER:
          shader_type_str = (char*)"Tess Control Shader";
          break;
        case TESS_EVALUATION_SHADER:
          shader_type_str = (char*)"Tess Evaluation Shader";
          break;
      }
      printf( "%s Compilation Failure :\n%s\n", shader_type_str, log_str );
      free( log_str );
    }
    return 0;
  }

  return s.compiled;
}

int 
msh_shader_prog_link_vf( msh_shader_prog_t *p 
                         const msh_shader_t *vs, 
                         const msh_shader_t *fs )
{
  // change to asserts?
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

  return p.linked;
}


int
msh_shader_prog_link_vgf( msh_hader_prog_t *p
                          const shader *vs, 
                          const shader *gs, 
                          const shader *fs )
{
  // change to asserts?
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

  return p.linked;
}

int
msh_shader_prog_use( const shader_prog *p )
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

  if ( !msh_shader_compile( vs_src, vs ) )        return 0;
  if ( !msh_shader_compile( fs_src, fs ) )        return 0;
  if ( !msh_shader_prog_link_vf( vs, fs, p ) )    return 0;

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

  if ( !compile_shader( vs_src, vs ) )     return 0;
  if ( !compile_shader( gs_src, gs ) )     return 0;
  if ( !compile_shader( fs_src, fs ) )     return 0;
  if ( !link_program( vs, gs, fs, p ) )    return 0;

  return 1;
}

int
msh_shader_prog_create_from_files_vf( msh_shader_prog_t *p
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
msh_shader_prog_create_from_files_vgf( msh_shader_prog_t *p
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

  if( !msh_shader_prog_create_from_source_vf( p, 
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
msh_shader_prog_set_uniform_1f( const msh_shader_t *p, 
                               const char *attrib_name, 
                               float x )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1f( location, (GLfloat)x );
}

void
msh_shader_prog_set_uniform_1i( const msh_shader_t *p, 
                               const char *attrib_name, 
                               int x )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1i( location, (GLint)x );
}

void
msh_shader_prog_set_uniform_1u( const msh_shader_t *p, 
                               const char *attrib_name, 
                               unsigned int x )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1ui( location, (GLuint)x );
}

void
msh_shader_prog_set_uniform_2f( const msh_shader_t *p, 
                                const char *attrib_name, 
                                float x, float y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2f( location, x, y );
}

void
msh_shader_prog_set_uniform_2i( const msh_shader_t *p, 
                                const char *attrib_name, 
                                int x, int y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2i( location, x, y );
}

void
msh_shader_prog_set_uniform_2u( const msh_shader_t *p, 
                                const char *attrib_name, 
                                unsigned x, unsigned y )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2ui( location, x, y );
}


void
msh_shader_prog_set_uniform_3f( const msh_shader_t *p, 
                                const char *attrib_name, 
                                float x, float y, float z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3f( location, x, y, z );
}

void
msh_shader_prog_set_uniform_3i( const msh_shader_t *p, 
                                const char *attrib_name, 
                                int x, int y, int z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3i( location, x, y, z );
}

void
msh_shader_prog_set_uniform_3u( const msh_shader_t *p, 
                                const char *attrib_name, 
                                unsigned x, unsigned y, unsigned z )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3ui( location, x, y, z );
}



/*

NOTE: These should only be active if specific header is already included
void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const vec2f & v)
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2fv( location, 1, &(v.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const vec2f * v, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform2fv( location, count, &( v->data[0] ) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const vec3f & v)
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3fv( location, 1, &(v.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const vec3f * v, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform3fv( location, count, &( v->data[0] ) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const vec4f &v )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4fv( location, 1, &(v.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const vec4f *v, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform4fv( location, count, &(v->data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const mat3f &m )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix3fv( location, 1, GL_FALSE, &(m.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const mat3f *m, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix3fv( location, count, GL_FALSE, &(m->data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const mat4f &m )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix4fv( location, 1, GL_FALSE, &(m.data[0]) );
}

void
msh_shader_prog_set_uniform ( const msh_shader_t *p, 
              const char *attrib_name, const mat4f *m, const unsigned int count  )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniformMatrix4fv( location, 1, GL_FALSE, &(m->data[0]) );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_t *p, 
               const char *attrib_name, bool val )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1i( location, (GLint)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_t *p, 
               const char *attrib_name, const bool * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1iv( location, count, (GLint*)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_t *p, 
               const char *attrib_name, const r32 * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation( p->id, attrib_name );
  glUniform1fv( location, count, (GLfloat*)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_t *p, 
               const char *attrib_name, const int * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1iv( location, count, (GLint*)val );
}

void
msh_shader_prog_set_uniform  ( const msh_shader_t *p, 
               const char *attrib_name, const unsigned int * val, const unsigned int count )
{
  GLuint location = glGetUniformLocation ( p->id, attrib_name );
  glUniform1uiv( location, count, (GLuint*)val );
}
*/



////////////////////////////////////////////////////////////////////////////////
// ARRAY/ELEMENT BUFFERS
////////////////////////////////////////////////////////////////////////////////

//TODO

////////////////////////////////////////////////////////////////////////////////
// END ARRAY/ELEMENT BUFFERS
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// TEXTURES
////////////////////////////////////////////////////////////////////////////////

//TODO

////////////////////////////////////////////////////////////////////////////////
// END TEXTURES
////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////
// FRAMEBUFFERS
////////////////////////////////////////////////////////////////////////////////

//TODO

////////////////////////////////////////////////////////////////////////////////
// END FRAMEBUFFERS
////////////////////////////////////////////////////////////////////////////////

#endif //MSH_GFX_IMPLEMENTATION