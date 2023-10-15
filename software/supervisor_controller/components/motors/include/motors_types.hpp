#pragma once

#include <cstdlib>

// 6 possible directions in clockwise order
enum class motors_direction_t : char {
    NONE = 0,
    UP,
    UP_RIGHT,
    RIGHT,
    DOWN_RIGHT,
    DOWN,
    DOWN_LEFT,
    LEFT,
    UP_LEFT,
};

// Convenient function for logging
inline const char* str(motors_direction_t direction)
{
    switch (direction) {
    case motors_direction_t::NONE:
        return "NONE";
    case motors_direction_t::UP:
        return "UP";
    case motors_direction_t::UP_RIGHT:
        return "UP_RIGHT";
    case motors_direction_t::RIGHT:
        return "RIGHT";
    case motors_direction_t::DOWN_RIGHT:
        return "DOWN_RIGHT";
    case motors_direction_t::DOWN:
        return "DOWN";
    case motors_direction_t::DOWN_LEFT:
        return "DOWN_LEFT";
    case motors_direction_t::LEFT:
        return "LEFT";
    case motors_direction_t::UP_LEFT:
        return "UP_LEFT";
    default:
        assert(false);
    }
}

enum class motors_state_t : signed char {
    ERROR = -1,
    UNINITIALIZED = 0,
    STOPPED,
    MOVING_CONTINUOUS,
    MOVING_ONE_STEP,
    STOPPING,
};

inline const char* str(motors_state_t state)
{
    switch (state) {
    case motors_state_t::ERROR:
        return "ERROR";
    case motors_state_t::UNINITIALIZED:
        return "UNINITIALIZED";
    case motors_state_t::STOPPED:
        return "STOPPED";
    case motors_state_t::MOVING_CONTINUOUS:
        return "MOVING_CONTINUOUS";
    case motors_state_t::MOVING_ONE_STEP:
        return "MOVING_ONE_STEP";
    case motors_state_t::STOPPING:
        return "STOPPING";
    default:
        assert(false);
    }
}
