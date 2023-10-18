#pragma once

#include "motors_direction.hpp"

#include <assert.h>

enum class supervisor_state_t : signed char {
    ERROR = -1,
    UNINITIALIZED = 0,
    IDLE,
    MANUAL_MOVING,
    STARTING_SUN_TRACKING,
    SUN_TRACKING,
    STOPPING_SUN_TRACKING,
    WAITING_SUN_MOVE,
};

inline const char* str(supervisor_state_t state)
{
    switch (state) {
    case supervisor_state_t::ERROR:
        return "ERROR";
    case supervisor_state_t::UNINITIALIZED:
        return "UNINITIALIZED";
    case supervisor_state_t::IDLE:
        return "IDLE";
    case supervisor_state_t::MANUAL_MOVING:
        return "MANUAL_MOVING";
    case supervisor_state_t::STARTING_SUN_TRACKING:
        return "STARTING_SUN_TRACKING";
    case supervisor_state_t::SUN_TRACKING:
        return "SUN_TRACKING";
    case supervisor_state_t::STOPPING_SUN_TRACKING:
        return "STOPPING_SUN_TRACKING";
    case supervisor_state_t::WAITING_SUN_MOVE:
        return "WAITING_SUN_MOVE";
    default:
        assert(false);
    }
}

enum class supervisor_transition_t : signed char {
    NONE = 0,
    STOP,
    START_MANUAL_MOVE_CONTINUOUS,
    START_MANUAL_MOVE_ONE_STEP,
    MOTORS_STOPPED,
    START_SUN_TRACKING,
    SUN_TRACKING_STARTED,
    SUN_TRACKING_FINISHED,
    SUN_TRACKING_ERROR,
};

inline const char* str(supervisor_transition_t transition)
{
    switch (transition) {
    case supervisor_transition_t::NONE:
        return "NONE";
    case supervisor_transition_t::STOP:
        return "STOP";
    case supervisor_transition_t::START_MANUAL_MOVE_CONTINUOUS:
        return "START_MANUAL_MOVE_CONTINUOUS";
    case supervisor_transition_t::START_MANUAL_MOVE_ONE_STEP:
        return "START_MANUAL_MOVE_ONE_STEP";
    case supervisor_transition_t::MOTORS_STOPPED:
        return "MOTORS_STOPPED";
    case supervisor_transition_t::START_SUN_TRACKING:
        return "START_SUN_TRACKING";
    case supervisor_transition_t::SUN_TRACKING_STARTED:
        return "SUN_TRACKING_STARTED";
    case supervisor_transition_t::SUN_TRACKING_FINISHED:
        return "SUN_TRACKING_FINISHED";
    case supervisor_transition_t::SUN_TRACKING_ERROR:
        return "SUN_TRACKING_ERROR";
    default:
        assert(false);
    }
}

// This function is not thread safe, the caller has the responsibility to :
// - never call it concurrently
// - cache supervisor state to give it (optionaly asynchronously) to external components
// - calling it each time a transition is triggered
// This function updates the state depending on current_state and transition :
// - start meaningful actions (non blocking calls to start actions)
// - return the new state
// This function can start long-time processing functions but must returns quickly
// so important future transitions can be treated quickly (motors manual move can
// start quickly, even if a full image is being captured asynchronously)
supervisor_state_t supervisor_state_machine_update(
    supervisor_state_t current_state,
    supervisor_transition_t transition,
    motors_direction_t motors_direction);

