#include "image_process.hpp"
#include <cmath>
#include <set>

uint32_t get_pixel(const CImg8 &image, int x, int y) {
    uint32_t rgb = (uint32_t)image(x,y,0,0);
    rgb = (rgb << 8) + (uint32_t)image(x,y,0,1);
    rgb = (rgb << 8) + (uint32_t)image(x,y,0,2);
    return rgb;
}

int get_unique_colors(const CImg8 &image) {
    return get_unique_colors(image, 0, 0, image.width(), image.height());
}

int get_unique_colors(const CImg8 &image, int base_x, int base_y, int width, int height)  {
    std::set<uint32_t> hist;

    for(int y = base_y; y < base_y + height; y++) {
        for(int x = base_x; x < base_x + width; x++) {
           hist.insert(get_pixel(image, x, y));
        }
    }

    return hist.size();
}


int pixel_diff(const CImg8 &image1, const CImg8 &image2, int x, int y) {
    // take difference between rg,b
    uint8_t r_diff = abs(image1(x,y,0,0)-image2(x,y,0,0));
    uint8_t g_diff = abs(image1(x,y,0,1)-image2(x,y,0,1));
    uint8_t b_diff = abs(image1(x,y,0,2)-image2(x,y,0,2));

    return r_diff + g_diff + b_diff;
}


int diff_region(const CImg8 &image1, const CImg8 &image2, int sx, int sy, int width, int height) {
    int sum = 0;
    for(int y = sy; y < sy+height; y++) {
        for(int x = sx; x < sx+width; x++) {
            sum += pixel_diff(image1, image2, x, y);
        }
    }
    return sum;
}

int regioned_diff(const CImg8 &image1, const CImg8 &image2, int width, int height, int hist_limit, int region_match_limit, int diff_limit, int &num_regions_passed, int &num_passed_histogram) {
    int smallest = -1;
 
    num_regions_passed = 0;
    num_passed_histogram = 0;
    int block_x = width / 6;
    int block_y = height / 6;

    for(int base_y = 0; base_y < height; base_y+=block_y) {
        for(int base_x = 0; base_x < width; base_x+=block_x) {
            int diff = diff_region(image1, image2, base_x, base_y, block_x, block_y);
            // make sure the images pass the histogram test
            if(get_unique_colors(image1, base_x, base_y, block_x, block_y) > hist_limit &&
               get_unique_colors(image2, base_x, base_y, block_x, block_y) > hist_limit) {
                num_passed_histogram++;
                if(diff <= diff_limit) {
                    num_regions_passed++;
                }
                if(smallest == -1 || diff < smallest) {
                    smallest = diff;
                }
            }
        }
    }

    if (num_regions_passed < region_match_limit) {
        return -1;
    }

    return smallest;
}
