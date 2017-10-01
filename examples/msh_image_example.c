#define MSH_IMG_PROC_IMPLEMENTATION
#define STBI_ONLY_JPG
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TT_IMPLEMENTATION
#include <stdio.h>
#include "msh_img_proc.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "tt/tiny_time.h"

void sampling_test();

int main()
{
  sampling_test();

  float resize_factor = 2.0;
  const unsigned char* image_name = "data/seal.jpg";
  msh_img_ui8_t original_img;
  original_img.data = stbi_load(image_name, 
                                &original_img.width, &original_img.height, 
                                &original_img.n_comp, 3);
  printf("Read image %s of the size: %dx%dx%d\n", image_name, 
                  original_img.width, original_img.height, original_img.n_comp);

  msh_img_ui8_t resized_img = mship_img_ui8_init(
                                        (int)(original_img.width*resize_factor),
                                        (int)(original_img.height*resize_factor),
                                        original_img.n_comp, 0);
  ttTime();
  for(int y = 0; y < resized_img.height; ++y)
  {
    for(int x = 0; x < resized_img.width; ++x)
    {
      float ox = x / resize_factor;
      float oy = y / resize_factor;
      msh_pixel3_ui8_t opix = mship_sample3_bl_ui8(&original_img, ox, oy);
      unsigned char* rpix = mship_pixel_ptr_ui8(&resized_img, x, y);
      rpix[0] = opix.data[0];
      rpix[1] = opix.data[1];
      rpix[2] = opix.data[2];
    }
  }
  printf("Bilinear resizing took %f s\n", ttTime());
  stbi_write_png("data/seal_resized_bl.png", resized_img.width, resized_img.height, 
                                             resized_img.n_comp, resized_img.data, 0);
  ttTime();
  for(int y = 0; y < resized_img.height; ++y)
  {
    for(int x = 0; x < resized_img.width; ++x)
    {
      float ox = x / resize_factor;
      float oy = y / resize_factor;
      msh_pixel3_ui8_t opix = mship_sample3_nn_ui8(&original_img, ox, oy);
      unsigned char* rpix = mship_pixel_ptr_ui8(&resized_img, x, y);
      rpix[0] = opix.data[0];
      rpix[1] = opix.data[1];
      rpix[2] = opix.data[2];
    }
  }
  printf("Nearest_neighbor 2 resizing took %f s\n", ttTime());
  stbi_write_png("data/seal_resized_nn1.png", resized_img.width, resized_img.height, 
                                              resized_img.n_comp, resized_img.data, 0);
  ttTime();
  for(int y = 0; y < resized_img.height; ++y)
  {
    for(int x = 0; x < resized_img.width; ++x)
    {
      int ox = (int)floorf(x / resize_factor);
      int oy = (int)floorf(y / resize_factor);
      unsigned char* opix = mship_pixel_ptr_ui8(&original_img, ox, oy);
      unsigned char* rpix = mship_pixel_ptr_ui8(&resized_img, x, y);
      rpix[0] = opix[0];
      rpix[1] = opix[1];
      rpix[2] = opix[2];
    }
  }
  printf("Nearest_neighbor 2 resizing took %f s\n", ttTime());
  stbi_write_png("data/seal_resized_nn2.png", resized_img.width, resized_img.height, 
                                             resized_img.n_comp, resized_img.data, 0);
  // TODO(maciej)
                                             
}

void sampling_test()
{
  msh_img_ui8_t img1_ui8 = mship_img_ui8_init(3, 3, 1, 1);
  msh_img_ui8_t img2_ui8 = mship_img_ui8_init(3, 3, 3, 1);
  msh_img_f32_t img1_f32 = mship_img_f32_init(3, 3, 1, 1);
  msh_img_f32_t img2_f32 = mship_img_f32_init(3, 3, 4, 1);

  unsigned char* pix_ptr1 = mship_pixel_ptr_ui8(&img1_ui8, 1, 1);
  pix_ptr1[0] = 16;
  unsigned char pix1 = mship_sample_nn_ui8(&img1_ui8, 1, 1);
  printf("Single channel unsigned byte image; nn sample: 16 == %2d (%s)\n", 
                                               pix1, (pix1==16)?"True":"False");
  unsigned char pix2 = mship_sample_bl_ui8(&img1_ui8, 1.0, 1.0);
  unsigned char pix3 = mship_sample_bl_ui8(&img1_ui8, 1.5, 1.5);
  printf("Single channel unsigned byte image; bl sample: 16 == %2d (%s)\n", 
                                               pix2, (pix2==16)?"True":"False");
  printf("Single channel unsigned byte image; bl sample:  4 == %2d (%s)\n", 
                                                pix3, (pix3==4)?"True":"False");
  
  unsigned char* pix_ptr2 = mship_pixel_ptr_ui8(&img2_ui8, 1, 1);
  pix_ptr2[0] = 10;
  pix_ptr2[1] = 12;
  pix_ptr2[2] = 14;

  msh_pixel3_ui8_t pix4 = mship_sample3_nn_ui8(&img2_ui8, 1, 1);
  printf(" Three channel unsigned byte image; nn sample: "
         "(10, 12, 14) == (%2d, %2d, %2d) (%s)\n", 
          pix4.data[0], pix4.data[1], pix4.data[2],
          (pix4.data[0] == 10 && pix4.data[1] == 12 
                              && pix4.data[2] == 14) ? "True": "False");
  msh_pixel3_ui8_t pix5 = mship_sample3_bl_ui8(&img2_ui8, 1.0, 1.0);
  msh_pixel3_ui8_t pix6 = mship_sample3_bl_ui8(&img2_ui8, 1.5, 1.5);
  printf(" Three channel unsigned byte image; bl sample: "
         "(10, 12, 14) == (%2d, %2d, %2d) (%s)\n", 
          pix5.data[0], pix5.data[1], pix5.data[2],
          (pix5.data[0] == 10 && pix5.data[1] == 12 
                              && pix5.data[2] == 14) ? "True": "False");
  printf(" Three channel unsigned byte image; bl sample: "
         "( 2,  3,  3) == (%2d, %2d, %2d) (%s)\n", 
          pix6.data[0], pix6.data[1], pix6.data[2],
          (pix6.data[0] == 2 && pix6.data[1] == 3 
                             && pix6.data[2] == 3) ? "True": "False");
  
  float* pix_ptr3 = mship_pixel_ptr_f32(&img1_f32, 1, 1);
  pix_ptr3[0] = 1600.0;
  float pix7 = mship_sample_nn_f32(&img1_f32, 1, 1);
  printf("Single channel float image; nn sample: "
         "1600.0 == %6.2f (%s)\n", pix7, (pix7==1600.0)?"True":"False");
  float pix8 = mship_sample_bl_f32(&img1_f32, 1.0, 1.0);
  float pix9 = mship_sample_bl_f32(&img1_f32, 1.5, 1.5);
  printf("Single channel float image; bl sample:"
         " 1600.0 == %6.2f (%s)\n", pix8, (pix8==1600.0)?"True":"False");
  printf("Single channel float image; bl sample:"
         "  400.0 == %6.2f (%s)\n", pix9, (pix9==400.0)?"True":"False");
  

  float* pix_ptr4 = mship_pixel_ptr_f32(&img2_f32, 1, 1);
  pix_ptr4[0] = 1000.0;
  pix_ptr4[1] = 1200.0;
  pix_ptr4[2] = 1400.0;
  pix_ptr4[3] = 1600.0;
  msh_pixel4_f32_t pix10 = mship_sample4_nn_f32(&img2_f32, 1, 1);
  printf(" Four channel float image; nn sample: (1000, 1200, 1400, 1600) == (%6.2f, %6.2f, %6.2f, %6.2f) (%s)\n", 
          pix10.data[0], pix10.data[1], pix10.data[2], pix10.data[3],
          (pix10.data[0] == 1000.0 && pix10.data[1] == 1200.0 && 
           pix10.data[2] == 1400.0 && pix10.data[3] == 1600.0) ? "True": "False");
  msh_pixel4_f32_t pix11 = mship_sample4_bl_f32(&img2_f32, 1.0, 1.0);
  msh_pixel4_f32_t pix12 = mship_sample4_bl_f32(&img2_f32, 1.5, 1.5);
  printf(" Four channel float image; bl sample: (1000, 1200, 1400, 1600) == (%6.2f, %6.2f, %6.2f, %6.2f) (%s)\n", 
          pix11.data[0], pix11.data[1], pix11.data[2], pix11.data[3],
          (pix11.data[0] == 1000.0 && pix11.data[1] == 1200.0 && 
           pix11.data[2] == 1400.0 && pix11.data[3] == 1600.0) ? "True": "False");
  printf(" Four channel float image; bl sample: (250, 300, 350, 400) == (%6.2f, %6.2f, %6.2f, %6.2f) (%s)\n", 
           pix12.data[0], pix12.data[1], pix12.data[2], pix12.data[3],
           (pix12.data[0] == 250.0 && pix12.data[1] == 300.0 && 
            pix12.data[2] == 350.0 && pix12.data[3] == 400.0) ? "True": "False");
  
  float* pix_ptr5 = mship_pixel_ptr_f32(&img2_f32, 2, 2);
  pix_ptr5[0] = 2000.0;
  pix_ptr5[1] = 2500.0;
  pix_ptr5[2] = 3000.0;
  pix_ptr5[3] = 3500.0;
  msh_pixel4_f32_t pix13 = mship_sample4_nn_f32(&img2_f32, 1, 1);
  printf(" Four channel float image; nn sample: (1000, 1200, 1400, 1600) == (%6.2f, %6.2f, %6.2f, %6.2f) (%s)\n", 
          pix13.data[0], pix13.data[1], pix13.data[2], pix13.data[3],
          (pix13.data[0] == 1000.0 && pix13.data[1] == 1200.0 && 
           pix13.data[2] == 1400.0 && pix13.data[3] == 1600.0) ? "True": "False");
  msh_pixel4_f32_t pix14 = mship_sample4_bl_f32(&img2_f32, 1.0, 1.0);
  msh_pixel4_f32_t pix15 = mship_sample4_bl_f32(&img2_f32, 1.5, 1.5);
  printf(" Four channel float image; bl sample: (1000, 1200, 1400, 1600) == (%6.2f, %6.2f, %6.2f, %6.2f) (%s)\n", 
          pix14.data[0], pix14.data[1], pix14.data[2], pix14.data[3],
          (pix14.data[0] == 1000.0 && pix14.data[1] == 1200.0 && 
           pix14.data[2] == 1400.0 && pix14.data[3] == 1600.0) ? "True": "False");
  printf(" Four channel float image; bl sample: (750, 925, 1100, 1275) == (%6.2f, %6.2f, %6.2f, %6.2f) (%s)\n", 
           pix15.data[0], pix15.data[1], pix15.data[2], pix15.data[3],
           (pix15.data[0] == 750.0 && pix15.data[1] == 925.0 && 
            pix15.data[2] == 1100.0 && pix15.data[3] == 1275.0) ? "True": "False");
}