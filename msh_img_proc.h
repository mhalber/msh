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

#ifdef MSH_IMG_PROC_INCLUDE_HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#endif

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


typedef unsigned char  mship_ui8_t;
typedef unsigned short mship_ui16_t;
typedef unsigned int   mship_ui32_t;
typedef char           mship_i8_t;
typedef short          mship_i16_t;
typedef int            mship_i32_t;
typedef float          mship_f32_t;
typedef double         mship_f64_t;

#define MSH_PIXEL_STRUCT_DEF(id,b,s)\
typedef struct msh_pixel##s##_##id##b\
{\
  mship_##id##b##_t data[s];\
}msh_pixel##s##_##id##b##_t;\

#define MSH_IMAGE_STRUCT_DEF(id, b) \
typedef struct msh_img_##id##b\
{\
 int width;\
 int height;\
 int n_comp;\
 mship_##id##b##_t *data;\
}msh_img_##id##b##_t;

#define MSH_IMAGE_INIT_DECL(id, b)\
msh_img_##id##b##_t mship_img_##id##b##_init(int w, int h, int n, int init);

#define MSH_IMAGE_COPY_DECL(id, b)\
msh_img_##id##b##_t mship_img_##id##b##_copy(msh_img_##id##b##_t *img);

#define MSH_IMAGE_FREE_DECL(id, b)\
msh_img_##id##b##_t mship_img_##id##b##_free(msh_img_##id##b##_t *img);

#define MSH_IMAGE_SAMPLE_NN_DECL(id, b)\
inline mship_##id##b##_t \
mship_sample_nn_##id##b(const msh_img_##id##b##_t *img, float x, float y);

#define MSH_IMAGE_SAMPLE_NN_PIX_DECL(id, b, s)\
inline msh_pixel##s##_##id##b##_t \
mship_sample##s##_nn_##id##b(const msh_img_##id##b##_t *img, float x, float y);

#define MSH_IMAGE_SAMPLE_BILINEAR_DECL(id, b)\
inline mship_##id##b##_t mship_sample_bl_##id##b(const msh_img_##id##b##_t *img, float x, float y);

#define MSH_IMAGE_PIXEL_PTR_DECL(id,b) \
inline mship_##id##b##_t* mship_pixel_ptr_##id##b(msh_img_##id##b##_t *img, int x, int y);

#define MSH_IMAGE_PIXEL_CPTR_DECL(id,b) \
inline const mship_##id##b##_t* mship_pixel_cptr_##id##b(const msh_img_##id##b##_t *img, int x, int y);


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

MSH_IMAGE_COPY_DECL(ui, 8)
MSH_IMAGE_COPY_DECL(ui, 16)
MSH_IMAGE_COPY_DECL(ui, 32)
MSH_IMAGE_COPY_DECL(f, 32)
MSH_IMAGE_COPY_DECL(f, 64)

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

MSH_IMAGE_PIXEL_CPTR_DECL(ui, 8)
MSH_IMAGE_PIXEL_CPTR_DECL(ui, 16)
MSH_IMAGE_PIXEL_CPTR_DECL(ui, 32)
MSH_IMAGE_PIXEL_CPTR_DECL(f, 32)
MSH_IMAGE_PIXEL_CPTR_DECL(f, 64)

#ifdef MSH_IMG_PROC_IO
msh_imgui8 msh_load_png();
#endif



#ifdef __cplusplus
}
#endif

#endif /*MSH_IMG_PROC_H*/

#ifdef MSH_IMG_PROC_IMPLEMENTATION

#ifndef mship_max 
#define mship_max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef mship_min 
#define mship_min(a, b) ((a) < (b) ? (a) : (b))
#endif

// NOTE(maciej): CPP mode in MSVC is wierd, boy
#if defined(__cplusplus) && defined(_MSC_VER)
#define MSHIP_INIT_CAST(x) x
#else
#define MSHIP_INIT_CAST(x) (x)
#endif

#define MSH_IMAGE__COMPAR(id, b)\
static int \
mship__compar_##id##b(const void* x, const void* y)\
{\
  return (*(mship_##id##b##_t*)x < *(mship_##id##b##_t*)y) ? 1 : -1;\
}

#define MSH_IMAGE__PIXEL2_SCALAR_MUL(id, b)\
static inline \
msh_pixel2_##id##b##_t mship__pixel2_##id##b##_mul_scalar(msh_pixel2_##id##b##_t pix, float s)\
{\
    return MSHIP_INIT_CAST(msh_pixel2_##id##b##_t){{\
                      (mship_##id##b##_t)(pix.data[0]*s),  \
                      (mship_##id##b##_t)(pix.data[1]*s)}};\
}

#define MSH_IMAGE__PIXEL3_SCALAR_MUL(id, b)\
static inline \
msh_pixel3_##id##b##_t mship__pixel3_##id##b##_mul_scalar(msh_pixel3_##id##b##_t pix, float s)\
{\
  return MSHIP_INIT_CAST(msh_pixel3_##id##b##_t){{\
                    (mship_##id##b##_t)(pix.data[0]*s),  \
                    (mship_##id##b##_t)(pix.data[1]*s),  \
                    (mship_##id##b##_t)(pix.data[2]*s)}};\
}

#define MSH_IMAGE__PIXEL4_SCALAR_MUL(id, b)\
static inline \
msh_pixel4_##id##b##_t mship__pixel4_##id##b##_mul_scalar(msh_pixel4_##id##b##_t pix, float s)\
{\
  return MSHIP_INIT_CAST(msh_pixel4_##id##b##_t){{\
                    (mship_##id##b##_t)(pix.data[0]*s), \
                    (mship_##id##b##_t)(pix.data[1]*s), \
                    (mship_##id##b##_t)(pix.data[2]*s), \
                    (mship_##id##b##_t)(pix.data[3]*s)}};\
}

#define MSH_IMAGE__PIXEL2_ADD(id, b)\
static inline \
msh_pixel2_##id##b##_t mship__pixel2_##id##b##_add(msh_pixel2_##id##b##_t pix_a, msh_pixel2_##id##b##_t pix_b)\
{\
  return MSHIP_INIT_CAST(msh_pixel2_##id##b##_t){{\
                    (mship_##id##b##_t)(pix_a.data[0]+pix_b.data[0]),\
                    (mship_##id##b##_t)(pix_a.data[1]+pix_b.data[1])}};\
}

#define MSH_IMAGE__PIXEL3_ADD(id, b)\
static inline \
msh_pixel3_##id##b##_t mship__pixel3_##id##b##_add(msh_pixel3_##id##b##_t pix_a, msh_pixel3_##id##b##_t pix_b)\
{\
  return MSHIP_INIT_CAST(msh_pixel3_##id##b##_t){{\
                    (mship_##id##b##_t)(pix_a.data[0]+pix_b.data[0]),\
                    (mship_##id##b##_t)(pix_a.data[1]+pix_b.data[1]),\
                    (mship_##id##b##_t)(pix_a.data[2]+pix_b.data[2])}};\
}

#define MSH_IMAGE__PIXEL4_ADD(id, b)\
static inline \
msh_pixel4_##id##b##_t mship__pixel4_##id##b##_add(msh_pixel4_##id##b##_t pix_a, msh_pixel4_##id##b##_t pix_b)\
{\
  return MSHIP_INIT_CAST(msh_pixel4_##id##b##_t){{\
                    (mship_##id##b##_t)(pix_a.data[0]+pix_b.data[0]),\
                    (mship_##id##b##_t)(pix_a.data[1]+pix_b.data[1]),\
                    (mship_##id##b##_t)(pix_a.data[2]+pix_b.data[2]),\
                    (mship_##id##b##_t)(pix_a.data[3]+pix_b.data[3])}};\
}


#define MSH_IMAGE_INIT_DEF(id, b)\
MSHIPDEF msh_img_##id##b##_t \
mship_img_##id##b##_init(int width, int height, int n_comp, int initialize)\
{\
  msh_img_##id##b##_t img;\
  int n_elems = width*height*n_comp;\
  int byte_size = n_elems*sizeof(mship_##id##b##_t);\
  img.width  = width;\
  img.height = height;\
  img.n_comp = n_comp;\
  img.data   = NULL;\
  if(initialize) {\
    img.data   = (mship_##id##b##_t*)malloc(byte_size);\
    memset((void*)img.data, 0, byte_size);\
  }\
  return img;\
}


#define MSH_IMAGE_COPY_DEF(id, b)\
MSHIPDEF msh_img_##id##b##_t \
mship_img_##id##b##_copy(msh_img_##id##b##_t *img)\
{\
  msh_img_##id##b##_t cpy;\
  int n_elems = img->width*img->height*img->n_comp;\
  int byte_size = n_elems*sizeof(mship_##id##b##_t);\
  cpy.width = img->width;\
  cpy.height = img->height;\
  cpy.n_comp = img->n_comp;\
  cpy.data = (mship_##id##b##_t*)malloc(byte_size);\
  memcpy(cpy.data, img->data, byte_size);\
  return cpy;\
}

#define MSH_IMAGE_FREE_DEF(id, b)\
MSHIPDEF void \
mship_img_##id##b##_free(msh_img_##id##b##_t* img)\
{\
  if(img->data) free(img->data);\
}

#define MSH_IMAGE_PIXEL_PTR_DEF(id,b) \
MSHIPDEF inline mship_##id##b##_t* \
mship_pixel_ptr_##id##b(msh_img_##id##b##_t *img, int x, int y)\
{\
  return &(img->data[img->n_comp*(img->width*y + x)]);\
}

#define MSH_IMAGE_PIXEL_CPTR_DEF(id,b) \
MSHIPDEF inline const mship_##id##b##_t* \
mship_pixel_cptr_##id##b( const msh_img_##id##b##_t *img, int x, int y)\
{\
  return &(img->data[img->n_comp*(img->width*y + x)]);\
}

#define MSH_IMAGE_SAMPLE_NN_DEF(id, b)\
MSHIPDEF inline mship_##id##b##_t \
mship_sample_nn_##id##b(const msh_img_##id##b##_t *img, float x, float y)\
{\
  return (img->data[(img->width*(int)(floorf(y)) + (int)(floorf(x)))]); \
}

#define MSH_IMAGE_SAMPLE_NN_PIX_DEF(id, b, s)\
MSHIPDEF inline msh_pixel##s##_##id##b##_t \
mship_sample##s##_nn_##id##b(const msh_img_##id##b##_t *img, float x, float y)\
{\
  msh_pixel##s##_##id##b##_t pix;\
  memcpy((&(pix.data[0])),(&(img->data[img->n_comp*(img->width*(int)(floorf(y)) + (int)(floorf(x)))])), (s*b)/8);\
  return pix;\
}

// NOTE(maciej): There might be more efficient version of this that writes to a pixel
#define MSH_IMAGE_SAMPLE_BILINEAR_DEF(id, b)\
MSHIPDEF inline mship_##id##b##_t \
mship_sample_bl_##id##b(const msh_img_##id##b##_t *img, float x, float y)\
{\
  float lx = floorf(x); float rx = mship_min(lx+1, img->width-1); \
  float ly = floorf(y); float ry = mship_min(ly+1, img->height-1); \
  double val1 = (double)mship_sample_nn_##id##b(img, lx, ly) * (1.0+lx-x)*(1.0+ly-y);\
  double val2 = (double)mship_sample_nn_##id##b(img, lx, ry) * (1.0+lx-x)*(y-ly);\
  double val3 = (double)mship_sample_nn_##id##b(img, rx, ly) * (x-lx)*(1.0+ly-y);\
  double val4 = (double)mship_sample_nn_##id##b(img, rx, ry) * (x-lx)*(y-ly);\
  return (mship_##id##b##_t)(val1+val2+val3+val4);\
}

// NOTE(maciej): Decide whether we want to have pixel with one elem
#define MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(id, b, s)\
MSHIPDEF inline msh_pixel##s##_##id##b##_t \
mship_sample##s##_bl_##id##b(const msh_img_##id##b##_t *img, float x, float y)\
{\
  float lx = floorf(x); float rx = mship_min(lx+1, img->width-1); \
  float ly = floorf(y); float ry = mship_min(ly+1, img->height-1); \
  float w1 = (1.0f+lx-x)*(1.0f+ly-y);\
  float w2 = (1.0f+lx-x)*(y-ly);\
  float w3 = (x-lx)*(1.0f+ly-y);\
  float w4 = (x-lx)*(y-ly);\
  msh_pixel##s##_##id##b##_t pix1 = mship_sample##s##_nn_##id##b(img, lx, ly);\
  msh_pixel##s##_##id##b##_t pix2 = mship_sample##s##_nn_##id##b(img, lx, ry);\
  msh_pixel##s##_##id##b##_t pix3 = mship_sample##s##_nn_##id##b(img, rx, ly);\
  msh_pixel##s##_##id##b##_t pix4 = mship_sample##s##_nn_##id##b(img, rx, ry);\
  pix1 = mship__pixel##s##_##id##b##_mul_scalar(pix1, w1);\
  pix2 = mship__pixel##s##_##id##b##_mul_scalar(pix2, w2);\
  pix3 = mship__pixel##s##_##id##b##_mul_scalar(pix3, w3);\
  pix4 = mship__pixel##s##_##id##b##_mul_scalar(pix4, w4);\
  msh_pixel##s##_##id##b##_t pixa = mship__pixel##s##_##id##b##_add(pix1, pix2);\
  msh_pixel##s##_##id##b##_t pixb = mship__pixel##s##_##id##b##_add(pix3, pix4);\
  return (mship__pixel##s##_##id##b##_add(pixa, pixb));\
}

MSH_IMAGE__COMPAR(ui, 8)
MSH_IMAGE__COMPAR(ui, 16)
MSH_IMAGE__COMPAR(ui, 32)
MSH_IMAGE__COMPAR(f, 32)
MSH_IMAGE__COMPAR(f, 64)

MSH_IMAGE__PIXEL2_SCALAR_MUL(ui, 8)
MSH_IMAGE__PIXEL2_SCALAR_MUL(ui, 16)
MSH_IMAGE__PIXEL2_SCALAR_MUL(ui, 32)
MSH_IMAGE__PIXEL2_SCALAR_MUL(f, 32)
MSH_IMAGE__PIXEL2_SCALAR_MUL(f, 64)

MSH_IMAGE__PIXEL3_SCALAR_MUL(ui, 8)
MSH_IMAGE__PIXEL3_SCALAR_MUL(ui, 16)
MSH_IMAGE__PIXEL3_SCALAR_MUL(ui, 32)
MSH_IMAGE__PIXEL3_SCALAR_MUL(f, 32)
MSH_IMAGE__PIXEL3_SCALAR_MUL(f, 64)

MSH_IMAGE__PIXEL4_SCALAR_MUL(ui, 8)
MSH_IMAGE__PIXEL4_SCALAR_MUL(ui, 16)
MSH_IMAGE__PIXEL4_SCALAR_MUL(ui, 32)
MSH_IMAGE__PIXEL4_SCALAR_MUL(f, 32)
MSH_IMAGE__PIXEL4_SCALAR_MUL(f, 64)

MSH_IMAGE__PIXEL2_ADD(ui, 8)
MSH_IMAGE__PIXEL2_ADD(ui, 16)
MSH_IMAGE__PIXEL2_ADD(ui, 32)
MSH_IMAGE__PIXEL2_ADD(f, 32)
MSH_IMAGE__PIXEL2_ADD(f, 64)

MSH_IMAGE__PIXEL3_ADD(ui, 8)
MSH_IMAGE__PIXEL3_ADD(ui, 16)
MSH_IMAGE__PIXEL3_ADD(ui, 32)
MSH_IMAGE__PIXEL3_ADD(f, 32)
MSH_IMAGE__PIXEL3_ADD(f, 64)

MSH_IMAGE__PIXEL4_ADD(ui, 8)
MSH_IMAGE__PIXEL4_ADD(ui, 16)
MSH_IMAGE__PIXEL4_ADD(ui, 32)
MSH_IMAGE__PIXEL4_ADD(f, 32)
MSH_IMAGE__PIXEL4_ADD(f, 64)

MSH_IMAGE_INIT_DEF(ui, 8)
MSH_IMAGE_INIT_DEF(ui, 16)
MSH_IMAGE_INIT_DEF(ui, 32)
MSH_IMAGE_INIT_DEF(f, 32)
MSH_IMAGE_INIT_DEF(f, 64)

MSH_IMAGE_COPY_DEF(ui, 8)
MSH_IMAGE_COPY_DEF(ui, 16)
MSH_IMAGE_COPY_DEF(ui, 32)
MSH_IMAGE_COPY_DEF(f, 32)
MSH_IMAGE_COPY_DEF(f, 64)

MSH_IMAGE_FREE_DEF(ui, 8)
MSH_IMAGE_FREE_DEF(ui, 16)
MSH_IMAGE_FREE_DEF(ui, 32)
MSH_IMAGE_FREE_DEF(f, 32)
MSH_IMAGE_FREE_DEF(f, 64)

MSH_IMAGE_PIXEL_PTR_DEF(ui, 8)
MSH_IMAGE_PIXEL_PTR_DEF(ui, 16)
MSH_IMAGE_PIXEL_PTR_DEF(ui, 32)
MSH_IMAGE_PIXEL_PTR_DEF(f, 32)
MSH_IMAGE_PIXEL_PTR_DEF(f, 64)

MSH_IMAGE_PIXEL_CPTR_DEF(ui, 8)
MSH_IMAGE_PIXEL_CPTR_DEF(ui, 16)
MSH_IMAGE_PIXEL_CPTR_DEF(ui, 32)
MSH_IMAGE_PIXEL_CPTR_DEF(f, 32)
MSH_IMAGE_PIXEL_CPTR_DEF(f, 64)

MSH_IMAGE_SAMPLE_NN_DEF(ui, 8)
MSH_IMAGE_SAMPLE_NN_DEF(ui, 16)
MSH_IMAGE_SAMPLE_NN_DEF(ui, 32)
MSH_IMAGE_SAMPLE_NN_DEF(f, 32)
MSH_IMAGE_SAMPLE_NN_DEF(f, 64)

MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 8, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 16, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 32, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(f, 32, 2)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(f, 64, 2)

MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 8, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 16, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 32, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(f, 32, 3)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(f, 64, 3)

MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 8, 4)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 16, 4)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(ui, 32, 4)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(f, 32, 4)
MSH_IMAGE_SAMPLE_NN_PIX_DEF(f, 64, 4)

MSH_IMAGE_SAMPLE_BILINEAR_DEF(ui, 8)
MSH_IMAGE_SAMPLE_BILINEAR_DEF(ui, 16)
MSH_IMAGE_SAMPLE_BILINEAR_DEF(ui, 32)
MSH_IMAGE_SAMPLE_BILINEAR_DEF(f, 32)
MSH_IMAGE_SAMPLE_BILINEAR_DEF(f, 64)

MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 8, 2)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 16, 2)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 32, 2)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(f,  32, 2)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(f,  64, 2)

MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 8, 3)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 16, 3)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 32, 3)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(f,  32, 3)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(f,  64, 3)

MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 8, 4)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 16, 4)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(ui, 32, 4)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(f,  32, 4)
MSH_IMAGE_SAMPLE_BILINEAR_PIX_DEF(f,  64, 4)

// TODO(maciej): Test and make generic
msh_img_ui8_t
mship_median_filter(msh_img_ui8_t* img, int filter_size )
{
  int w = img->width;
  int h = img->height;
  filter_size = (filter_size%2==0) ? filter_size+1 : filter_size;
  int r = (filter_size-1)/2;
  mship_ui8_t filter_data[1024];
  msh_img_ui8_t filtered= mship_img_ui8_copy(img);
  int step = filter_size*filter_size;
  for(int y=10; y<h; ++y)
  {
    for(int x=10; x<w; ++x)
    {
      int filter_idx = 0;
      for(int oy = -r; oy<=r; ++oy)
      {
        for(int ox = -r; ox<=r; ++ox)
        {
          int cx = mship_max(0, mship_min(x+ox, w-1));
          int cy = mship_max(0, mship_min(y+oy, h-1));
          mship_ui8_t* pix_ptr = mship_pixel_ptr_ui8(img, cx, cy);
          for(int c=0 ; c<img->n_comp; ++c)
          {
            filter_data[step*c + filter_idx] = pix_ptr[c];
          }
          filter_idx++;
        }
      }
      mship_ui8_t* filtered_ptr = mship_pixel_ptr_ui8(&filtered, x, y);
      for(int c=0 ; c<img->n_comp; ++c)
      {
        qsort( &(filter_data[c*step]), step, sizeof(mship_ui8_t), mship__compar_ui8);
        filtered_ptr[c] = filter_data[c*step + (step+1)/2];
      }
    }
  }
  return filtered;
}

msh_img_ui8_t
mship_erode_filter(msh_img_ui8_t* img, int filter_size )
{
  int w = img->width;
  int h = img->height;
  filter_size = (filter_size%2==0) ? filter_size+1 : filter_size;
  int r = (filter_size-1)/2;
  msh_img_ui8_t filtered= mship_img_ui8_copy(img);
  for(int y=0; y<h; ++y)
  {
    for(int x=0; x<w; ++x)
    {
      mship_ui8_t* filtered_ptr = mship_pixel_ptr_ui8(&filtered, x, y);
      for(int oy = -r; oy<=r; ++oy)
      {
        for(int ox = -r; ox<=r; ++ox)
        {
          int cx = mship_max(0, mship_min(x+ox, w-1));
          int cy = mship_max(0, mship_min(y+oy, h-1));
          mship_ui8_t* pix_ptr = mship_pixel_ptr_ui8(img, cx, cy);
          for(int c=0 ; c<img->n_comp; ++c)
          {
            if(pix_ptr[c] < filtered_ptr[c]) filtered_ptr[c] = pix_ptr[c];
          }
        }
      }
    }
  }
  return filtered;
}

msh_img_ui8_t
mship_dilate_filter(msh_img_ui8_t* img, int filter_size )
{
  int w = img->width;
  int h = img->height;
  filter_size = (filter_size%2==0) ? filter_size+1 : filter_size;
  int r = (filter_size-1)/2;
  msh_img_ui8_t filtered= mship_img_ui8_copy(img);
  for(int y=0; y<h; ++y)
  {
    for(int x=0; x<w; ++x)
    {
      mship_ui8_t* filtered_ptr = mship_pixel_ptr_ui8(&filtered, x, y);
      for(int oy = -r; oy<=r; ++oy)
      {
        for(int ox = -r; ox<=r; ++ox)
        {
          int cx = mship_max(0, mship_min(x+ox, w-1));
          int cy = mship_max(0, mship_min(y+oy, h-1));
          mship_ui8_t* pix_ptr = mship_pixel_ptr_ui8(img, cx, cy);
          for(int c=0 ; c<img->n_comp; ++c)
          {
            if(pix_ptr[c] > filtered_ptr[c]) filtered_ptr[c] = pix_ptr[c];
          }
        }
      }
    }
  }
  return filtered;
}

#ifdef MSH_IMG_PROC_IO
#endif

#endif /*MSH_IMG_PROC_IMPLEMENTATION*/