#pragma once

#include "image.hpp"
#include "motors_direction.hpp"
#include "sun_tracker_logic_result.hpp"

#include <assert.h>

sun_tracker_logic_result_t sun_tracker_logic_start(CImg<unsigned char> &target_img,
                                                   motors_direction_t &motors_direction);

// motors_direction contains the previously returned motors direction
// and should be updated with new motors direction
sun_tracker_logic_result_t sun_tracker_logic_update(CImg<unsigned char> &target_img,
                                                    motors_direction_t &motors_direction);
