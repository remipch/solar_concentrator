#pragma once

#include "motors_direction.hpp"

#include <assert.h>

enum class motors_state_t : signed char {
    ERROR = -1,
    UNINITIALIZED = 0,
    STOPPED,
    MOVING,
    STOPPING,
};

inline const char *str(motors_state_t state)
{
    switch (state) {
    case motors_state_t::ERROR:
        return "ERROR";
    case motors_state_t::UNINITIALIZED:
        return "UNINITIALIZED";
    case motors_state_t::STOPPED:
        return "STOPPED";
    case motors_state_t::MOVING:
        return "MOVING";
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

inline const char *str(motors_transition_t transition)
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

// This function is not thread safe, the caller has the responsibility to :
// - never call it concurrently
// - cache state to give it (optionaly asynchronously) to external components
// - calling it each time a transition is triggered
// This function updates the state depending on current_state and transition :
// - start meaningful actions (non blocking calls to start actions)
// - return the new state
// This function can start long-time processing functions but must returns quickly
// so important future transitions can be treated quickly (motors manual move can
// start quickly, even if a full image is being captured asynchronously)
motors_state_t motors_state_machine_update(motors_state_t current_state,
                                           motors_transition_t transition,
                                           motors_direction_t motors_direction);
