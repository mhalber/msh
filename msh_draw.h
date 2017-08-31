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
  [ ] OpenGL backend
  [ ] Software Rasterizer backend
  [ ] Rethink the name
  [x] How to deal with materials - materials need to be hashed somehow. For now linear search.
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
  MSHD_TRIANGLE,  MSHD_CIRCLE, MSHD_RECTANGLE
} msh_draw_cmd_type;

typedef enum
{
  MSHD_FLAT = 0, MSHD_LINEAR_GRADIENT, MSHD_RADIAL_GRADIENT, MSHD_BOX_GRADIENT,
  MSHD_POLAR_GRADIENT, MSHD_IMAGE
} msh_draw_paint_type;

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
  msh_draw_paint_type type;
  msh_draw_color_t fill_color_a;
  msh_draw_color_t fill_color_b;
  float offset_x, offset_y;
  float feather, radius;
  int image_idx;
} msh_draw_paint_t;

typedef struct msh_draw_cmd
{
  msh_draw_cmd_type type;
  int paint_idx;
  float geometry[10];
  float z_idx;
} msh_draw_cmd_t;

typedef struct msh_draw_image
{
  int id;
  int width;
  int height;
  int n_channels;
} msh_draw_image_t;

typedef struct msh_draw_ctx
{
  int viewport_width;
  int viewport_height;

  int cmd_buf_size;
  int cmd_buf_capacity;
  msh_draw_cmd_t* cmd_buf;
  int cmd_idx;

  // TODO(maciej): Switch to a hash-table?
  int paint_buf_size;
  int paint_buf_capacity;
  msh_draw_paint_t* paint_buf;
  int paint_idx;

  int image_buf_size;
  int image_buf_capacity;
  msh_draw_image_t* image_buf;
  int image_idx;

  float z_idx;

#ifdef MSH_DRAW_OPENGL
  msh_draw_opengl_backend_t backend;
#endif
} msh_draw_ctx_t;


int msh_draw_init_ctx( msh_draw_ctx_t* ctx );
int msh_draw_new_frame( msh_draw_ctx_t* ctx, int viewport_width, int viewport_height );
int msh_draw_render( msh_draw_ctx_t* ctx );

void msh_draw_set_paint( msh_draw_ctx_t* ctx, const int paint_idx );
const int msh_draw_flat_fill( msh_draw_ctx_t* ctx, float r, float g, float b, float a );
const int msh_draw_box_gradient( /*TODO(maciej):Find correct params*/ );
const int msh_draw_radial_gradient( msh_draw_ctx_t* ctx, 
                                   float r1, float g1, float, float b1, float a1,
                                   float r2, float g2, float, float b2, float a2,
                                   float size, float feather );

// TODO(maciej): More primitives
void msh_draw_circle( msh_draw_ctx_t* ctx, float x, float y, float r );
void msh_draw_triangle( msh_draw_ctx_t* ctx, float x, float y, float s );
void msh_draw_line( msh_draw_ctx_t* ctx, float x1, float y1, float x2, float y2 );

#endif /*MSH_DRAW_H*/


////////////////////////////////////////////////////////////////////////////////


#ifdef MSH_DRAW_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////////////
// INTERNAL UTILITY
////////////////////////////////////////////////////////////////////////////////


int 
msh_draw__cmd_compare( const void* a, const void* b)
{
  const msh_draw_cmd_t* cmd_a = (const msh_draw_cmd_t*)a;
  const msh_draw_cmd_t* cmd_b = (const msh_draw_cmd_t*)b;
  // NOTE(maciej): For now we sort based on paint
  return ( cmd_a->paint_idx - cmd_b->paint_idx );
}

// TODO(maciej): Resize function defienitly should return an error code
void
msh_draw__resize_paint_buf( msh_draw_ctx_t* ctx, int n_paints )
{
  if ( ctx->paint_buf_size + n_paints > ctx->paint_buf_capacity )
  {
    ctx->paint_buf_capacity = (int)(ctx->paint_buf_capacity * 1.5f);
    ctx->paint_buf = (msh_draw_paint_t*)realloc( ctx->paint_buf, 
                           ctx->paint_buf_capacity * sizeof(msh_draw_paint_t) );
  }
}

void
msh_draw__resize_image_buf( msh_draw_ctx_t* ctx, int n_images )
{
  if ( ctx->image_buf_size + n_images > ctx->image_buf_capacity )
  {
    ctx->image_buf_capacity = (int)(ctx->image_buf_capacity * 1.5f);
    ctx->image_buf = (msh_draw_image_t*)realloc( ctx->image_buf, 
                           ctx->image_buf_capacity * sizeof(msh_draw_image_t) );
  }
}

void
msh_draw__resize_cmd_buf( msh_draw_ctx_t* ctx, int n_cmds )
{
  
  if ( ctx->cmd_buf_size + n_cmds > ctx->cmd_buf_capacity )
  {
    ctx->cmd_buf_capacity = (int)(ctx->cmd_buf_capacity * 1.5f);
    ctx->cmd_buf = (msh_draw_cmd_t*)realloc( ctx->cmd_buf, 
                               ctx->cmd_buf_capacity * sizeof(msh_draw_cmd_t) );
  }
}

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

////////////////////////////////////////////////////////////////////////////////
// Initialization
// NOTE: Sets up context. Depending on the backend, all backend variables will
//       be initially setup here
////////////////////////////////////////////////////////////////////////////////


int 
msh_draw_init_ctx( msh_draw_ctx_t* ctx )
{
  // Setup command buffer
  ctx->cmd_buf_capacity = MSH_DRAW_INIT_CMD_BUF_SIZE;
  ctx->cmd_buf = (msh_draw_cmd_t*)malloc( ctx->cmd_buf_capacity * sizeof(msh_draw_cmd_t) );

  // Setup paint buffer
  ctx->paint_buf_capacity = MSH_DRAW_INIT_CMD_BUF_SIZE;
  ctx->paint_buf = (msh_draw_paint_t*)malloc( ctx->paint_buf_capacity * sizeof(msh_draw_paint_t) );

  // Setup image buffer
  ctx->image_buf_capacity = MSH_DRAW_INIT_CMD_BUF_SIZE;
  ctx->image_buf = (msh_draw_image_t*)malloc( ctx->image_buf_capacity * sizeof(msh_draw_image_t) );


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
    uniform vec4 gradient_params;
    uniform int paint_type;
    uniform sampler2D tex;

    in vec2 v_tcoord;
    in vec2 v_pos;

    float roundrect( vec2 p, vec2 e, float r)
    {
        vec2 d = abs(p) - e + vec2(r,r);
        return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;
    }

    vec3 calculate_scaling( in vec2 t_coord )
    {
      // scaling.xy will scale coordinate system to conform to aspect ratio
      // scaling.z will convert user provided pixel values to approperiately scaled values in paint space
      vec2 ar      = fwidth(t_coord);
      vec3 scaling = vec3( (ar/min(ar.x, ar.y)).yx, max(ar.x, ar.y) );
      return scaling;  
    }

    vec2 calculate_paint_space( in vec2 t_coord, in vec2 scaling, in int type )
    {
      if      ( type == 1 /*LINEAR*/ ) { return t_coord; }
      else if ( type == 2 /*RADIAL*/ ) { return ((t_coord * 2.0 - vec2(1.0, 1.0)) * scaling); }
      else if ( type == 3 /*BOX*/)     { return ((t_coord * 2.0 - vec2(1.0, 1.0)) * scaling); } 
      else if ( type == 4 /*POLAR*/)   { return ((t_coord * 2.0 - vec2(1.0, 1.0)) * scaling); } 
      else if ( type == 5 /*IMAGE*/)   { return t_coord; }
      else { return t_coord; }
    }

    vec4 linear_gradient( in vec4 c_a, in vec4 c_b, in vec2 t )
    {
      return mix( c_a, c_b, t.y );
    }

    vec4 radial_gradient( in vec4 c_a, in vec4 c_b, in vec4 params, in vec2 t, in float normalizer )
    {
      float radius  = params.w  * normalizer;
      float feather = params.z  * normalizer;
      vec2 extend   = params.xy * normalizer;
      float f = clamp(roundrect(t, extend, radius ) / feather, 0.0, 1.0);
      return mix(c_a, c_b, f);
    }

    vec4 box_gradient( in vec4 c_a, in vec4 c_b, in vec4 params, in vec2 t, in vec3 scaling_params )
    {
      vec2 scaling     = scaling_params.xy;
      float normalizer = scaling_params.z;
      float radius  = params.w  * normalizer;
      float feather = params.z  * normalizer;
      vec2 extend = vec2(1.0, 1.0) * scaling - gradient_params.xy * normalizer;
      float f = clamp(roundrect(t, extend, radius ) / feather, 0.0, 1.0);
      return mix(c_a, c_b, f);
    }

    vec4 polar_gradient( in vec4 c_a, in vec4 c_b, in vec2 t)
    {
      // TODO(maciej): Test different mixing factors
      float f = ((atan( t.x, t.y) * 0.15915494309189533576888376337251 /*OneOverTau*/) ) +0.5;
      return mix(c_a, c_b, f);
    }

    void main()
    {
      vec3 s = calculate_scaling( v_tcoord );
      vec2 t = calculate_paint_space( v_tcoord, s.xy, paint_type ); 
      frag_color = vec4(t.xy, 0.0, 1.0);
      if      ( paint_type == 1 ) { frag_color=linear_gradient(color_a, color_b, t);}
      else if ( paint_type == 2 ) { frag_color=radial_gradient(color_a, color_b, gradient_params, t, s.z);}
      else if ( paint_type == 3 ) { frag_color=box_gradient(color_a, color_b, gradient_params, t, s);}
      else if ( paint_type == 4 ) { frag_color=polar_gradient(color_a, color_b, t);}
      else if ( paint_type == 5 ) { frag_color=texture(tex, t);}
      else                        { frag_color=color_a; }
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
  ctx->image_idx = 0;
  ctx->image_buf_size = 1;

  // Reset z indexing
  ctx->z_idx = 0.0f;

  // Add default material
  msh_draw_flat_fill(ctx, 0.5f, 0.5f, 0.5f, 1.0f);

  return 1;
}



////////////////////////////////////////////////////////////////////////////////
// MAIN DRAW CALL
// NOTE: The calls below translate what user has submitted to the context at
//       the end of each frame and do the actual drawing
////////////////////////////////////////////////////////////////////////////////

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


        ogl_buf[ogl_idx+15] = x1;
        ogl_buf[ogl_idx+16] = y1;
        ogl_buf[ogl_idx+17] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+18] = 0.0f;
        ogl_buf[ogl_idx+19] = 0.0f;

        ogl_buf[ogl_idx+20] = x2;
        ogl_buf[ogl_idx+21] = y2;
        ogl_buf[ogl_idx+22] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+23] = 1.0f;
        ogl_buf[ogl_idx+24] = 1.0f;

        ogl_buf[ogl_idx+25] = x2;
        ogl_buf[ogl_idx+26] = y1;
        ogl_buf[ogl_idx+27] = cur_cmd->z_idx;
        ogl_buf[ogl_idx+28] = 1.0;
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
        // printf("%d | %d | %d | %d\n", res, res * 5 * 3, offset, ogl_idx );
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
      if( paint->image_idx ) 
      {
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture( GL_TEXTURE_2D, paint->image_idx);
        printf("TEST\n");
      }
      
      //TODO(maciej): Check different ways for setting up an uniform
      GLuint location = glGetUniformLocation( ogl->prog_id, "color_a" );
      glUniform4f( location, c_a.r, c_a.g, c_a.b, 1.0f );
      location = glGetUniformLocation( ogl->prog_id, "color_b" );
      glUniform4f( location, c_b.r, c_b.g, c_b.b, 1.0f );
      location = glGetUniformLocation(ogl->prog_id, "gradient_params" );
      glUniform4f( location, paint->offset_x, paint->offset_y, paint->feather, paint->radius );
      location = glGetUniformLocation( ogl->prog_id, "paint_type" );
      glUniform1i( location, (int)paint->type );
      location = glGetUniformLocation( ogl->prog_id, "tex" );
      glUniform1i( location, (int)paint->image_idx );
      location = glGetUniformLocation( ogl->prog_id, "viewport_res" );
      glUniform2f( location, (float)ctx->viewport_width, (float)ctx->viewport_height );
    
      glBindVertexArray(ogl->vao);
  
      
      // NOTE(maciej): We will probably be drawing in chunks, so no need for this resize
      glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, ogl_idx*sizeof(float), ogl_buf );
      
      glDrawArrays(GL_TRIANGLES, 0, ogl_idx / 5);
      
      if( paint->image_idx ) 
      {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture( GL_TEXTURE_2D, 0);
      }

      glBindVertexArray(0);
#endif
    n_draw_calls++;
  }
  printf("NDrawCalls: %d | %d\n", n_draw_calls, ctx->cmd_buf_size );
  
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Paint management
////////////////////////////////////////////////////////////////////////////////

int
msh_draw__find_paint( msh_draw_ctx_t* ctx, const msh_draw_paint_t* paint )
{
  // NOTE(maciej): Linear search for now
  // TODO(maciej): Prodcue hash from bit representation of a paint?
  int out_idx = -1;
  for( int i = 0 ; i < ctx->paint_buf_size; i++ )
  {
    msh_draw_paint_t cur_paint = ctx->paint_buf[i];
    if( cur_paint.type           == paint->type &&
        cur_paint.image_idx      == paint->image_idx &&
        cur_paint.fill_color_a.r == paint->fill_color_a.r && 
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

const int msh_draw__add_paint( msh_draw_ctx_t* ctx, msh_draw_paint_t* p )
{
  int paint_idx = msh_draw__find_paint( ctx, p );
  if( paint_idx != -1 )
  {
    ctx->paint_idx = paint_idx;
  }
  else
  {
    //NOTE(maciej): For now we just push. Some hashing would be nice here
    msh_draw__resize_paint_buf( ctx, 1 );

    ctx->paint_idx = ctx->paint_buf_size; 
    ctx->paint_buf[ ctx->paint_idx ] = *p;
    ctx->paint_buf_size = ctx->paint_buf_size + 1;
  }
  return ctx->paint_idx;
}

const int
msh_draw_flat_fill( msh_draw_ctx_t* ctx, float r, float g, float b, float a )
{
  msh_draw_color_t c = {.r=r, .g=g, .b=b, .a=a};
  msh_draw_paint_t p = (msh_draw_paint_t){.type=MSHD_FLAT,
                                          .fill_color_a = c, .fill_color_b = c,
                                          .offset_x = 0.0f, .offset_y = 0.0f,
                                          .feather = 1.0f,  .radius = 0.0f };

  return msh_draw__add_paint( ctx, &p );
}

const int
msh_draw_linear_gradient_fill( msh_draw_ctx_t* ctx, float r1, float g1, float b1, float a1,
                                                    float r2, float g2, float b2, float a2 )
{
  msh_draw_color_t c1 = {.r=r1, .g=g1, .b=b1, .a=a1};
  msh_draw_color_t c2 = {.r=r2, .g=g2, .b=b2, .a=a2};
  msh_draw_paint_t p = (msh_draw_paint_t){.type=MSHD_LINEAR_GRADIENT,
                                          .fill_color_a = c1, .fill_color_b = c2,
                                          .offset_x = 0.0, .offset_y = 0.0 };

  return msh_draw__add_paint( ctx, &p );
}

const int
msh_draw_radial_gradient_fill( msh_draw_ctx_t* ctx, float r1, float g1, float b1, float a1,
                                                 float r2, float g2, float b2, float a2, 
                                                 float feather, float radius )
{
  msh_draw_color_t c1 = {.r=r1, .g=g1, .b=b1, .a=a1};
  msh_draw_color_t c2 = {.r=r2, .g=g2, .b=b2, .a=a2};
  msh_draw_paint_t p = (msh_draw_paint_t){.type=MSHD_RADIAL_GRADIENT,
                                          .fill_color_a = c1, .fill_color_b = c2,
                                          .offset_x = radius, .offset_y = radius,
                                          .feather = feather,  .radius = radius };

  return msh_draw__add_paint( ctx, &p );
}


const int
msh_draw_box_gradient_fill( msh_draw_ctx_t* ctx, float r1, float g1, float b1, float a1,
                                                 float r2, float g2, float b2, float a2, 
                                                 float offset, float feather, float radius )
{
  msh_draw_color_t c1 = {.r=r1, .g=g1, .b=b1, .a=a1};
  msh_draw_color_t c2 = {.r=r2, .g=g2, .b=b2, .a=a2};
  msh_draw_paint_t p = (msh_draw_paint_t){.type=MSHD_BOX_GRADIENT,
                                          .fill_color_a = c1, .fill_color_b = c2,
                                          .offset_x = offset, .offset_y = offset,
                                          .feather = feather,  .radius = radius };

  return msh_draw__add_paint( ctx, &p );
}

const int
msh_draw_polar_gradient_fill( msh_draw_ctx_t* ctx, float r1, float g1, float b1, float a1,
                                                    float r2, float g2, float b2, float a2 )
{
  msh_draw_color_t c1 = {.r=r1, .g=g1, .b=b1, .a=a1};
  msh_draw_color_t c2 = {.r=r2, .g=g2, .b=b2, .a=a2};
  msh_draw_paint_t p = (msh_draw_paint_t){.type=MSHD_POLAR_GRADIENT,
                                          .fill_color_a = c1, .fill_color_b = c2,
                                          .offset_x = 0.0, .offset_y = 0.0 };

  return msh_draw__add_paint( ctx, &p );
}


// TODO(maciej): Create image in opengl, push it onto the image stack( just like commands and paints)
// TODO(maciej): Should return index, but push the struct onto an array.
const int
msh_draw_register_image( msh_draw_ctx_t* ctx, unsigned char* data, int w, int h, int n )
{
  // NOTE(maciej): Can id of an image/paint can be used as their identifying property AND index to an array?
  msh_draw_image_t tex;
  tex.width = w;
  tex.height = h;
  tex.n_channels = n;
  msh_draw__resize_image_buf( ctx, 1 );
  ctx->image_idx = ctx->image_buf_size; 
  ctx->image_buf[ ctx->image_idx ] = tex;
  ctx->image_buf_size = ctx->image_buf_size + 1;

  glGenTextures(1, &tex.id);
  glActiveTexture(GL_TEXTURE0 + 1);//TODO(maciej): What does nanovg do with texture unit activation?
  glBindTexture(GL_TEXTURE_2D, tex.id);
  // TODO(maciej): Use flags to resolve this
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  // TODO(maciej): Switch based on the number of components
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
  glBindTexture( GL_TEXTURE_2D, 0 );

  return tex.id;
}

// NOTE(maciej): Paint just selects an image.
// NOTE(maciej): This again points out that we need to come up with paint building api.
const int
msh_draw_image_fill( msh_draw_ctx_t* ctx, int image_idx )
{
  msh_draw_paint_t p = (msh_draw_paint_t){.type=MSHD_IMAGE,
                                          .image_idx=image_idx };

  return msh_draw__add_paint( ctx, &p );
}

void 
msh_draw_set_paint( msh_draw_ctx_t* ctx, const int paint_idx )
{
  ctx->paint_idx = paint_idx;
}


////////////////////////////////////////////////////////////////////////////////
// DRAW PRIMITIVES
////////////////////////////////////////////////////////////////////////////////

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
