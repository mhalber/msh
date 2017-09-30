/*
  ==============================================================================
  
  MSH_IMG_PROC.H - WIP!
  
  A single header library for simple machine learning algorithms. 

  To use the library you simply add:
  
  #define MSH_IMG_PROC_IMPLEMENTATION
  #include "msh_img_proc.h"

  The define should only include once in your source. If you need to include 
  library in multiple places, simply use the include:

  #include "msh_img_proc.h"

  ==============================================================================
  DEPENDENCIES

  This library requires anonymous structs, which is a C11 extension.

  This library depends on following standard headers:
    // ADD HEADERS
  
  The main purpose of this library is image processing, hence we do not provide
  loaders. There are implementation of loaders that depend on stb_image.h, 
  stb_image_write.h and lodepng.h libraries, but these are disabled by
  default. To enable them simply define 
  #define MSH_IMG_PROC_IO
  ==============================================================================
  AUTHORS

    Maciej Halber (macikuh@gmail.com)

  ==============================================================================
  LICENSE
  The MIT License (MIT)

  Copyright (c) 2017 Maciej Halber

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
  
  
  ==============================================================================
  NOTES 

  ==============================================================================
  TODOs

  ==============================================================================
  REFERENCES:

 */


#ifndef MSH_IMG_PROC_H
#define MSH_IMG_PROC_H

#ifdef __cplusplus
extern "C" {
#endif

// #ifdef MSH_IMG_PROC_INCLUDE_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <float.h>
// #include <math.h>
// #include <stdbool.h>
// #endif

#ifdef MSH_IMG_PROC_STATIC
#define MSHIPDEF static
#else
#define MSHIPDEF extern
#endif

#if !defined(__cplusplus)
  #if defined(_MSC_VER) 
    #define inline __inline
  #else
    #define inline
  #endif
#endif


typedef unsigned char  mship_ui8;
typedef unsigned short mship_ui16;
typedef unsigned int   mship_ui32;
typedef char           mship_i8;
typedef short          mship_i16;
typedef int            mship_i32;
typedef float          mship_f32;
typedef double         mship_f64;

#define MSH_PIXEL_STRUCT_DEF(id,b,s)\
typedef struct msh_pixel##s##_##id##b\
{\
  mship_##id##b data[##s##];\
}msh_pixel##s##_##id##b##_t;\

#define MSH_IMAGE_STRUCT_DEF(id, b) \
typedef struct msh_img##id##b\
{\
 int width;\
 int height;\
 int n_comp;\
 mship_##id##b *data;\
}msh_img_##id##b##_t;

#define MSH_IMAGE_INIT_DECL(id, b)\
msh_img_##id##b##_t mship_img_##id##b##_init(int w, int h, int n, int init);

#define MSH_IMAGE_FREE_DECL(id, b)\
msh_img_##id##b##_t mship_img_##id##b##_free(msh_img_##id##b##_t *img);

#define MSH_IMAGE_SAMPLE_NN_DECL(id, b)\
inline mship_##id##b \
mship_sample_nn_##id##b##(msh_img_##id##b##_t *img, float x, float y, int c);

#define MSH_IMAGE_SAMPLE_NN_PIX_DECL(id, b, s)\
inline msh_pixel##s##_##id##b##_t \
mship_sample##s##_nn_##id##b##(msh_img_##id##b##_t *img, float x, float y);

#define MSH_IMAGE_SAMPLE_BILINEAR_DECL(id, b)\
inline mship_##id##b mship_sample_bl_##id##b##(msh_img_##id##b##_t *img, float x, float y, int c);

#define MSH_IMAGE_PIXEL_PTR_DECL(id,b) \
inline mship_##id##b* mship_pixel_ptr_##id##b##(msh_img_##id##b##_t *img, int x, int y);

// Code gen
MSH_PIXEL_STRUCT_DEF(ui, 8, 2)
MSH_PIXEL_STRUCT_DEF(ui, 8, 3)
MSH_PIXEL_STRUCT_DEF(ui, 8, 4)
MSH_PIXEL_STRUCT_DEF(ui, 16, 2)
MSH_PIXEL_STRUCT_DEF(ui, 16, 3)
MSH_PIXEL_STRUCT_DEF(ui, 16, 4)
MSH_PIXEL_STRUCT_DEF(ui, 32, 2)
MSH_PIXEL_STRUCT_DEF(ui, 32, 3)
MSH_PIXEL_STRUCT_DEF(ui, 32, 4)
MSH_PIXEL_STRUCT_DEF(f, 32, 2)
MSH_PIXEL_STRUCT_DEF(f, 32, 3)
MSH_PIXEL_STRUCT_DEF(f, 32, 4)
MSH_PIXEL_STRUCT_DEF(f, 64, 2)
MSH_PIXEL_STRUCT_DEF(f, 64, 3)
MSH_PIXEL_STRUCT_DEF(f, 64, 4)

MSH_IMAGE_STRUCT_DEF(ui, 8)
MSH_IMAGE_STRUCT_DEF(ui, 16)
MSH_IMAGE_STRUCT_DEF(ui, 32)
MSH_IMAGE_STRUCT_DEF(f, 32)
MSH_IMAGE_STRUCT_DEF(f, 64)

MSH_IMAGE_INIT_DECL(ui, 8)
MSH_IMAGE_INIT_DECL(ui, 16)
MSH_IMAGE_INIT_DECL(ui, 32)
MSH_IMAGE_INIT_DECL(f, 32)
MSH_IMAGE_INIT_DECL(f, 64)

MSH_IMAGE_SAMPLE_NN_DECL(ui, 8)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 8, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 8, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 8, 4)
MSH_IMAGE_SAMPLE_NN_DECL(ui, 16)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 16, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 16, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 16, 4)
MSH_IMAGE_SAMPLE_NN_DECL(ui, 32)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 32, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 32, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(ui, 32, 4)
MSH_IMAGE_SAMPLE_NN_DECL(f, 32)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(f, 32, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(f, 32, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(f, 32, 4)
MSH_IMAGE_SAMPLE_NN_DECL(f, 64)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(f, 64, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(f, 64, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DECL(f, 64, 4)

MSH_IMAGE_SAMPLE_BILINEAR_DECL(ui, 8)
MSH_IMAGE_SAMPLE_BILINEAR_DECL(ui, 16)
MSH_IMAGE_SAMPLE_BILINEAR_DECL(ui, 32)
MSH_IMAGE_SAMPLE_BILINEAR_DECL(f, 32)
MSH_IMAGE_SAMPLE_BILINEAR_DECL(f, 64)

MSH_IMAGE_PIXEL_PTR_DECL(ui, 8)
MSH_IMAGE_PIXEL_PTR_DECL(ui, 16)
MSH_IMAGE_PIXEL_PTR_DECL(ui, 32)
MSH_IMAGE_PIXEL_PTR_DECL(f, 32)
MSH_IMAGE_PIXEL_PTR_DECL(f, 64)

#ifdef MSH_IMG_PROC_IO
msh_imgui8 msh_load_png();
#endif



#ifdef __cplusplus
}
#endif

#endif /*MSH_IMG_PROC_H*/

#ifdef MSH_IMG_PROC_IMPLEMENTATION

#define MSH_IMAGE_INIT_DEF(id, b)\
MSHIPDEF msh_img_##id##b##_t \
mship_img_##id##b##_init(int width, int height, int n_comp, int initialize)\
{\
  msh_img_##id##b##_t img;\
  int n_elems = width*height*n_comp;\
  int byte_size = n_elems*sizeof(mship_ui8);\
  img.width  = width;\
  img.height = height;\
  img.n_comp = n_comp;\
  img.data   = (mship_##id##b*)malloc(byte_size);\
  if(initialize) { memset((void*)img.data, 0, byte_size); }\
  return img;\
}\

#define MSH_IMAGE_FREE_DEF(id, b)\
MSHIPDEF void \
mship_img_##id##b##_free(msh_img_##id##b##_t* img)\
{\
  free(img->data);\
}\

#define MSH_IMAGE_PIXEL_PTR_DEF(id,b,s) \
MSHIPDEF inline mship_##id##b* \
mship_pixel_ptr_##id##b(msh_img##id##b_t *img, int x, int y)\
{\
  return &(img->data[img->n_comp*(img->width*y + x)])\
}\

#define MSH_IMAGE_SAMPLE_NN_DEF(id, b)\
MSHIPDEF inline mship_##id##b \
mship_sample_nn_##id##b(msh_img##id##b##_t *img, int x, int y, int c)\
{\
  return (img->data[img->n_comp*(img->width*y + x)+c])\
}\ 

#define MSH_IMAGE_SAMPLE_NN_PIX_DEF(id, b, s)\
MSHIPDEF inline msh_pixel##s##_##id##b \
mship_sample##s##_nn_##id##b(msh_img##id##b##_t *img, int x, int y)\
{\
  msh_pixel##s##_##id##b pix;\
  memcpy(img->data[img->n_comp*(img->width*y + x)], &pix.data[0], (s##*##b)/sizeof(char));\
  return pix;\
}\ 

// NOTE(maciej): There might be more efficient version of this that writes to a pixel
#define MSH_IMAGE_SAMPLE_BILINEAR_DEF(id, b)\
MSHIPDEF inline mship_##id##b \
mship_sample_bl_##id##b(msh_img##id##b##_t *img, float x, float y, int c)\
{\
  int lx = floorf(x); int rx = min(lx+1.0, img->width-1); \
  int ly = floorf(y); int ry = min(ly+1.0, img->height-1); \
  float val1 = mship_sample_nn_##id##b(img_ptr, lx, ly, c) * (1.0f+lx-x)*(1.0f+ly-y);\
  float val2 = mship_sample_nn_##id##b(img_ptr, lx, ry, c) * (1.0f+lx-x)*(y-ly);\
  float val3 = mship_sample_nn_##id##b(img_ptr, rx, ly, c) * (x-lx)*(1.0f+ly-y);\
  float val4 = mship_sample_nn_##id##b(img_ptr, rx, ry, c) * (x-lx)*(y-ly);\
  return (val1+val2+val3+val3)\
}

#define MSH_IMAGE__PIXEL2_SCALAR_MUL(id, b)\
static inline \
msh_pixel2_##id##b mship__pixel2_##id##b##_mul_scalar(msh_pixel2_##id##b pix, float s)\
{\
  return (msh_pixel2_##id##b){ pix.data[0]*s, pix.data[1]*s};\
}

#define MSH_IMAGE__PIXEL3_SCALAR_MUL(id, b)\
static inline \
msh_pixel3_##id##b mship__pixel3_##id##b##_mul_scalar(msh_pixel3_##id##b pix, float s)\
{\
  return (msh_pixel3_##id##b){pix.data[0]*s, pix.data[1]*s, pix.data[2]*s};\
}

#define MSH_IMAGE__PIXEL4_SCALAR_MUL(id, b)\
static inline \
msh_pixel4_##id##b mship__pixel4_##id##b##_mul_scalar(msh_pixel4_##id##b pix, float s)\
{\
  return (msh_pixel4_##id##b){pix.data[0]*s, pix.data[1]*s, pix.data[2]*s, pix.data[3]*s};\
}

#define MSH_IMAGE__PIXEL2_ADD(id, b)\
static inline \
msh_pixel2_##id##b mship__pixel2_##id##b##_add(msh_pixel2_##id##b pix_a, msh_pixel2_##id##b pix_b)\
{\
  return (msh_pixel2_##id##b){pix_a.data[0]+pix_b.data[0], pix_a.data[1]+pix_b.data[1]};\
}

#define MSH_IMAGE__PIXEL3_ADD(id, b)\
static inline \
msh_pixel3_##id##b mship__pixel3_##id##b##_mul_scalar(msh_pixel3_##id##b pix, float s)\
{\
  return (msh_pixel3_##id##b){\
    pix_a.data[0]+pix_b.data[0], pix_a.data[1]+pix_b.data[1], pix_a.data[2]+pix_b.data[2]};\
}

#define MSH_IMAGE__PIXEL4_ADD(id, b)\
static inline \
msh_pixel4_##id##b mship__pixel4_##id##b##_mul_scalar(msh_pixel4_##id##b pix, float s)\
{\
  return (msh_pixel4_##id##b){\
    pix_a.data[0]+pix_b.data[0], pix_a.data[1]+pix_b.data[1],\
    pix_a.data[2]+pix_b.data[2], pix_a.data[3]+pix_b.data[3]};\
}


// NOTE(maciej): Decide whether we want to have pixel with one elem
#define MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(id, b, s)\
MSHIPDEF inline mship_##id##b \
mship_sample##s##_bl_##id##b(msh_img##id##b##_t *img, float x, float y)\
{\
  int lx = floorf(x); int rx = min(lx+1.0, img->width-1); \
  int ly = floorf(y); int ry = min(ly+1.0, img->height-1); \
  float w1 = (1.0f+lx-x)*(1.0f+ly-y);\
  float w2 = (1.0f+lx-x)*(y-ly);\
  float w3 = (x-lx)*(1.0f+ly-y);\
  float w4 = (x-lx)*(y-ly);\
  msh_pixel##s##_##id##b pix1 = mship_sample##s##_nn_##id##b(img_ptr, lx, ry);\
  msh_pixel##s##_##id##b pix2 = mship_sample##s##_nn_##id##b(img_ptr, lx, ry);\
  msh_pixel##s##_##id##b pix3 = mship_sample##s##_nn_##id##b(img_ptr, rx, ly);\
  msh_pixel##s##_##id##b pix4 = mship_sample##s##_nn_##id##b(img_ptr, rx, ry);\
  pix1 = mship__pixel##s##_##id##b##_mul_scalar(pix1, w1);\
  pix2 = mship__pixel##s##_##id##b##_mul_scalar(pix1, w1);\
  pix2 = mship__pixel##s##_##id##b##_mul_scalar(pix1, w1);\
  pix2 = mship__pixel##s##_##id##b##_mul_scalar(pix1, w1);\
  msh_pixel##s##_##id##b pixa = mship__pixel##s##_##id##b##_add(pix1, pix2);\
  msh_pixel##s##_##id##b pixb = mship__pixel##s##_##id##b##_add(pix3, pix4);\
  return (mship__pixel##s##_##id##b##_add(pix_a, pix_b);\
}

MSH_IMAGE_INIT_DEF(ui, 8)
MSH_IMAGE_INIT_DEF(ui, 16)
MSH_IMAGE_INIT_DEF(ui, 32)
MSH_IMAGE_INIT_DEF(f, 32)
MSH_IMAGE_INIT_DEF(f, 64)

MSH_IMAGE_FREE_DEF(ui, 8)
MSH_IMAGE_FREE_DEF(ui, 16)
MSH_IMAGE_FREE_DEF(ui, 32)
MSH_IMAGE_FREE_DEF(f, 32)
MSH_IMAGE_FREE_DEF(f, 64)

// MSH_IMAGE_PIXEL_PTR_DEF(ui, 8)
// MSH_IMAGE_PIXEL_PTR_DEF(ui, 16)
// MSH_IMAGE_PIXEL_PTR_DEF(ui, 32)
// MSH_IMAGE_PIXEL_PTR_DEF(f, 32)
// MSH_IMAGE_PIXEL_PTR_DEF(f, 64)

// MSH_IMAGE_SAMPLE_NN_DEF(ui, 8)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 8, 2)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 8, 3)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 8, 4)
// MSH_IMAGE_SAMPLE_NN_DEF(ui, 16)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 16, 2)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 16, 3)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 16, 4)
// MSH_IMAGE_SAMPLE_NN_DEF(ui, 32)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 32, 2)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 32, 3)
// MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 32, 4)


#ifdef MSH_IMG_PROC_IO
#endif

#endif /*MSH_IMG_PROC_IMPLEMENTATION*/