#define MSH_IMG_PROC_IMPLEMENTATION
#include <stdio.h>
#include "msh_img_proc.h"

int main()
{
  printf("Hello World!");
  msh_pixel2_ui8_t pix2 = {125, 255};
  msh_pixel3_ui8_t pix3 = {125, 255, 231};
  msh_pixel4_ui8_t pix4 = {125, 255, 231, 255};

  msh_img_ui8_t img1 = {.width=320, .height=240, .n_comp=3, .data=NULL};
  msh_img_ui8_t img2 = mship_img_ui8_init(320, 240, 3, 1);

  // msh_img_ui8_t pix = 
}