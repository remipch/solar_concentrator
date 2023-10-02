#pragma once

#include "motors_types.hpp"

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
inline const char* str(motor_hw_state_t state)
{
    switch (state) {
    case motor_hw_state_t::UNKNOWN:
        return "UNKNOWN";
    case motor_hw_state_t::STOPPED:
        return "STOPPED";
    case motor_hw_state_t::MOVING:
        return "MOVING";
    default:
        abort();
    }
}

motor_hw_error_t motors_hw_init();

void motors_hw_stop();

void motors_hw_move_big_step(motors_direction_t direction);

void motors_hw_move_small_step(motors_direction_t direction);

motor_hw_state_t motor_hw_get_state();
