#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_camera.h"

#include "image.hpp"

// Define constant image size and format so camera user components can pre-allocate memory
// before camera initialization
#define CAMERA_PIXEL_FORMAT PIXFORMAT_GRAYSCALE
#define CAMERA_FRAMESIZE FRAMESIZE_SVGA
#define CAMERA_WIDTH 800
#define CAMERA_HEIGHT 600

void camera_init();

// Take an image, fill the given image,
// which must have been allocated by the caller to size CAMERA_WIDTH,CAMERA_HEIGHT
// if drop_current_image : the currently grabed image will be dropped
// before captuing a new image
// return true if capture is successful
bool camera_capture(bool drop_current_image, CImg<unsigned char>& grayscale_cimg);

// output_buffer must be allocated by the caller
camera_fb_t grayscale_cimg_to_grayscale_frame(CImg<unsigned char>& input, uint8_t *output_buffer);

void rgb888_cimg_to_rgb565_frame(CImg<unsigned char>& input, camera_fb_t *output);

