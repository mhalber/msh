#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <time.h>
#include <float.h>
#include <math.h>

#define MSH_VEC_MATH_IMPLEMENTATION
#include "msh_vec_math.h"

#define MSH_GFX_IMPLEMENTATION
#define MAX_WINDOWS 10
#include "msh_gfx.h"

char *vs_source = (char *)MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY(
    layout(location = 0) in vec3 position;
    layout(location = 4) in vec4 color;
    out vec4 v_color;
    void main() {
      gl_Position = vec4(position, 1.0);
      v_color = color;
    });

char *fs_source = (char *)MSHGFX_SHADER_HEAD MSHGFX_SHADER_STRINGIFY(
    in vec4 v_color;
    out vec4 frag_color;
    void main() {
      frag_color = v_color;
    });

msh_viewport_t viewports[3];
mshgfx_shader_prog_t triangle_shader;
mshgfx_geometry_t triangle_geo;
mshgfx_texture2d_t fb_texture;
mshgfx_framebuffer_t fb;

int window_display(void)
{
  mshgfx_framebuffer_bind(&fb);

  msh_viewport_begin(&viewports[1]);
  mshgfx_background_gradient4fv(msh_vec4(1.0f, 0.0f, 0.0f, 1.0f),
                                msh_vec4(0.0f, 1.0f, 1.0f, 1.0f));
  mshgfx_shader_prog_use(&triangle_shader);
  mshgfx_geometry_draw(&triangle_geo, GL_TRIANGLES);
  msh_viewport_end();

  msh_viewport_begin(&viewports[0]);
  mshgfx_background_flat4f(0.5f, 0.2f, 0.8f, 1.0f);
  mshgfx_shader_prog_use(&triangle_shader);
  mshgfx_geometry_draw(&triangle_geo, GL_TRIANGLES);
  msh_viewport_end();

  mshgfx_framebuffer_bind(NULL);
  msh_viewport_begin(&viewports[2]);
  mshgfx_background_tex(&(fb_texture));
  msh_viewport_end();

  return 1;
}

void window_refresh(mshgfx_window_t *window)
{
  int w, h, fb_w, fb_h;
  glfwGetWindowSize(window, &w, &h);            //mshgfx_window_size( window, fb_w, fb_h );
  glfwGetFramebufferSize(window, &fb_w, &fb_h); //mshgfx_window_framebuffer_size( window, fb_w, fb_h );

  float pix_ratio = w / (float)fb_w;

  viewports[0].p1 = msh_vec2(0.0f, 0.0f);
  viewports[0].p2 = msh_vec2(pix_ratio * fb_w / 2.0f, pix_ratio * fb_h);

  viewports[1].p1 = msh_vec2(pix_ratio * fb_w / 2.0f, 0.0f);
  viewports[1].p2 = msh_vec2(pix_ratio * fb_w / 2.0f, pix_ratio * fb_h);

  viewports[2].p1 = viewports[0].p1;
  viewports[2].p2 = msh_vec2(pix_ratio * fb_w, pix_ratio * fb_h);

  mshgfx_texture_free(&fb_texture); // TODO(maciej): Texture and renderbuffer resize?
  mshgfx_texture_init_u8(&fb_texture, NULL, fb_w, fb_h, 3, 0);

  mshgfx_framebuffer_resize(&fb, fb_w, fb_h);
  GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
  mshgfx_framebuffer_attach_color_texture(&fb, &fb_texture, &attachments[0], 1);
  mshgfx_framebuffer_add_depth_renderbuffer(&fb);
  mshgfx_framebuffer_check_status(&fb);

  mshgfx_window_display(window, window_display);
}

int main(void)
{
  /* setup the functions */
  mshgfx_window_t *windows[MAX_WINDOWS];
  int window_width = 640;
  int window_height = 480;
  int (*display_functions[MAX_WINDOWS])(void);
  windows[0] = mshgfx_window_create("Test1", -1, -1,
                                    window_width, window_height);
  display_functions[0] = window_display;
  int n_windows = 1;

  /* ogl initialization */
  mshgfx_window_activate(windows[0]);
  mshgfx_window_set_callback_refresh(windows[0], window_refresh);

  mshgfx_shader_prog_create_from_source_vf(&triangle_shader,
                                           vs_source,
                                           fs_source);
  msh_viewport_init(&viewports[0], (msh_vec2_t){{0, 0}},
                    (msh_vec2_t){{window_width / 2, window_height / 2}});
  msh_viewport_init(&viewports[1], (msh_vec2_t){{window_width / 2, 0}},
                    (msh_vec2_t){{window_width / 2, window_height / 2}});
  msh_viewport_init(&viewports[2], (msh_vec2_t){{0, 0}},
                    (msh_vec2_t){{window_width, window_height}});

  mshgfx_framebuffer_init(&fb, window_width, window_height);

  // The color texture we're going to render to
  mshgfx_texture_init_u8(&fb_texture, NULL, window_width, window_height, 3, 0);
  GLuint attachments[1] = {GL_COLOR_ATTACHMENT0};
  mshgfx_framebuffer_attach_color_texture(&fb, &fb_texture, &attachments[0], 1);

  // The renderbuffer for depth
  mshgfx_framebuffer_add_depth_renderbuffer(&fb);
  mshgfx_framebuffer_check_status(&fb);

  float positions[9] = {-0.8f, -0.8f, 0.0f,
                        0.8f, -0.8f, 0.0f,
                        0.0f, 0.8f, 0.0f};
  unsigned char colors[12] = {255, 0, 0, 255,
                              0, 255, 0, 255,
                              0, 0, 255, 255};

  mshgfx_geometry_data_t triangle;
  triangle.positions = &(positions[0]);
  triangle.colors_a = &(colors[0]);
  triangle.n_vertices = 3;

  mshgfx_geometry_init(&triangle_geo, &triangle, POSITION | COLOR_A);

  /* iterate over the windows */
  while (mshgfx_window_is_any_open(windows, n_windows))
  {
    for (int i = 0; i < n_windows; ++i)
    {
      if (windows[i])
      {
        mshgfx_window_display(windows[i], display_functions[i]);
      }
    }
    mshgfx_window_poll_events();
  }

  mshgfx_window_terminate();

  return 0;
}
