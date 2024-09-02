// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license

#pragma once

// Tell CImg not to use display capabilities.
#undef cimg_display
#define cimg_display 0
#include "CImg.h"

using namespace cimg_library;

// Useful type not defined in CImg libs
struct rectangle_t {
    int left_px;
    int top_px;
    int right_px;
    int bottom_px;
    int get_center_x_px() { return (right_px + left_px) / 2; }
    int get_center_y_px() { return (bottom_px + top_px) / 2; }
    int get_width_px() { return right_px - left_px; }
    int get_height_px() { return bottom_px - top_px; }
};
