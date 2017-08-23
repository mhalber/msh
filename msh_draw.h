/*
  ==============================================================================
  
  MSH_DRAW.H - RESEARCH DRAWING API!
  
  Intent of this header file is to learn about drawing api's, how they should be
  designed etc.
  Purpose is building a plot app in the future.

  ==============================================================================
  DEPENDENCIES

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
  [x] How to deal with materials - materials need to be hashed somehow. For now linear search
  ==============================================================================
  REFERENCES:
  [1] NanoGUI
 */

#ifndef MSH_DRAW_H
#define MSH_DRAW_H

#define MSH_DRAW_INIT_CMD_BUF_SIZE 1024
#define MSH_DRAW_OGL_BUF_SIZE 8192
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
  msh_draw_color_t fill_color_a;
  msh_draw_color_t fill_color_b;
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
  int viewport_width;
  int viewport_height;

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
int msh_draw_new_frame( msh_draw_ctx_t* ctx, int viewport_width, int viewport_height );
int msh_draw_render( msh_draw_ctx_t* ctx );
void msh_draw_circle( msh_draw_ctx_t* ctx, float x, float y, float r );
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
    layout(location = 1) in vec2 tcoord;
    uniform vec2 viewport_res;

    out vec2 v_tcoord;
    out vec2 v_pos;
    void main()
    {
      v_tcoord = vec2(tcoord.x, tcoord.y);
      v_pos = position.xy;
      gl_Position = vec4( 2.0*position.x/viewport_res.x - 1.0, 
                          1.0 - 2.0*position.y/viewport_res.y, -position.z, 1);
    }
  );
  const char* frag_shdr_src = (const char*) "#version 330 core\n" MSH_DRAW_STRINGIFY
  (
    out vec4 frag_color;
    uniform vec4 color_a;
    uniform vec4 color_b;
    
    in vec2 v_tcoord;
    in vec2 v_pos;

    float roundrect( vec2 p, vec2 e, float r)
    {
        vec2 d = abs(p) - e + vec2(r,r);
        return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;
    }
    // #extension GL_OES_standard_derivatives : enable
    float edgeFactor(){
        vec2 d = fwidth(v_tcoord);
        vec2 a3 = smoothstep(vec2(0.0), d * 50.5, v_tcoord);
        return min(a3.x, a3.y);
    }

    void main()
    {
    //   vec3 output_color = v_tcoord.y * color_a + (1.0-v_tcoord.y) * color_b;
    //   frag_color = vec4(output_color, 1.0); 
    // }

      // This creates bbox as required
      vec2 ar = fwidth(v_tcoord);
      // ar = vec2(1.0, 1.0);
      // ar /= max(ar.x, ar.y);
      vec2 t = v_tcoord * 2.0 - vec2(1.0, 1.0);
      t /= ar;

      vec2 offset = vec2(20, 20);
      vec2 extend = (vec2(1.0, 1.0) / ar) - offset;
      float feather = 20.0;
      float radius = 44.0; // needs pixel ratio.
      float f = clamp(roundrect(t, extend, radius ) / feather, 0.0, 1.0);
      vec4 color = mix(color_a, color_b, f);
      frag_color = color;


    // Probably can do strokes with this!!
    // vec2 t = v_tcoord;
    // vec2 ar = fwidth(v_tcoord);    
    // if( any(   lessThan(t, vec2(10) * ar)) ||
    //     any(greaterThan(t, vec2(1.0) - vec2(10) * ar )) ) {
    //   frag_color = vec4(0.0, 0.0, 0.0, 1.0);
    // }
    // else {
    //   frag_color = vec4(0.5, 0.5, 0.5, 1.0);
    // }

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
  glBufferData(GL_ARRAY_BUFFER, MSH_DRAW_OGL_BUF_SIZE * sizeof(float), NULL, GL_STREAM_DRAW);

  GLuint pos_attrib = glGetAttribLocation( ogl->prog_id, "position" );
  GLuint tcoord_attrib = glGetAttribLocation( ogl->prog_id, "tcoord" );
  glEnableVertexAttribArray(pos_attrib);
  glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 );
  glEnableVertexAttribArray(tcoord_attrib);
  glVertexAttribPointer(tcoord_attrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) );

  glBindVertexArray( 0 );
  glDisableVertexAttribArray( pos_attrib );
#endif
  return 1;
}

int msh_draw_new_frame( msh_draw_ctx_t* ctx, int viewport_width, int viewport_height )
{
  // Update viewport size
  ctx->viewport_width = viewport_width;
  ctx->viewport_height = viewport_height;

  // Reset indices
  ctx->cmd_idx = 0;
  ctx->cmd_buf_size = 0;
  ctx->paint_idx = 0;
  ctx->paint_buf_size = 1;

  // Reset z indexing
  ctx->z_idx = 0.0f;

  // Add default material
  ctx->paint_buf[ctx->paint_idx] = (msh_draw_paint_t){.fill_color_a = (msh_draw_color_t){.r=1.0f, .g=0.0f, .b=0.0f},
                                                      .fill_color_b = (msh_draw_color_t){.r=0.0f, .g=0.1f, .b=0.0f}};

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
  qsort( ctx->cmd_buf, ctx->cmd_buf_size, sizeof(msh_draw_cmd_t), msh_draw__cmd_compare );
  
  // TODO(maciej): Custom meshes will probably be quite big. Maybe we need to allocate more for them.
  // Big meshes should probably not be uploaded every frame though...
  int ogl_idx = 0;
  float ogl_buf[MSH_DRAW_OGL_BUF_SIZE];

  // Gather commands and draw
  int n_draw_calls = 0;
  int i = 0;
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
        if( ogl_idx + 9 > MSH_DRAW_OGL_BUF_SIZE ) break;
        float x_pos = cur_cmd->geometry[0];
        float y_pos = cur_cmd->geometry[1];
        float size  = cur_cmd->geometry[2];

        // TODO(maciej): can we not populate this?
        ogl_buf[ogl_idx+0] = x_pos;
        ogl_buf[ogl_idx+1] = y_pos + size;
        ogl_buf[ogl_idx+2] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+3] = 0.5;
        ogl_buf[ogl_idx+4] = 0.0;

        ogl_buf[ogl_idx+5] = x_pos + 0.866025404f * size;
        ogl_buf[ogl_idx+6] = y_pos - 0.5f * size;
        ogl_buf[ogl_idx+7] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+8] = 1.0;
        ogl_buf[ogl_idx+9] = 0.0;

        ogl_buf[ogl_idx+10] = x_pos - 0.86602540378f * size;
        ogl_buf[ogl_idx+11] = y_pos - 0.5f * size;
        ogl_buf[ogl_idx+12] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+13] = 0.0;
        ogl_buf[ogl_idx+14] = 0.0;

        ogl_idx += 15;
      }
      if( cur_cmd->type == MSHD_RECTANGLE )
      {
        if( ogl_idx + 18 > MSH_DRAW_OGL_BUF_SIZE ) break;
        float x1 = cur_cmd->geometry[0];
        float y1 = cur_cmd->geometry[1];
        float x2 = cur_cmd->geometry[2];
        float y2 = cur_cmd->geometry[3];

        ogl_buf[ogl_idx+0] = x1;
        ogl_buf[ogl_idx+1] = y1;
        ogl_buf[ogl_idx+2] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+3] = 0.0f;
        ogl_buf[ogl_idx+4] = 0.0f;

        ogl_buf[ogl_idx+5] = x1;
        ogl_buf[ogl_idx+6] = y2;
        ogl_buf[ogl_idx+7] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+8] = 0.0f;
        ogl_buf[ogl_idx+9] = 1.0f;

        ogl_buf[ogl_idx+10] = x2;
        ogl_buf[ogl_idx+11] = y2;
        ogl_buf[ogl_idx+12] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+13] = 1.0f;
        ogl_buf[ogl_idx+14] = 1.0f;


        ogl_buf[ogl_idx+15] = x2;
        ogl_buf[ogl_idx+16] = y2;
        ogl_buf[ogl_idx+17] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+18] = 1.0f;
        ogl_buf[ogl_idx+19] = 1.0f;

        ogl_buf[ogl_idx+20] = x2;
        ogl_buf[ogl_idx+21] = y1;
        ogl_buf[ogl_idx+22] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+23] = 1.0f;
        ogl_buf[ogl_idx+24] = 0.0f;

        ogl_buf[ogl_idx+25] = x1;
        ogl_buf[ogl_idx+26] = y1;
        ogl_buf[ogl_idx+27] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+28] = 0.0;
        ogl_buf[ogl_idx+29] = 0.0;

        ogl_idx += 30;
      }
      else if (cur_cmd->type == MSHD_CIRCLE)
      {
        float x_pos = cur_cmd->geometry[0];
        float y_pos = cur_cmd->geometry[1];
        float rad   = cur_cmd->geometry[2];
        
        // TODO(maciej): Decide resolution as a function of radius
        int res = (int)(0.5f * rad + 8.0f);
        float delta = 2.0f*(float)msh_PI / res;
        // if( ogl_idx + res > MSH_DRAW_OGL_BUF_SIZE ) break;
        
        // NOTE(maciej): This would be better drawn with GL_TRIANGLE_FAN...
        // Maybe add it to the list of things we want to sort on. Or just use element buffer
        int offset = 0;
        for( int step = 0; step < res; ++step )
        {
          float theta = step * delta;
          float omega = (step + 1) * delta;
          float sint = sinf(theta);
          float cost = cosf(theta);
          float sino = sinf(omega);
          float coso = cosf(omega);

          ogl_buf[ogl_idx+(offset++)] = x_pos;
          ogl_buf[ogl_idx+(offset++)] = y_pos;
          ogl_buf[ogl_idx+(offset++)] = cur_cmd->z_idx;
          ogl_buf[ogl_idx+(offset++)] = 0.5;
          ogl_buf[ogl_idx+(offset++)] = 0.5;
  
          ogl_buf[ogl_idx+(offset++)] = x_pos + sint * rad;
          ogl_buf[ogl_idx+(offset++)] = y_pos + cost * rad;
          ogl_buf[ogl_idx+(offset++)] = cur_cmd->z_idx;
          ogl_buf[ogl_idx+(offset++)] = 0.5f * (sint + 1.0f);
          ogl_buf[ogl_idx+(offset++)] = 0.5f * (cost + 1.0f);
  
          ogl_buf[ogl_idx+(offset++)] = x_pos + sino * rad;
          ogl_buf[ogl_idx+(offset++)] = y_pos + coso * rad;
          ogl_buf[ogl_idx+(offset++)] = cur_cmd->z_idx;
          ogl_buf[ogl_idx+(offset++)] = 0.5f * (sino + 1.0f);
          ogl_buf[ogl_idx+(offset++)] = 0.5f * (coso + 1.0f);
        }
        printf("%d | %d | %d | %d\n", res, res * 5 * 3, offset, ogl_idx );
        ogl_idx += offset;
      }

      // printf("%d | %d - Uploading triangle %f %f %f using material %d\n", i, ctx->cmd_buf_size, x_pos, y_pos, size, cur_paint_idx );
      cur_cmd = &ctx->cmd_buf[++i];
      if( i >= ctx->cmd_buf_size ) break;
    }

#ifdef MSH_DRAW_OPENGL
      msh_draw_opengl_backend_t* ogl = &ctx->backend;
      msh_draw_paint_t* paint = &ctx->paint_buf[cur_paint_idx];
      msh_draw_color_t c_a = paint->fill_color_a;
      msh_draw_color_t c_b = paint->fill_color_b;
      glUseProgram(ogl->prog_id);
      GLuint location = glGetUniformLocation( ogl->prog_id, "color_a" );
      glUniform4f( location, c_a.r, c_a.g, c_a.b, 1.0f );
      location = glGetUniformLocation( ogl->prog_id, "color_b" );
      glUniform4f( location, c_b.r, c_b.g, c_b.b, 0.0f );
      location = glGetUniformLocation( ogl->prog_id, "viewport_res" );

      glUniform2f( location, (float)ctx->viewport_width, (float)ctx->viewport_height );

      glBindVertexArray(ogl->vao);
  
      // NOTE(maciej): We will probably be drawing in chunks, so no need for this resize
      glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, ogl_idx*sizeof(float), ogl_buf );
      
      glDrawArrays(GL_TRIANGLES, 0, ogl_idx / 5);
      glBindVertexArray(0);
#endif
    n_draw_calls++;
  }
  printf("NDrawCalls: %d | %d\n", n_draw_calls, ctx->cmd_buf_size );
  
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
    if( cur_paint.fill_color_a.r == paint->fill_color_a.r && 
        cur_paint.fill_color_a.g == paint->fill_color_a.g &&
        cur_paint.fill_color_a.b == paint->fill_color_a.b &&
        cur_paint.fill_color_b.r == paint->fill_color_b.r && 
        cur_paint.fill_color_b.g == paint->fill_color_b.g &&
        cur_paint.fill_color_b.b == paint->fill_color_b.b )
    {
      out_idx = i;
    }
  }
  return out_idx;
}

//TODO(maciej):Separate buffer resize
void
msh_draw_gradient( msh_draw_ctx_t* ctx, float r1, float g1, float b1,
                                        float r2, float g2, float b2 )
{
  // construct candidate paint
  msh_draw_color_t c_a = {r1, g1, b1};
  msh_draw_color_t c_b = {r2, g2, b2};
  msh_draw_paint_t p = (msh_draw_paint_t){.fill_color_a = c_a,
                                          .fill_color_b = c_b};
  
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
msh_draw_fill_color( msh_draw_ctx_t* ctx, float r, float g, float b )
{
  // construct candidate paint
  msh_draw_color_t c = {r, g, b};
  msh_draw_paint_t p = (msh_draw_paint_t){.fill_color_a = c,
                                          .fill_color_b = c};
  
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

// TODO(maciej): This defienitly should return an error code
void
msh_draw__resize_cmd_buf( msh_draw_ctx_t* ctx, int n_cmds )
{
  if ( ctx->cmd_buf_size + n_cmds > ctx->cmd_buf_capacity )
  {
    ctx->cmd_buf_capacity = (int)(ctx->cmd_buf_capacity * 1.5f);
    ctx->cmd_buf = (msh_draw_cmd_t*)realloc( ctx->cmd_buf, ctx->cmd_buf_capacity * sizeof(msh_draw_cmd_t) );
  }
}

void
msh_draw_triangle( msh_draw_ctx_t* ctx, float x, float y, float s )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_TRIANGLE;
  cmd.paint_idx = ctx->paint_idx;
  cmd.geometry[0] = x;
  cmd.geometry[1] = y;
  cmd.geometry[2] = s;
  cmd.z_idx = ctx->z_idx;
  
  // Modify the context
  ctx->z_idx += 0.001f;
  ctx->cmd_idx = ctx->cmd_buf_size;  
  ctx->cmd_buf[ctx->cmd_idx] = cmd;
  ctx->cmd_buf_size += 1;
}

void 
msh_draw_circle( msh_draw_ctx_t* ctx, float x, float y, float r )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_CIRCLE;
  cmd.paint_idx = ctx->paint_idx;
  cmd.geometry[0] = x;
  cmd.geometry[1] = y;
  cmd.geometry[2] = r;
  cmd.z_idx = ctx->z_idx;
  
  // Modify the context
  ctx->z_idx += 0.001f;
  ctx->cmd_idx = ctx->cmd_buf_size;  
  ctx->cmd_buf[ctx->cmd_idx] = cmd;
  ctx->cmd_buf_size += 1;
}

void 
msh_draw_rectangle( msh_draw_ctx_t* ctx, float x1, float y1, float x2, float y2 )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_RECTANGLE;
  cmd.paint_idx = ctx->paint_idx;
  cmd.geometry[0] = x1;
  cmd.geometry[1] = y1;
  cmd.geometry[2] = x2;
  cmd.geometry[3] = y2;
  cmd.z_idx = ctx->z_idx;
  
  // Modify the context
  ctx->z_idx += 0.001f;
  ctx->cmd_idx = ctx->cmd_buf_size;  
  ctx->cmd_buf[ctx->cmd_idx] = cmd;
  ctx->cmd_buf_size += 1;
}

#endif /*MSH_DRAW_IMPLEMENTATION */
