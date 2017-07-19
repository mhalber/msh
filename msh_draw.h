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
  [ ] OpenGL backend
  [ ] Software Rasterizer backend
  ==============================================================================
  REFERENCES:

 */

#ifndef MSH_DRAW_H
#define MSH_DRAW_H

typedef struct msh_draw_ctx
{
  // shader for binding?
} msh_draw_ctx_t;

int msh_init_ctx( msh_draw_ctx* ctx );
void msh_draw_circle( msh_draw_ctx_t* ctx, float x, float y, float radius );
void msh_draw_line( msh_draw_ctx_t* ctx, float x1, float y1, float x2, float y2 );

#endif /*MSH_DRAW_H*/

#ifdef MSH_DRAW_IMPLEMENTATION

int 
msh_init_ctx( msh_draw_ctx* ctx )
{
  // create shader
  char* vert_shdr_src = "";
  char* frag_shdr_src = ""; 
  GLuint vert_shdr_id, frag_shdr_id;

  // compile them etc.
  vert_shdr_id = glCreateShader( GL_VERTEX_SHADER );
  frag_shdr_id = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( vert_shdr_id, 1, &vert_shdr_src, NULL );
  glShaderSource( frag_shdr_id, 1, &vert_shdr_src, NULL );
  // link
  // create vao?

}

#endif /*MSH_DRAW_IMPLEMENTATION */