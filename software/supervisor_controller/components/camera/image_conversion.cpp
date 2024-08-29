// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license (see software/LICENSE.md)

#include "image_conversion.hpp"

#include "esp_log.h"

// output_buffer must be allocated by the caller
camera_fb_t grayscale_cimg_to_grayscale_frame(CImg<unsigned char> &input, uint8_t *output_buffer)
{
    assert(input.depth() == 1);
    assert(input.spectrum() == 1);
    assert(output_buffer != NULL);
    camera_fb_t output{.buf = output_buffer,
                       .len = (size_t)(input.width() * input.height()),
                       .width = (size_t)input.width(),
                       .height = (size_t)input.height(),
                       .format = PIXFORMAT_GRAYSCALE,
                       .timestamp = {
                           .tv_sec = 0,
                           .tv_usec = 0,
                       }};
    int i = 0;
    for (int y = 0; y < input.height(); y++) {
        for (int x = 0; x < input.width(); x++) {
            uint8_t gray = *input.data(x, y, 0);
            output.buf[i++] = gray;
        }
    }
    return output;
}

void rgb565_frame_to_rgb888_cimg(camera_fb_t *input, CImg<unsigned char> &output)
{
    assert(input->format == PIXFORMAT_RGB565);
    assert(output.width() == input->width);
    assert(output.height() == input->height);
    assert(output.depth() == 1);
    assert(output.spectrum() == 3);
    int i = 0;
    for (int y = 0; y < input->height; y++) {
        for (int x = 0; x < input->width; x++) {
            // format is deduced from esp-who/components/esp-dl/include/image/dl_image.hpp
            uint16_t rgb565 = ((uint16_t *)input->buf)[i++];
            uint8_t b = (uint8_t)((rgb565 & 0x1F00) >> 5);
            uint8_t g = (uint8_t)(((rgb565 & 0x7) << 5) | ((rgb565 & 0xE000) >> 11));
            uint8_t r = (uint8_t)(rgb565 & 0xF8);
            *output.data(x, y, 0, 0) = r;
            *output.data(x, y, 0, 1) = g;
            *output.data(x, y, 0, 2) = b;
        }
    }
}

void grayscale_frame_to_grayscale_cimg(camera_fb_t *input, CImg<unsigned char> &output)
{
    assert(input->format == PIXFORMAT_GRAYSCALE);
    assert(output.width() == input->width);
    assert(output.height() == input->height);
    assert(output.depth() == 1);
    assert(output.spectrum() == 1);
    // TODO? memcpy (seems to be the same pixel ordering)
    int i = 0;
    for (int y = 0; y < input->height; y++) {
        for (int x = 0; x < input->width; x++) {
            *output.data(x, y, 0) = input->buf[i++];
        }
    }
}

void rgb888_cimg_to_grayscale_quirc(CImg<unsigned char> &input, uint8_t *output)
{
    int i = 0;
    for (int y = 0; y < input.height(); y++) {
        for (int x = 0; x < input.width(); x++) {
            float r = (float)(*input.data(x, y, 0, 0));
            float g = (float)(*input.data(x, y, 0, 1));
            float b = (float)(*input.data(x, y, 0, 2));
            uint8_t gray = (uint8_t)((0.2126 * r) + (0.7152 * g) + (0.0722 * b));
            output[i++] = gray;

            // Temp : change input to grayscale
            *input.data(x, y, 0, 0) = gray;
            *input.data(x, y, 0, 1) = gray;
            *input.data(x, y, 0, 2) = gray;
        }
    }
}

// TODO : idem grayscale_cimg_to_grayscale_frame
void rgb888_cimg_to_rgb565_frame(CImg<unsigned char> &input, camera_fb_t *output)
{
    output->len = 2 * input.width() * input.height();
    output->width = input.width();
    output->height = input.height();
    output->format = PIXFORMAT_GRAYSCALE;
    output->timestamp.tv_sec = 0;
    output->timestamp.tv_usec = 0;

    int i = 0;
    for (int y = 0; y < input.height(); y++) {
        for (int x = 0; x < input.width(); x++) {
            uint8_t r = *input.data(x, y, 0, 0);
            uint16_t g = (uint16_t)(*input.data(x, y, 0, 1));
            uint16_t b = (uint16_t)(*input.data(x, y, 0, 2));
            uint16_t rgb565 = (r & 0xf8) | ((g & 0xe0) >> 5) | ((g & 0x1c) << 11) | ((b & 0xf8) << 5);
            ((uint16_t *)output->buf)[i++] = rgb565;
        }
    }
}
