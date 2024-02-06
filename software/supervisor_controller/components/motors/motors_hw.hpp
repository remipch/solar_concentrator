#pragma once

#include "motors_direction.hpp"

#include <assert.h>

enum class motor_hw_error_t : char {
    NO_ERROR = 0,
    CANNOT_USE_UART
};

enum class motor_hw_state_t : char {
    UNKNOWN = 0,
    STOPPED,
    MOVING
};

// Convenient function for logging
inline const char *str(motor_hw_state_t state)
{
    switch (state) {
    case motor_hw_state_t::UNKNOWN:
        return "UNKNOWN";
    case motor_hw_state_t::STOPPED:
        return "STOPPED";
    case motor_hw_state_t::MOVING:
        return "MOVING";
    default:
        assert(false);
    }
}

motor_hw_error_t motors_hw_init();

void motors_hw_stop();

void motors_hw_start_move(motors_direction_t direction, bool continuous);

motor_hw_state_t motor_hw_get_state();
