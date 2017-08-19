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

    Nanovg seems to avoid calls by having the Fill operations
      ->Paint settings set the materials
      ->Shape settings append geometry
      ->Fill/Stroke command converts what's in current state into the glcalls that will be used when flushing

  ==============================================================================
  TODO
  [ ] API Design
     [ ] Have a set of commands that you append to
  [ ] OpenGL backend
  [ ] Software Rasterizer backend
  [ ] Rethink the name
  [ ] How to deal with materials
  [ ] Separate buffer for ogl? How to avoid that?
  ==============================================================================
  REFERENCES:
  [1] NanoGUI
 */

#ifndef MSH_DRAW_H
#define MSH_DRAW_H

// #ifdef __APPLE__
  // #include <OpenGL/gl3.h>
// #else
  #include "glad/glad.h"
// #endif

#define GLFW_INCLUDE_NONE
#include "GLFW/glfw3.h"

#define MSH_DRAW_INIT_CMD_BUF_SIZE 1024
#define MSH_DRAW_STRINGIFY(x) #x
#define MSH_DRAW_OPENGL

typedef enum
{
  MSHD_TRIANGLE,
  MSHD_CIRCLE,
  MSHD_RECTANGLE
} msh_cmd_type;

#ifdef MSH_DRAW_OPENGL
typedef struct msh_draw_ogl_backend
{
  GLuint vert_shdr_id;
  GLuint frag_shdr_id;
  GLuint prog_id;
  GLuint vao;
  GLuint vbo;
} msh_draw_opengl_backend_t;
#endif

typedef struct msh_draw_color
{
  float r;
  float g;
  float b;
  float a;
} msh_draw_color_t;

typedef struct msh_draw_paint
{
  msh_draw_color_t fill_color;
} msh_draw_paint_t;

typedef struct msh_draw_cmd
{
  msh_cmd_type type;
  int paint_idx;
  float geometry[10];
  float z_idx;
} msh_draw_cmd_t;

typedef struct msh_draw_ctx
{
  int cmd_buf_size;
  int cmd_buf_capacity;
  msh_draw_cmd_t* cmd_buf; //TODO(maciej): Switch to unsigned char
  int cmd_idx;

  // TODO(maciej): Switch to a hash-table?
  int paint_buf_size;
  int paint_buf_capacity;
  msh_draw_paint_t* paint_buf;
  int paint_idx;

  float z_idx;

#ifdef MSH_DRAW_OPENGL
  msh_draw_opengl_backend_t backend;
#endif
} msh_draw_ctx_t;


int msh_draw_init_ctx( msh_draw_ctx_t* ctx );
int msh_draw_new_frame( msh_draw_ctx_t* ctx );
int msh_draw_render( msh_draw_ctx_t* ctx );
void msh_draw_circle( msh_draw_ctx_t* ctx, float x, float y, float radius );
void msh_draw_line( msh_draw_ctx_t* ctx, float x1, float y1, float x2, float y2 );
void msh_draw_triangle( msh_draw_ctx_t* ctx, float x, float y, float s );

#endif /*MSH_DRAW_H*/

#ifdef MSH_DRAW_IMPLEMENTATION

#ifdef MSH_DRAW_OPENGL
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
#endif

int 
msh_draw_init_ctx( msh_draw_ctx_t* ctx )
{
  // Setup command buffer
  ctx->cmd_buf_capacity = MSH_DRAW_INIT_CMD_BUF_SIZE;
  ctx->cmd_buf = (msh_draw_cmd_t*)malloc( ctx->cmd_buf_capacity * sizeof(msh_draw_cmd_t) );

  // Setup paint buffer
  ctx->paint_buf_capacity = MSH_DRAW_INIT_CMD_BUF_SIZE;
  ctx->paint_buf = (msh_draw_paint_t*)malloc( ctx->paint_buf_capacity * sizeof(msh_draw_paint_t) );

  // Setup z indexing
  ctx->z_idx = 0.0f;

#ifdef MSH_DRAW_OPENGL
  // create shader
  const char* vert_shdr_src = (const char*) "#version 330 core\n" MSH_DRAW_STRINGIFY
  (
    layout(location = 0) in vec3 position;
    void main()
    {
      gl_Position = vec4( position, 1 );
    }
  );
  const char* frag_shdr_src = (const char*) "#version 330 core\n" MSH_DRAW_STRINGIFY
  (
    out vec4 frag_color;
    uniform vec3 input_color;
    void main()
    {
      frag_color = vec4(input_color, 1.0); 
    }
  ); 

  msh_draw_opengl_backend_t* ogl = &ctx->backend;
  ogl->vert_shdr_id = 0;
  ogl->frag_shdr_id = 0;
  ogl->prog_id = 0;
  ogl->vao = 0;
  ogl->vbo = 0;
  
  ogl->vert_shdr_id = glCreateShader( GL_VERTEX_SHADER );
  ogl->frag_shdr_id = glCreateShader( GL_FRAGMENT_SHADER );

  // compile
  glShaderSource( ogl->vert_shdr_id, 1, &vert_shdr_src, NULL );
  glShaderSource( ogl->frag_shdr_id, 1, &frag_shdr_src, NULL );
  glCompileShader( ogl->vert_shdr_id );
  glCompileShader( ogl->frag_shdr_id );
  int vert_compiled = msh__check_compilation_status( ogl->vert_shdr_id, GL_VERTEX_SHADER );
  int frag_compiled = msh__check_compilation_status( ogl->frag_shdr_id, GL_FRAGMENT_SHADER);
  if( !vert_compiled || !frag_compiled ) return 0;
  
  // create and link program
  ogl->prog_id = glCreateProgram();
  if( !ogl->prog_id ) 
  {
    printf("Failed to create drawing context program!\n");
    return 0;
  }
  glAttachShader( ogl->prog_id, ogl->vert_shdr_id );
  glAttachShader( ogl->prog_id, ogl->frag_shdr_id );
  glLinkProgram( ogl->prog_id );
  int linked = msh__check_linking_status( ogl->prog_id );

  if( !linked ) 
  {
    printf("Failed to create drawing context program!\n");
    return 0;
  }

  // free shaders
  glDetachShader( ogl->prog_id, ogl->vert_shdr_id );
  glDetachShader( ogl->prog_id, ogl->frag_shdr_id );

  glDeleteShader( ogl->vert_shdr_id );
  glDeleteShader( ogl->frag_shdr_id );

  // create vao and vbo
  glGenVertexArrays( 1, &(ogl->vao) );
  glGenBuffers(1, &ogl->vbo);

  glBindVertexArray( ogl->vao );
  glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
  glBufferData(GL_ARRAY_BUFFER, 1024 * sizeof(float), NULL, GL_STREAM_DRAW);

  GLuint pos_attrib = glGetAttribLocation( ogl->prog_id, "position" );

  glEnableVertexAttribArray( pos_attrib );
  glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
  glBindVertexArray( 0 );
  glDisableVertexAttribArray( pos_attrib );
#endif
  return 1;
}

int msh_draw_new_frame( msh_draw_ctx_t* ctx )
{
  // Reset indices
  ctx->cmd_idx = 0;
  ctx->cmd_buf_size = 0;
  ctx->paint_idx = 0;
  ctx->paint_buf_size = 1;

  // Reset z indexing
  ctx->z_idx = 0.0f;

  // Add default material
  ctx->paint_buf[ctx->paint_idx] = (msh_draw_paint_t){.fill_color = (msh_draw_color_t){.r=1.0f, .g=0.0f, .b=0.0f}};

  return 1;
}


int 
msh_draw__cmd_compare( const void* a, const void* b)
{
  const msh_draw_cmd_t* cmd_a = (const msh_draw_cmd_t*)a;
  const msh_draw_cmd_t* cmd_b = (const msh_draw_cmd_t*)b;
  // NOTE(maciej): For now we sort based on paint
  return ( cmd_a->paint_idx - cmd_b->paint_idx );
}

int msh_draw_render( msh_draw_ctx_t* ctx )
{
  // Sort draw calls
  // NOTE(maciej): This suggests a standardized size for each
  qsort( ctx->cmd_buf, ctx->cmd_buf_size, sizeof(msh_draw_cmd_t), msh_draw__cmd_compare );
  
  // TODO(maciej): This should be adaptive as well.
  // TODO(maciej): This should be replaced and constructer per material idx.
  int ogl_idx = 0;
  float ogl_buf[4096];
  // if( !ogl_buf )
  // {
  //  ogl_buf = malloc( 4096 * sizeof(float) ); 
  // }

  // NOTE(maciej): REthink this
  int i = 0;
  int n_draw_calls = 0;

  while( i < ctx->cmd_buf_size )
  {
    ogl_idx = 0;
    msh_draw_cmd_t* cur_cmd = &ctx->cmd_buf[i];

    int cur_paint_idx = cur_cmd->paint_idx;

    while( cur_paint_idx == cur_cmd->paint_idx )
    {
      cur_paint_idx = cur_cmd->paint_idx;
      if( cur_cmd->type == MSHD_TRIANGLE )
      {
        float x_pos = cur_cmd->geometry[0];
        float y_pos = cur_cmd->geometry[1];
        float size  = cur_cmd->geometry[2];

        // TODO(maciej): can we not populate this?
        ogl_buf[ogl_idx+0] = x_pos - size;
        ogl_buf[ogl_idx+1] = y_pos - size;
        ogl_buf[ogl_idx+2] = cur_cmd->z_idx;

        ogl_buf[ogl_idx+3] = x_pos + size;
        ogl_buf[ogl_idx+4] = y_pos - size;
        ogl_buf[ogl_idx+5] = cur_cmd->z_idx;

        ogl_buf[ogl_idx+6] = x_pos;
        ogl_buf[ogl_idx+7] = y_pos + 0.8*size;
        ogl_buf[ogl_idx+8] = cur_cmd->z_idx;

        ogl_idx += 9;
      }

      // printf("%d | %d - Uploading triangle %f %f %f using material %d\n", i, ctx->cmd_buf_size, x_pos, y_pos, size, cur_paint_idx );
      cur_cmd = &ctx->cmd_buf[++i];
      if( i >= ctx->cmd_buf_size || ogl_idx >= 4000 ) break;
    }

#ifdef MSH_DRAW_OPENGL
      msh_draw_opengl_backend_t* ogl = &ctx->backend;
      msh_draw_paint_t* paint = &ctx->paint_buf[cur_paint_idx];
      msh_draw_color_t c = paint->fill_color;
      glUseProgram(ogl->prog_id);
      GLuint location = glGetUniformLocation( ogl->prog_id, "input_color" );
      glUniform3f( location, c.r, c.g, c.b );
  
      glBindVertexArray(ogl->vao);
  
      glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
      static long long prev_size = -1;
      long long cur_size = ogl_idx * sizeof(float);
      if( prev_size != cur_size )
      {
        glBufferData(GL_ARRAY_BUFFER, cur_size, ogl_buf, GL_STREAM_DRAW);
      }
      else 
      {
        glBufferSubData(GL_ARRAY_BUFFER, 0, cur_size, ogl_buf );
      }
      prev_size = cur_size;
      glDrawArrays(GL_TRIANGLES, 0, ogl_idx / 3);
      glBindVertexArray(0);
#endif
    n_draw_calls++;
    // printf("------------\n");
  }
  printf("NDrawCalls: %d\n", n_draw_calls );
  
  return 1;
}

int
msh_draw__find_paint( msh_draw_ctx_t* ctx, const msh_draw_paint_t* paint )
{
  // NOTE(maciej): Linear search for now
  int out_idx = -1;
  for( int i = 0 ; i < ctx->paint_buf_size; i++ )
  {
    msh_draw_paint_t cur_paint = ctx->paint_buf[i];
    if( cur_paint.fill_color.r == paint->fill_color.r && 
        cur_paint.fill_color.g == paint->fill_color.g &&
        cur_paint.fill_color.b == paint->fill_color.b )
    {
      out_idx = i;
    }
  }
  return out_idx;
}

void
msh_draw_fill_color( msh_draw_ctx_t* ctx, float r, float g, float b )
{
  // construct candidate paint
  msh_draw_color_t c = {r, g, b};
  msh_draw_paint_t p = (msh_draw_paint_t){.fill_color = c};
  
  // attempt to find it
  int paint_idx = msh_draw__find_paint( ctx, &p );
  if( paint_idx != -1 )
  {
    ctx->paint_idx = paint_idx;
  }
  else
  {
    //NOTE(maciej): For now we just push. Some hashing would be nice here
    if( ctx->paint_buf_size + 1 > ctx->paint_buf_capacity )
    {
      ctx->paint_buf_capacity = (int)(ctx->paint_buf_capacity * 1.5f);
      ctx->paint_buf = (msh_draw_paint_t*)realloc( ctx->paint_buf, ctx->paint_buf_capacity * sizeof(msh_draw_paint_t) );
    }

    ctx->paint_idx = ctx->paint_buf_size; 
    ctx->paint_buf[ ctx->paint_idx ] = p;
    ctx->paint_buf_size = ctx->paint_buf_size + 1;
  }
  // printf("BUF SIZE: %d | BUF CAPACITY %d\n", ctx->paint_buf_size, ctx->paint_buf_capacity );
}

void
msh_draw_triangle( msh_draw_ctx_t* ctx, float x, float y, float s )
{
  if ( ctx->cmd_buf_size + 1 > ctx->cmd_buf_capacity  )
  {
    ctx->cmd_buf_capacity = (int)(ctx->cmd_buf_capacity * 1.5f);
    // TODO(maciej): Handle errors
    ctx->cmd_buf = (msh_draw_cmd_t*)realloc( ctx->cmd_buf, ctx->cmd_buf_capacity * sizeof(msh_draw_cmd_t) );
  }

  ctx->cmd_idx = ctx->cmd_buf_size;
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_TRIANGLE;
  cmd.paint_idx = ctx->paint_idx;
  cmd.geometry[0] = x;
  cmd.geometry[1] = y;
  cmd.geometry[2] = s;
  cmd.z_idx = ctx->z_idx;
  ctx->z_idx -= 0.01;
  ctx->cmd_buf[ctx->cmd_idx] = cmd;
  ctx->cmd_buf_size += 1;
  // printf("Adding trinagle with material %d | %d\n", ctx->cmd_buf[ctx->cmd_idx].paint_idx, ctx->cmd_buf_size );
}
#endif /*MSH_DRAW_IMPLEMENTATION */
