#ifndef IMAGE_PROCESS_HPP
#define IMAGE_PROCESS_HPP

#include <CImg.h>

using namespace cimg_library;

typedef CImg<uint8_t> CImg8;

uint32_t get_pixel(const CImg8 &image, int x, int y);
int get_unique_colors(const CImg8 &image);
int pixel_diff(const CImg8 &image1, const CImg8 &image2, int x, int y);
int diff_region(const CImg8 &image1, const CImg8 &image2, int sx, int sy, int width, int height);
int regioned_diff(const CImg8 &image1, const CImg8 &image2, int width, int height, int hist_limit);

#endif
