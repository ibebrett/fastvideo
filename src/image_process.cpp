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
    std::set<uint32_t> hist;

    int width = image.width();
    int height = image.height();

    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
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
    for(int x = sx; x < sx+width; x++) {
        for(int y = sy; y < sy+height; y++) {
            sum += pixel_diff(image1, image2, x, y);
        }
    }
    return sum;
}

int regioned_diff(const CImg8 &image1, const CImg8 &image2, int width, int height, int hist_limit) {
    int smallest = -1;
 
    int block_x = width / 6;
    int block_y = height / 6;

    for(int base_y = 0; base_y < height; base_y+=block_y) {
        for(int base_x = 0; base_x < width; base_x+=block_x) {
            int diff = diff_region(image1, image2, base_x, base_y, block_x, block_y);
            if(smallest == -1 || diff < smallest) {
                // make sure the images pass the histogram test
                if(get_unique_colors(image1) > hist_limit && get_unique_colors(image2) > hist_limit)
                    smallest = diff;
            }
        }
    }

    return smallest;
}
