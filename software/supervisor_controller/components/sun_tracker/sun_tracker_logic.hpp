#pragma once

#include "image.hpp"
#include "motors_direction.hpp"

#include <assert.h>

enum class sun_tracker_logic_result_t : signed char {
    UNKNOWN,
    NO_SPOT,
    SPOT_TOO_SMALL,
    MUST_MOVE,
    TARGET_REACHED,
};

inline const char *str(sun_tracker_logic_result_t state)
{
    switch (state) {
    case sun_tracker_logic_result_t::UNKNOWN:
        return "UNKNOWN";
    case sun_tracker_logic_result_t::NO_SPOT:
        return "NO_SPOT";
    case sun_tracker_logic_result_t::SPOT_TOO_SMALL:
        return "SPOT_TOO_SMALL";
    case sun_tracker_logic_result_t::MUST_MOVE:
        return "MUST_MOVE";
    case sun_tracker_logic_result_t::TARGET_REACHED:
        return "TARGET_REACHED";
    default:
        assert(false);
    }
}

sun_tracker_logic_result_t sun_tracker_logic_get_best_motors_direction(CImg<unsigned char> &target_img,
                                                                       motors_direction_t &motors_direction);
