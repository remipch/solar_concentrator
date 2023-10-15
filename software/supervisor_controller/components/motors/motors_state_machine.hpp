#pragma once

#include "motors_types.hpp"

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

void motors_state_machine_init();

motors_state_t motors_state_machine_update(
    motors_state_t current_state,
    motors_transition_t transition,
    motors_direction_t motors_direction);

