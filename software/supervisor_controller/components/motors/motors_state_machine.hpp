#pragma once

#include "motors_direction.hpp"

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

enum class motors_transition_t : signed char {
    NONE = 0,
    STOP,
    START_MOVE_CONTINUOUS,
    START_MOVE_ONE_STEP,
};

inline const char* str(motors_transition_t transition)
{
    switch (transition) {
    case motors_transition_t::NONE:
        return "NONE";
    case motors_transition_t::STOP:
        return "STOP";
    case motors_transition_t::START_MOVE_CONTINUOUS:
        return "START_MOVE_CONTINUOUS";
    case motors_transition_t::START_MOVE_ONE_STEP:
        return "START_MOVE_ONE_STEP";
    default:
        assert(false);
    }
}

motors_state_t motors_state_machine_update(
    motors_state_t current_state,
    motors_transition_t transition,
    motors_direction_t motors_direction);

