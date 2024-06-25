#pragma once

#include "image.hpp"
#include "motors_direction.hpp"
#include "sun_tracker_detection_result.hpp"

#include <assert.h>

struct sun_tracker_detection_t {
    sun_tracker_detection_result_t result;
    rectangle_t target_area;
    rectangle_t spot_light; // relative to target_area
    bool left_border;       // true if spot light is near or on border
    bool top_border;
    bool right_border;
    bool bottom_border;
    motors_direction_t direction;
};

// Detect target area and spot light rectangle
// detected elements are drawn in the given full_img for debug purpose
sun_tracker_detection_t sun_tracker_logic_detect(CImg<unsigned char> &full_img);
