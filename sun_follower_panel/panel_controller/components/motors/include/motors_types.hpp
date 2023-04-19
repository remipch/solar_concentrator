#pragma once

#include <cstdlib>

// 6 possible directions in clockwise order
enum class motors_direction_t : char {
    NONE = 0,
    UP_RIGHT,
    RIGHT,
    DOWN_RIGHT,
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
    case motors_direction_t::UP_RIGHT:
        return "UP_RIGHT";
    case motors_direction_t::RIGHT:
        return "RIGHT";
    case motors_direction_t::DOWN_RIGHT:
        return "DOWN_RIGHT";
    case motors_direction_t::DOWN_LEFT:
        return "DOWN_LEFT";
    case motors_direction_t::LEFT:
        return "LEFT";
    case motors_direction_t::UP_LEFT:
        return "UP_LEFT";
    default:
        abort();
    }
}

enum class motors_state_t : signed char {
    ERROR = -1,
    UNINITIALIZED = 0,
    IDLE,
    MOVING,
    MOVING_ONE_STEP,
    TIGHTENING,
    LOCKED,
};

inline const char* str(motors_state_t state)
{
    switch (state) {
    case motors_state_t::ERROR:
        return "ERROR";
    case motors_state_t::UNINITIALIZED:
        return "UNINITIALIZED";
    case motors_state_t::IDLE:
        return "IDLE";
    case motors_state_t::MOVING:
        return "MOVING";
    case motors_state_t::MOVING_ONE_STEP:
        return "MOVING_ONE_STEP";
    case motors_state_t::TIGHTENING:
        return "TIGHTENING";
    case motors_state_t::LOCKED:
        return "LOCKED";
    default:
        abort();
    }
}

enum class motors_current_t : char {
    UNKNWON = 0,
    LOW,
    HIGH,
};

inline const char* str(motors_current_t current)
{
    switch (current) {
    case motors_current_t::UNKNWON:
        return "UNKNWON";
    case motors_current_t::LOW:
        return "LOW";
    case motors_current_t::HIGH:
        return "HIGH";
    default:
        abort();
    }
}

struct motors_full_status_t {
    motors_state_t state;
    motors_direction_t direction;
    motors_current_t current;
};

