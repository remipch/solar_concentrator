// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license (see software/LICENSE.md)

#pragma once

#include "image.hpp"

#include "esp_camera.h"

// output_buffer must be allocated by the caller
camera_fb_t grayscale_cimg_to_grayscale_frame(CImg<unsigned char> &input, uint8_t *output_buffer);

void rgb565_frame_to_rgb888_cimg(camera_fb_t *input, CImg<unsigned char> &output);

void grayscale_frame_to_grayscale_cimg(camera_fb_t *input, CImg<unsigned char> &output);

void rgb888_cimg_to_grayscale_quirc(CImg<unsigned char> &input, uint8_t *output);

void rgb888_cimg_to_rgb565_frame(CImg<unsigned char> &input, camera_fb_t *output);
