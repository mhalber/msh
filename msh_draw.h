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
  [ ] Change the name to msh_cutouts
  [ ] Add Paint Builder
  [ ] Add Shape Builder
  [ ] Add Triangulation
  [ ] Add line mitter/caps/bevels etc.
  [ ] Strokes 
  [ ] Text/Fonts are very special, and should be treated differently
  [ ] OpenGL backend
  [ ] Blending
  [ ] Antialiasing
  [ ] Software Rasterizer backend
  [x] How to deal with materials - materials need to be hashed somehow. For now linear search.
  ==============================================================================
  REFERENCES:
  [1] NanoGUI
  [2] stb_truetype
 */

#ifndef MSH_DRAW_H
#define MSH_DRAW_H

// Options
#define MSH_DRAW_INIT_CMD_BUF_SIZE 1024
#define MSH_DRAW_OGL_BUF_SIZE 8192
#define MSH_DRAW_STRINGIFY(x) #x
#define MSH_DRAW_OPENGL

#define MSH_FONT_MAX_FILE_SIZE 256000
#define MSH_FONT_RES 512
#define MSH_FONT_MAX_CHARS 128 //NOTE(maciej): Support for extended ascii only | This needs to be done once per font size.

// Aliases for stb_truetype types
typedef stbtt_packedchar msh_draw_packedchar_t;
typedef stbtt_aligned_quad msh_draw_aligned_quad_t;

typedef enum
{
  MSHD_RECTANGLE, MSHD_ARC, MSHD_POLYGON, MSHD_LINE_START, MSHD_LINE_TO, MSHD_LINE_END, MSHD_TEXT
} msh_draw_cmd_type;

typedef enum
{
  MSHD_FLAT = 0, MSHD_LINEAR_GRADIENT, MSHD_RADIAL_GRADIENT, MSHD_BOX_GRADIENT,
  MSHD_POLAR_GRADIENT, MSHD_IMAGE, MSHD_FONT
} msh_draw_paint_type;

#ifdef MSH_DRAW_OPENGL
typedef struct msh_draw_ogl_backend
{
  GLuint vert_shdr_id;
  GLuint frag_shdr_id;
  GLuint prog_id;
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
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
  int id;
  msh_draw_paint_type type;
  msh_draw_color_t fill_color_a;
  msh_draw_color_t fill_color_b;
  float offset_x, offset_y;
  float feather, radius;
  int image_idx;
} msh_draw_paint_t;

typedef struct msh_draw_cmd
{
  int id;
  msh_draw_cmd_type type;
  int paint_id;
  float geometry[10]; //TODO(maciej): Make this into buffer, as ints
  float z_idx;
  const char* str;//TODO(maciej): Put that in the buffer if cmd type is text; 
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
  int paint_id;

  int image_buf_size;
  int image_buf_capacity;
  msh_draw_image_t* image_buf;
  int image_idx;

  msh_draw_packedchar_t char_info[MSH_FONT_MAX_CHARS];

  float z_idx;

#ifdef MSH_DRAW_OPENGL
  msh_draw_opengl_backend_t backend;
#endif
} msh_draw_ctx_t;


int msh_draw_init_ctx( msh_draw_ctx_t* ctx );
int msh_draw_new_frame( msh_draw_ctx_t* ctx, int viewport_width, int viewport_height );
int msh_draw_render( msh_draw_ctx_t* ctx );

void msh_draw_set_paint( msh_draw_ctx_t* ctx, const int paint_id );
const int msh_draw_register_image( msh_draw_ctx_t* ctx, unsigned char* data, int w, int h, int n );
const int msh_draw_flat_fill( msh_draw_ctx_t* ctx, float r, float g, float b, float a );
const int msh_draw_box_gradient( /*TODO(maciej):Find correct params*/ );
const int msh_draw_radial_gradient( msh_draw_ctx_t* ctx, 
                                   float r1, float g1, float, float b1, float a1,
                                   float r2, float g2, float, float b2, float a2,
                                   float size, float feather );
const int msh_draw_image_fill( msh_draw_ctx_t* ctx, int image_idx );


// TODO(maciej): More primitives
void msh_draw_circle( msh_draw_ctx_t* ctx, float x, float y, float r );
void msh_draw_line_start( msh_draw_ctx_t* ctx, float x1, float y1 );
void msh_draw_line_to( msh_draw_ctx_t* ctx, float x1, float y1 );
void msh_draw_line_end( msh_draw_ctx_t* ctx, float x1, float y1 );

int msh_draw_add_font( msh_draw_ctx_t* ctx, const char* filename, const float size );


// ALTERNATIVE API
// The heart of this api needs to be polygon triangulator. I will either use 
// earclipping (possibly the FIST variant) or monotone subdivision

typedef struct msh_cutouts_path
{
  float vertices[1024 * 2];
  
  // cursor (?)
  float cur_x;
  float cur_y;

  int idx;
} msh_cutouts_path_t;

// These functions are path builders.
int msh_cutouts_path_begin(msh_cutouts_path_t* path, float x, float y);
int msh_cutouts_move_to(msh_cutouts_path_t* path, float x, float y);
int msh_cutouts_line_to(msh_cutouts_path_t* path, float x, float y);
int msh_cutouts_path_close(msh_cutouts_path_t* path);
int msh_cutouts_path_end(msh_cutouts_path_t* path);

int msh_cutouts__path_to_shape();// This function will do the triangulation


int msh_cutouts_path_begin(msh_cutouts_path_t* path, float x, float y)
{
  path->vertices[0] = x;
  path->vertices[1] = y;
  path->idx = 1;
  return 1;
}

int msh_cutouts_line_to(msh_cutouts_path_t* path, float x, float y)
{
  path->vertices[2*path->idx  ] = x;
  path->vertices[2*path->idx+1] = y;
  path->idx += 1;
  return 1;
}

int msh_cutouts_path_close(msh_cutouts_path_t *path)
{
  path->vertices[2*path->idx  ] = path->vertices[0];
  path->vertices[2*path->idx+1] = path->vertices[1];
  path->idx += 1;
  return msh_cutouts_path_end(path);
}

int msh_cutouts_path_end(msh_cutouts_path_t *path)
{
  //This command will move path to some other buffer
  return 1;
}

float msh_cutouts__signed_area(float x1, float y1, 
                             float x2, float y2, 
                             float x3, float y3)
{
  // NOTE(maciej): in our case positive y goes "down" hence need to flip this.
  return -0.5f*(-x2*y1+x3*y1+x1*y2-x3*y2-x1*y3+x2*y3);
}

int msh__amod(int x, int m)
{
  return (x % m + m) % m;
}

int msh_cutouts__is_convex(msh_cutouts_path_t *path, int idx)
{
  int prev_idx = msh__amod(idx-1, path->idx);
  int next_idx = msh__amod(idx+1, path->idx);
  float x1 = path->vertices[2*prev_idx];
  float y1 = path->vertices[2*prev_idx+1];
  float x2 = path->vertices[2*idx];
  float y2 = path->vertices[2*idx+1];
  float x3 = path->vertices[2*next_idx];
  float y3 = path->vertices[2*next_idx+1];
  return (msh_cutouts__signed_area(x1, y1, x2, y2, x3, y3) > 0.0f) ? 1 : 0;
}

int msh_cutouts__ear_test(msh_cutouts_path_t *path, int convex_idx, int reflex_idx)
{
  int prev_idx = msh__amod(convex_idx-1, path->idx);
  int next_idx = msh__amod(convex_idx+1, path->idx);
  if(prev_idx == reflex_idx || next_idx == reflex_idx) return 1;

  float cx1 = path->vertices[2*prev_idx];
  float cy1 = path->vertices[2*prev_idx+1];
  float cx2 = path->vertices[2*convex_idx];
  float cy2 = path->vertices[2*convex_idx+1];
  float cx3 = path->vertices[2*next_idx];
  float cy3 = path->vertices[2*next_idx+1]; 
  float rx  = path->vertices[2*reflex_idx]; 
  float ry  = path->vertices[2*reflex_idx+1];
  // printf("%d %d %d | %d\n", prev_idx, convex_idx, next_idx, reflex_idx);
  // printf(" %f %f  %f %f  %f %f\n", cx1, cy1, cx2, cy2, rx, ry);
  // printf(" %f %f  %f %f  %f %f\n", cx2, cy2, cx3, cy3, rx, ry);
  // printf(" %f %f  %f %f  %f %f\n", cx3, cy3, cx1, cy1, rx, ry);
  // printf("  %f %f %f\n", msh_cutouts__signed_area(cx1, cy1, cx2, cy2, rx, ry),
  //                        msh_cutouts__signed_area(cx2, cy2, cx3, cy3, rx, ry),
  //                        msh_cutouts__signed_area(cx3, cy3, cx1, cy1, rx, ry));
  if( msh_cutouts__signed_area(cx1, cy1, cy2, cy2, rx, ry) > 0.0f &&
      msh_cutouts__signed_area(cx2, cy2, cy3, cy3, rx, ry) > 0.0f &&
      msh_cutouts__signed_area(cx3, cy3, cy1, cy1, rx, ry) > 0.0f )
  {
    return 0;
  }
  return 1;
}

typedef enum msh_cutouts__vertex_type
{
  MSHC_UNKNOWN = 0,
  MSHC_REFLEX = 1,
  MSHC_CONVEX = 2,
  MSHC_EAR = 3,
  MSHC_CLIPPED = 4;
} msh_cutouts__vert_type;

int msh_cutouts__path_to_shape(msh_cutouts_path_t *path)
{
  // categorize vertices
  int n_verts = 1024;
  int vertex_type[1024] = {MSHC_UNKNOWN};

  for( int i = 0 ; i < path->idx; i++ )
  {
    if(!msh_cutouts__is_convex(path, i)) { vertex_type[i] = MSHC_REFLEX; }
    else                                 { vertex_type[i] = MSHC_CONVEX; }
  }

  // DEBUG: Print the list
  printf("Reflex verices: ");
  for(int i = 0; i < path->idx; ++i)
  {
    if(vertex_type[i] == MSHC_REFLEX)printf("%d ", i);
  }
  printf("\n");

  // Get initial ear list
  for(int i = 0; i < path->idx; ++i )
  {
    int is_ear = 1;
    if(vertex_type[i] == MSHC_REFLEX) continue; // only consider convex
    for(int j = 0; j < path->idx; ++j) // TODO(maciej): At each iteration create reflex list?
    {
      if(vertex_type[j] != MSHC_REFLEX) continue; // only consider reflex
      if(!msh_cutouts__ear_test(path, i, j))//TODO(maciej): Need a better name for this function
      {
        is_ear = 0;
      }
    }
    if( is_ear ) vertex_type[i] = MSHC_EAR;
    if( is_ear )
    {
      int prev_i = msh__amod(i-1, path->idx);
      int next_i = msh__amod(i+1, path->idx);
      printf("TRIANGLE: %d %d %d", prev_i, i, next_i);
      // Classify vertex
      
    }
  }

  printf("Ear verices: ");
  for(int i = 0; i < path->idx; ++i)
  {
    if(vertex_type[i] == MSHC_EAR)printf("%d ", i);
  }
  printf("\n");

  return 1;
}



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
  return ( cmd_a->paint_id - cmd_b->paint_id );
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
  ctx->cmd_buf_size = 0;
  ctx->cmd_buf = (msh_draw_cmd_t*)malloc( ctx->cmd_buf_capacity * sizeof(msh_draw_cmd_t) );

  // Setup paint buffer
  ctx->paint_buf_capacity = MSH_DRAW_INIT_CMD_BUF_SIZE;
  ctx->paint_buf_size = 0;
  ctx->paint_buf = (msh_draw_paint_t*)malloc( ctx->paint_buf_capacity * sizeof(msh_draw_paint_t) );

  // Setup image buffer
  ctx->image_buf_capacity = MSH_DRAW_INIT_CMD_BUF_SIZE;
  ctx->image_buf_size = 0;
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
    out vec2 v_auto_param;
    out vec2 v_pos;
    void main()
    {
      v_tcoord = vec2(tcoord.x, tcoord.y);
      v_pos = position.xy;
      // TODO(maciej): Once moved to elements draw, revisit autoparametrization
      v_auto_param = vec2( gl_VertexID % 3, gl_VertexID % 2 );
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
    uniform sampler2D font;

    in vec2 v_tcoord;
    in vec2 v_auto_param;
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
      else if ( type == 6 /*FONT*/)    { return t_coord; }
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

    vec4 polar_gradient(in vec4 c_a, in vec4 c_b, in vec2 t)
    {
      // TODO(maciej): Test different mixing factors
      float f = ((atan( t.x, t.y) * 0.15915494309189533576888376337251 /*OneOverTau*/) ) +0.5;
      return mix(c_a, c_b, f);
    }

    // TODO(maciej): SDF fonts?
    vec4 font_fill(in vec4 c_a, in vec4 c_b, in vec2 t)
    {
      vec4 c = mix( c_a, c_b, t.y);
      return vec4(vec3(c), texture(font, t).r);
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
      else if ( paint_type == 5 ) { frag_color=texture(tex, t); }
      else if ( paint_type == 6 ) { frag_color=font_fill(color_a, color_b, t); }
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

  // create vao and vbo/ebo
  glGenVertexArrays( 1, &(ogl->vao) );
  glGenBuffers(1, &ogl->vbo);
  glGenBuffers(1, &ogl->ebo);

  glBindVertexArray(ogl->vao);
  glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
  glBufferData(GL_ARRAY_BUFFER, MSH_DRAW_OGL_BUF_SIZE * sizeof(float), NULL, GL_STREAM_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl->ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, MSH_DRAW_OGL_BUF_SIZE * sizeof(int), NULL, GL_STREAM_DRAW);

  GLuint pos_attrib = glGetAttribLocation( ogl->prog_id, "position" );
  GLuint tcoord_attrib = glGetAttribLocation( ogl->prog_id, "tcoord" );
  glEnableVertexAttribArray(pos_attrib);
  glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 );
  glEnableVertexAttribArray(tcoord_attrib);
  glVertexAttribPointer(tcoord_attrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) );

  glBindVertexArray( 0 );
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  // glDisableVertexAttribArray( pos_attrib );
  glDisable(GL_CULL_FACE);
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
  ctx->paint_id = 0;
  ctx->paint_buf_size = 1;
  ctx->image_idx = 0;

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
  // qsort( ctx->cmd_buf, ctx->cmd_buf_size, sizeof(msh_draw_cmd_t), msh_draw__cmd_compare );
  
  // TODO(maciej): Custom meshes will probably be quite big. Maybe we need to allocate more for them.
  // Big meshes should probably not be uploaded every frame though...
  int data_idx = 0, elem_idx = 0;
  float data_buf[MSH_DRAW_OGL_BUF_SIZE];
  unsigned int elem_buf[MSH_DRAW_OGL_BUF_SIZE];

  // Gather commands and draw
  int n_draw_calls = 0;
  int i = 0;
  while( i < ctx->cmd_buf_size )
  {
    data_idx = 0;
    elem_idx = 0;
    int base_idx = 0;
    msh_draw_cmd_t* cur_cmd = &ctx->cmd_buf[i];
    int cur_paint_id = cur_cmd->paint_id;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while( cur_paint_id == cur_cmd->paint_id )
    {
      cur_paint_id = cur_cmd->paint_id;
      
      if( cur_cmd->type == MSHD_RECTANGLE )
      {
        printf("REctangle\n");
        // if( data_idx + 18 > MSH_DRAW_OGL_BUF_SIZE ) break;
        float x1 = cur_cmd->geometry[0];
        float y1 = cur_cmd->geometry[1];
        float x2 = cur_cmd->geometry[2];
        float y2 = cur_cmd->geometry[3];

        data_buf[data_idx++] = x1;
        data_buf[data_idx++] = y1;
        data_buf[data_idx++] = cur_cmd->z_idx;
        data_buf[data_idx++] = 0.0f;
        data_buf[data_idx++] = 0.0f;

        data_buf[data_idx++] = x1;
        data_buf[data_idx++] = y2;
        data_buf[data_idx++] = cur_cmd->z_idx;
        data_buf[data_idx++] = 0.0f;
        data_buf[data_idx++] = 1.0f;

        data_buf[data_idx++] = x2;
        data_buf[data_idx++] = y1;
        data_buf[data_idx++] = cur_cmd->z_idx;
        data_buf[data_idx++] = 1.0f;
        data_buf[data_idx++] = 0.0f;

        data_buf[data_idx++] = x2;
        data_buf[data_idx++] = y2;
        data_buf[data_idx++] = cur_cmd->z_idx;
        data_buf[data_idx++] = 1.0f;
        data_buf[data_idx++] = 1.0f;

        elem_buf[elem_idx++] = base_idx + 0;
        elem_buf[elem_idx++] = base_idx + 3;
        elem_buf[elem_idx++] = base_idx + 1;

        elem_buf[elem_idx++] = base_idx + 0;
        elem_buf[elem_idx++] = base_idx + 3;
        elem_buf[elem_idx++] = base_idx + 2;
        base_idx += 4;
      }
      else if( cur_cmd->type == MSHD_TEXT )
      {
        float x = cur_cmd->geometry[0];
        float y = cur_cmd->geometry[1];
        float s = 1.0;
        size_t len = strlen(cur_cmd->str);
        for(int i = 0; i < len; ++i)
        {
          msh_draw_aligned_quad_t q;
          stbtt_GetPackedQuad( ctx->char_info, MSH_FONT_RES, MSH_FONT_RES, 
                              cur_cmd->str[i], &x, &y, &q, 0);

          data_buf[data_idx++] = (q.x0 - cur_cmd->geometry[0]) * s + cur_cmd->geometry[0];
          data_buf[data_idx++] = (q.y0 - cur_cmd->geometry[1]) * s + cur_cmd->geometry[1];
          data_buf[data_idx++] = cur_cmd->z_idx;
          data_buf[data_idx++] = q.s0;
          data_buf[data_idx++] = q.t0;
          
          data_buf[data_idx++] = (q.x0 - cur_cmd->geometry[0]) * s + cur_cmd->geometry[0];
          data_buf[data_idx++] = (q.y1 - cur_cmd->geometry[1]) * s + cur_cmd->geometry[1];
          data_buf[data_idx++] = cur_cmd->z_idx;
          data_buf[data_idx++] = q.s0;
          data_buf[data_idx++] = q.t1;

          data_buf[data_idx++] = (q.x1 - cur_cmd->geometry[0]) * s + cur_cmd->geometry[0];
          data_buf[data_idx++] = (q.y0 - cur_cmd->geometry[1]) * s + cur_cmd->geometry[1];
          data_buf[data_idx++] = cur_cmd->z_idx;
          data_buf[data_idx++] = q.s1;
          data_buf[data_idx++] = q.t0;

          data_buf[data_idx++] = (q.x1 - cur_cmd->geometry[0]) * s + cur_cmd->geometry[0];
          data_buf[data_idx++] = (q.y1 - cur_cmd->geometry[1]) * s + cur_cmd->geometry[1];
          data_buf[data_idx++] = cur_cmd->z_idx;
          data_buf[data_idx++] = q.s1;
          data_buf[data_idx++] = q.t1;

          elem_buf[elem_idx++] = base_idx + i*4 + 0;
          elem_buf[elem_idx++] = base_idx + i*4 + 3;
          elem_buf[elem_idx++] = base_idx + i*4 + 1;
  
          elem_buf[elem_idx++] = base_idx + i*4 + 0;
          elem_buf[elem_idx++] = base_idx + i*4 + 3;
          elem_buf[elem_idx++] = base_idx + i*4 + 2;
        }
        base_idx += 4 * (int)len;
      }
      else if (cur_cmd->type == MSHD_ARC)
      {
        float x_pos  = cur_cmd->geometry[0];
        float y_pos  = cur_cmd->geometry[1];
        float rad    = cur_cmd->geometry[2];
        float fraction = cur_cmd->geometry[3];
        
        int res = (int)(0.5f * rad + 8.0f);
        float delta = 2.0f*(float)MSH_PI / res;
        
        data_buf[data_idx++] = x_pos;
        data_buf[data_idx++] = y_pos;
        data_buf[data_idx++] = cur_cmd->z_idx;
        data_buf[data_idx++] = 0.5;
        data_buf[data_idx++] = 0.5;

        // NOTE(maciej): This is still wasteful.
        int n_verts = 1;
        for( int step = 0; step < fraction * res; step++ )
        {
          float theta = step * delta;
          float omega = (step + 1) * delta;
          float sint = sinf(theta);
          float cost = cosf(theta);
          float sino = sinf(omega);
          float coso = cosf(omega);
  
          data_buf[data_idx++] = x_pos + sint * rad;
          data_buf[data_idx++] = y_pos - cost * rad;
          data_buf[data_idx++] = cur_cmd->z_idx;
          data_buf[data_idx++] = 0.5f * (sint + 1.0f);
          data_buf[data_idx++] = 0.5f * (-cost + 1.0f);
  
          data_buf[data_idx++] = x_pos + sino * rad;
          data_buf[data_idx++] = y_pos - coso * rad;
          data_buf[data_idx++] = cur_cmd->z_idx;
          data_buf[data_idx++] = 0.5f * (sino + 1.0f);
          data_buf[data_idx++] = 0.5f * (-coso + 1.0f);
          
          elem_buf[elem_idx++] = base_idx;
          elem_buf[elem_idx++] = base_idx + 2*(step+1) - 1 ;
          elem_buf[elem_idx++] = base_idx + 2*(step+1) + 0;
          n_verts += 2;
        }
        base_idx += n_verts;
      }
      else if (cur_cmd->type == MSHD_LINE_START)
      {
        msh_draw_cmd_t* line_cmd_a = &ctx->cmd_buf[i];
        msh_draw_cmd_t* line_cmd_b = &ctx->cmd_buf[i+1];
        // TODO(maciej): HOW to even draw lines correctly? Line adjacency seems relevant.
        int n_verts = 0;
        int idx = 0;
        while( line_cmd_a->type != MSHD_LINE_END )
        {
          float x0 = line_cmd_a->geometry[0];
          float x1 = line_cmd_b->geometry[0];
          
          float y0 = line_cmd_a->geometry[1];
          float y1 = line_cmd_b->geometry[1];
          
          float v0 = x1 - x0;
          float v1 = y1 - y0;
          float n0 = 12 * (v1 / sqrtf(v1*v1 + v0*v0));
          float n1 = 12 * -(v0 / sqrtf(v1*v1 + v0*v0));
// printf("%f %f | %f |$|  %f %f | %f\n", v0, v1, sqrt(v0*v0 + v1*v1), n0, n1, sqrtf(n0*n0 + n1*n1));
          data_buf[data_idx++] = x0 - n0;
          data_buf[data_idx++] = y0 - n1;
          data_buf[data_idx++] = line_cmd_a->z_idx;
          data_buf[data_idx++] = 0.0f;
          data_buf[data_idx++] = 0.0f;

          data_buf[data_idx++] = x0 + n0;
          data_buf[data_idx++] = y0 + n1;
          data_buf[data_idx++] = line_cmd_a->z_idx;
          data_buf[data_idx++] = 0.0f;
          data_buf[data_idx++] = 1.0f;


          data_buf[data_idx++] = x1 - n0;
          data_buf[data_idx++] = y1 - n1;
          data_buf[data_idx++] = line_cmd_a->z_idx;
          data_buf[data_idx++] = 1.0f;
          data_buf[data_idx++] = 0.0f;


          data_buf[data_idx++] = x1 + n0;
          data_buf[data_idx++] = y1 + n1;
          data_buf[data_idx++] = line_cmd_a->z_idx;
          data_buf[data_idx++] = 1.0f;
          data_buf[data_idx++] = 1.0f;

          elem_buf[elem_idx++] = base_idx + 4 * idx + 0;
          elem_buf[elem_idx++] = base_idx + 4 * idx + 1;
          elem_buf[elem_idx++] = base_idx + 4 * idx + 3;

          elem_buf[elem_idx++] = base_idx + 4 * idx + 3;
          elem_buf[elem_idx++] = base_idx + 4 * idx + 2;
          elem_buf[elem_idx++] = base_idx + 4 * idx + 0;

          i++;
          idx++;
          n_verts += 4;
          line_cmd_a = &ctx->cmd_buf[i];
          line_cmd_b = &ctx->cmd_buf[i+1];
        }
        printf("Data_idx = %d\n", data_idx );
        base_idx += n_verts;
      }
      cur_cmd = &ctx->cmd_buf[++i];
      if( i >= ctx->cmd_buf_size ) break;
    }

#ifdef MSH_DRAW_OPENGL
    msh_draw_opengl_backend_t* ogl = &ctx->backend;
    msh_draw_paint_t* paint = &ctx->paint_buf[cur_paint_id];
    msh_draw_color_t c_a = paint->fill_color_a;
    msh_draw_color_t c_b = paint->fill_color_b;
    
    glUseProgram(ogl->prog_id);
    if( paint->type == MSHD_IMAGE ) 
    {
      glActiveTexture(GL_TEXTURE0 + 1);
      glBindTexture( GL_TEXTURE_2D, paint->image_idx);
    }
    if( paint->type == MSHD_FONT )
    {
      glActiveTexture(GL_TEXTURE0 + 2);
      glBindTexture( GL_TEXTURE_2D, paint->image_idx);
      // printf("PAINT: %d\n", cur_paint_id );
    }
    
    //TODO(maciej): Check different ways for setting up an uniform
    GLuint location = glGetUniformLocation( ogl->prog_id, "color_a" );
    glUniform4f( location, c_a.r, c_a.g, c_a.b, c_a.a );
    location = glGetUniformLocation( ogl->prog_id, "color_b" );
    glUniform4f( location, c_b.r, c_b.g, c_b.b, c_b.a );
    location = glGetUniformLocation(ogl->prog_id, "gradient_params" );
    glUniform4f( location, paint->offset_x, paint->offset_y, paint->feather, paint->radius );
    location = glGetUniformLocation( ogl->prog_id, "paint_type" );
    glUniform1i( location, (int)paint->type );
    location = glGetUniformLocation( ogl->prog_id, "tex" );
    glUniform1i( location, (int)1 ); //NOTE(maciej): It's the unit!
    location = glGetUniformLocation( ogl->prog_id, "font" );
    glUniform1i( location, (int)2 ); //NOTE(maciej): It's the unit!
    location = glGetUniformLocation( ogl->prog_id, "viewport_res" );
    glUniform2f( location, (float)ctx->viewport_width, (float)ctx->viewport_height );

    // NOTE(maciej): We will probably be drawing in chunks, so no need for this resize
    // glBindVertexArray(ogl->vao);
    // glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
    // glBufferSubData(GL_ARRAY_BUFFER, 0, data_idx*sizeof(float), data_buf );
    // glDrawArrays(GL_TRIANGLES, 0, data_idx / 5);
    
    glBindVertexArray(ogl->vao);
    glBindBuffer(GL_ARRAY_BUFFER, ogl->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, data_idx*sizeof(float), data_buf );
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ogl->ebo);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, elem_idx*sizeof(unsigned), elem_buf );
    glDrawElements(GL_TRIANGLES, elem_idx, GL_UNSIGNED_INT, 0);
    

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
  int out_id = -1;
  for( int i = 0 ; i < ctx->paint_buf_size; i++ )
  {
    msh_draw_paint_t cur_paint = ctx->paint_buf[i];
    if( cur_paint.id == paint->id)
    {
      out_id = i;
    }
  }
  return out_id;
}

const int msh_draw__add_paint( msh_draw_ctx_t* ctx, msh_draw_paint_t* p )
{
  //NOTE(maciej): For now we just push. Should we add some free list thing?
  msh_draw__resize_paint_buf( ctx, 1 );
  p->id         = ctx->paint_buf_size; //NOTE(maciej): Why do we need this?
  ctx->paint_id = p->id; 
  ctx->paint_buf[ p->id ] = *p;
  ctx->paint_buf_size = ctx->paint_buf_size + 1;
  
  return ctx->paint_id;
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
  glActiveTexture(GL_TEXTURE0 + 2);//NOTE(maciej): What does nanovg do with texture unit activation?
  glBindTexture(GL_TEXTURE_2D, tex.id);
  // TODO(maciej): Use flags to resolve this
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  

  GLuint internal_format = 0;
  GLuint format = 0;
  switch( n )
  {
    case 1:
      format = GL_RED;
      internal_format = GL_R8;
      break;
    case 2:
      format = GL_RG;
      internal_format = GL_RG;
      break;
    case 3:
      format = GL_RGB;
      internal_format = GL_RGB8;
      break;
    case 4:
      format = GL_RGBA;
      internal_format = GL_RGBA8;
      break;
    default:
      internal_format = GL_RGBA;
      format = GL_RGB;
  }
  glTexImage2D( GL_TEXTURE_2D, 0, internal_format, 
                w, h, 0, format, GL_UNSIGNED_BYTE, data );
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
msh_draw_set_paint( msh_draw_ctx_t* ctx, const int paint_id )
{
  ctx->paint_id = paint_id;
}


////////////////////////////////////////////////////////////////////////////////
// Font
////////////////////////////////////////////////////////////////////////////////

// TODO(maciej): Error checks
int msh_draw_add_font( msh_draw_ctx_t* ctx, const char* filename, const float font_size )
{
  unsigned char ttf_buffer[MSH_FONT_MAX_FILE_SIZE];
  unsigned char* bitmap = (unsigned char*) malloc(MSH_FONT_RES*MSH_FONT_RES);
  FILE* font_file = fopen(filename, "rb");
  fread(ttf_buffer, 1, MSH_FONT_MAX_FILE_SIZE, font_file);
  fclose(font_file);

  stbtt_pack_context pack_context;
  stbtt_PackBegin( &pack_context, bitmap, MSH_FONT_RES, MSH_FONT_RES, 0, 1, NULL);
  stbtt_PackSetOversampling(&pack_context, 1, 1);

  // NOTE(maciej): Apparently better to use sparse codepoints instead of range
  // NOTE(maciej): Also for different font sizes we need different char_info
  int res = stbtt_PackFontRange(&pack_context, ttf_buffer, 0, font_size, 0, MSH_FONT_MAX_CHARS, ctx->char_info);
  stbtt_PackEnd(&pack_context);

  msh_draw_image_t font_tex;
  font_tex.width      = MSH_FONT_RES;
  font_tex.height     = MSH_FONT_RES;
  font_tex.n_channels = 1;

  // TODO(maciej): Add separate buffer for fonts?
  msh_draw__resize_image_buf( ctx, 1 );
  ctx->image_idx = ctx->image_buf_size; 
  ctx->image_buf[ ctx->image_idx ] = font_tex;
  ctx->image_buf_size = ctx->image_buf_size + 1;

  glGenTextures(1, &font_tex.id);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, font_tex.id);

  glPixelStorei(GL_UNPACK_ALIGNMENT,1);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, MSH_FONT_RES, MSH_FONT_RES, 0, 
                GL_RED, GL_UNSIGNED_BYTE, bitmap );
  glBindTexture( GL_TEXTURE_2D, 0 );

  msh_draw_color_t c_a = {.r=0.0f, .g=0.0f, .b=0.0f, .a=0.0f};
  msh_draw_color_t c_b = {.r=0.0f, .g=0.0f, .b=0.0f, .a=0.0f};
  msh_draw_paint_t p = (msh_draw_paint_t){.type=MSHD_FONT,
                                          .fill_color_a = c_a, .fill_color_b = c_b,
                                          .image_idx = font_tex.id };

  int font_paint_id = msh_draw__add_paint( ctx, &p );

  free( bitmap );
  return font_paint_id;
}

////////////////////////////////////////////////////////////////////////////////
// DRAW PRIMITIVES
////////////////////////////////////////////////////////////////////////////////

void 
msh_draw_arc( msh_draw_ctx_t* ctx, float x, float y, float r, float fraction )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_ARC;
  cmd.paint_id = ctx->paint_id;
  cmd.geometry[0] = x;
  cmd.geometry[1] = y;
  cmd.geometry[2] = r;
  cmd.geometry[3] = fraction;
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
  msh_draw_arc(ctx, x, y, r, 1.0f );
}

void 
msh_draw_line_start( msh_draw_ctx_t* ctx, float x, float y )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_LINE_START;
  cmd.paint_id = ctx->paint_id;
  cmd.geometry[0] = x;
  cmd.geometry[1] = y;
  cmd.z_idx = ctx->z_idx;
  
  // Modify the context
  ctx->cmd_idx = ctx->cmd_buf_size;  
  ctx->cmd_buf[ctx->cmd_idx] = cmd;
  ctx->cmd_buf_size += 1;
}


void 
msh_draw_line_to( msh_draw_ctx_t* ctx, float x, float y )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_LINE_TO;
  cmd.paint_id = ctx->paint_id;
  cmd.geometry[0] = x;
  cmd.geometry[1] = y;
  cmd.z_idx = ctx->z_idx;
  
  // Modify the context
  ctx->cmd_idx = ctx->cmd_buf_size;  
  ctx->cmd_buf[ctx->cmd_idx] = cmd;
  ctx->cmd_buf_size += 1;
}

void 
msh_draw_line_end( msh_draw_ctx_t* ctx, float x, float y )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_LINE_END;
  cmd.paint_id = ctx->paint_id;
  cmd.geometry[0] = x;
  cmd.geometry[1] = y;
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
  cmd.paint_id = ctx->paint_id;
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


void 
msh_draw_text( msh_draw_ctx_t* ctx, float x1, float y1, const char* str, int paint_id )
{
  msh_draw__resize_cmd_buf( ctx, 1 );
  
  // Populate command 
  msh_draw_cmd_t cmd;
  cmd.type = MSHD_TEXT;
  cmd.paint_id = paint_id;
  cmd.geometry[0] = x1;
  cmd.geometry[1] = y1;
  cmd.str = str;
  cmd.z_idx = ctx->z_idx;
  
  // Modify the context
  ctx->z_idx += 0.001f;
  ctx->cmd_idx = ctx->cmd_buf_size;  
  ctx->cmd_buf[ctx->cmd_idx] = cmd;
  ctx->cmd_buf_size += 1;
}

#endif /*MSH_DRAW_IMPLEMENTATION */
