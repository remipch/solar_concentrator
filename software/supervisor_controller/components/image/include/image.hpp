#pragma once

// Tell CImg not to use display capabilities.
#undef cimg_display
#define cimg_display 0
#include "CImg.h"

using namespace cimg_library;

// Useful types not defined by CImg libs
// but are very useful when dealing with images
struct rectangle_t {
    int left_px;
    int top_px;
    int right_px;
    int bottom_px;
};
