#pragma once

#include "image.hpp"

// Define constant image size and format so camera user components can pre-allocate memory
// before camera initialization
#define CAMERA_WIDTH 800
#define CAMERA_HEIGHT 600

void camera_init();

// Take an image, fill the given image,
// which must have been allocated by the caller to size CAMERA_WIDTH,CAMERA_HEIGHT
// if drop_current_image : the currently grabed image will be dropped
// before capturing a new image
// return true if capture is successful
bool camera_capture(bool drop_current_image, CImg<unsigned char> &grayscale_cimg);
