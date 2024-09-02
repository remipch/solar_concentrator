// Copyright (C) 2024 Rémi Peuchot
// This code is distributed under GNU GPL v3 license

#pragma once

#include "image.hpp"
#include "motors_direction.hpp"
#include "sun_tracker_detection_result.hpp"

#include <assert.h>

struct sun_tracker_detection_t {
    sun_tracker_detection_result_t result;
    rectangle_t target_area;
    rectangle_t spot_light; // relative to target_area
    motors_direction_t direction;
};

// Detect target area and spot light rectangle
// detected elements are drawn in the given full_img for debug purpose
sun_tracker_detection_t sun_tracker_logic_detect(CImg<unsigned char> &full_img);
