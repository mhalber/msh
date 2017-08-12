/*
  ==============================================================================
  
  MSH_DRAW.H - RESEARCH DRAWING API!
  
  Intent of this header file is to learn about drawing api's, how they should be
  designed etc.
  Purpose is building a plot app in the future.

  ==============================================================================
  DEPENDENCIES

  This library depends on following standard headers:
  
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

  ==============================================================================
  TODO
  [ ] API Design
     [ ] Have a set of commands that you append to
  [ ] OpenGL backend
  [ ] Software Rasterizer backend
  ==============================================================================
  REFERENCES:

 */

#ifndef MSH_DRAW_H
#define MSH_DRAW_H

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#else
  #include "glad/glad.h"
#endif

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

typedef struct msh_draw_ctx
{
  GLuint vert_shdr_id, frag_shdr_id, prog_id;
  GLuint vao, vbo;
} msh_draw_ctx_t;

int msh_init_ctx( msh_draw_ctx_t* ctx );
void msh_draw_circle( msh_draw_ctx_t* ctx, float x, float y, float radius );
void msh_draw_line( msh_draw_ctx_t* ctx, float x1, float y1, float x2, float y2 );
void msh_draw_triangle( msh_draw_ctx_t* ctx, float x, float y, float s );

#endif /*MSH_DRAW_H*/

#ifdef MSH_DRAW_IMPLEMENTATION


int 
msh__check_compilation_status( GLuint shader_id, GLuint shader_type )
{
  int compiled = 0;
  glGetShaderiv( shader_id, GL_COMPILE_STATUS, &compiled );

  if( !compiled )
  {
    GLint log_len;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_len);
    if ( log_len > 0 )
    {
      char* log_str = (char*)malloc(log_len);
      GLsizei written;
      glGetShaderInfoLog(shader_id, log_len, &written, log_str);
      char * mshgfx_shader_type_str = NULL;
      switch( shader_type )
      {
        case GL_VERTEX_SHADER:
          mshgfx_shader_type_str = (char*)"Vertex Shader";
          break;
        case GL_FRAGMENT_SHADER:
          mshgfx_shader_type_str = (char*)"Fragment Shader";
          break;
      }
      printf( "%s Compilation Failure :\n%s\n", mshgfx_shader_type_str, log_str );
      free( log_str );
    }
  }
  return compiled;
}

int
msh__check_linking_status( GLuint prog_id )
{
  int linked = 0;
  glGetProgramiv( prog_id, GL_LINK_STATUS, &linked );

  if ( !linked )
  {
    GLint log_len;
    glGetProgramiv( prog_id, GL_INFO_LOG_LENGTH, &log_len );
    if( log_len > 0 )
    {
      char* log_str = (char*) malloc( log_len );
      GLsizei written;
      glGetProgramInfoLog( prog_id, log_len, &written, log_str );
      printf( "Program Linking Failure : %s\n", log_str );
      free( log_str );
    }
  }
  return linked;
}

#define MSHDRAW_STRINGIFY(x) #x

int 
msh_draw_init_ctx( msh_draw_ctx_t* ctx )
{
  // create shader
  const char* vert_shdr_src = (const char*) "#version 330 core\n" MSHDRAW_STRINGIFY
  (
    in vec3 position;
    void main()
    {
      gl_Position = vec4( position, 1 );
    }
  );
  const char* frag_shdr_src = (const char*) "#version 330 core\n" MSHDRAW_STRINGIFY
  (
    out vec4 frag_color;
    // uniform vec4 input_color;
    void main()
    {
      frag_color = vec4(1.0, 0.0, 1.0, 1.0); 
    }
  ); 
  ctx->vert_shdr_id = 0;
  ctx->frag_shdr_id = 0;
  ctx->prog_id = 0;
  ctx->vao = 0;
  ctx->vbo = 0;
  
  ctx->vert_shdr_id = glCreateShader( GL_VERTEX_SHADER );
  ctx->frag_shdr_id = glCreateShader( GL_FRAGMENT_SHADER );

  // compile
  glShaderSource( ctx->vert_shdr_id, 1, &vert_shdr_src, NULL );
  glShaderSource( ctx->frag_shdr_id, 1, &frag_shdr_src, NULL );
  glCompileShader( ctx->vert_shdr_id );
  glCompileShader( ctx->frag_shdr_id );
  int vert_compiled = msh__check_compilation_status( ctx->vert_shdr_id, GL_VERTEX_SHADER );
  int frag_compiled = msh__check_compilation_status( ctx->frag_shdr_id, GL_FRAGMENT_SHADER);
  if( !vert_compiled || !frag_compiled ) return 0;
  
  // create and link program
  ctx->prog_id = glCreateProgram();
  if( !ctx->prog_id ) 
  {
    printf("Failed to create drawing context program!\n");
    return 0;
  }
  glAttachShader( ctx->prog_id, ctx->vert_shdr_id );
  glAttachShader( ctx->prog_id, ctx->frag_shdr_id );
  glLinkProgram( ctx->prog_id );
  int linked = msh__check_linking_status( ctx->prog_id );

  if( !linked ) 
  {
    printf("Failed to create drawing context program!\n");
    return 0;
  }

  // free shaders
  glDetachShader( ctx->prog_id, ctx->vert_shdr_id );
  glDetachShader( ctx->prog_id, ctx->frag_shdr_id );

  glDeleteShader( ctx->vert_shdr_id );
  glDeleteShader( ctx->frag_shdr_id );

  // create vao and vbo
  glGenVertexArrays( 1, &(ctx->vao) );
  glGenBuffers(1, &ctx->vbo);

  glBindVertexArray( ctx->vao );
  glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
  glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), NULL, GL_STREAM_DRAW);

  GLuint pos_attrib = glGetAttribLocation( ctx->prog_id, "position" );

  glEnableVertexAttribArray( pos_attrib );
  glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glBindVertexArray( 0 );
  glDisableVertexAttribArray( pos_attrib );
  return 1;
}


void
msh_draw_triangle( msh_draw_ctx_t* ctx, float x, float y, float s )
{
  glUseProgram(ctx->prog_id);

  glBindVertexArray(ctx->vao);

  GLfloat vbo_data[] = {
   x-s, y-s, 0.0f,
   x+s, y-s, 0.0f,
   x, y+s, 0.0f,
  };

  glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vbo_data), vbo_data, GL_STREAM_DRAW);

  glDrawArrays(GL_TRIANGLES, 0, 3);
  glBindVertexArray(0);

}
#endif /*MSH_DRAW_IMPLEMENTATION */
