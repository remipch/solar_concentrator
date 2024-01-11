#pragma once

#include "camera.hpp"
#include "sun_tracker_callbacks.hpp"

#include <assert.h>

enum class sun_tracker_state_t : signed char {
    ERROR = -1,
    UNINITIALIZED = 0,
    IDLE,
    TRACKING,
    MOVING_ONE_STEP,
    SUCCESS,
};

inline const char* str(sun_tracker_state_t state)
{
    switch (state) {
    case sun_tracker_state_t::ERROR:
        return "ERROR";
    case sun_tracker_state_t::UNINITIALIZED:
        return "UNINITIALIZED";
    case sun_tracker_state_t::IDLE:
        return "IDLE";
    case sun_tracker_state_t::TRACKING:
        return "TRACKING";
    case sun_tracker_state_t::MOVING_ONE_STEP:
        return "MOVING_ONE_STEP";
    case sun_tracker_state_t::SUCCESS:
        return "SUCCESS";
    default:
        assert(false);
    }
}

// Transitions are declared as bitfield to allow several different transitions
// to be triggered while the state machine is processing the current state
// It's usefull for sun_tracker because :
// - its state execution can have long execution time (grab frame and do image processing)
// - we must properly manage the case where 'STOP' and 'MOTORS_STOPPED' transitions
// are triggered during the same single call of state_machine_update
// because neither of these transition can be ignored
enum sun_tracker_transition_t {
    NONE = 0,
    START = 1,
    STOP = 2,
    MOTORS_STOPPED = 4,
    RESET = 8,
};

inline const char* str(sun_tracker_transition_t transition)
{
    switch (transition) {
    case sun_tracker_transition_t::NONE:
        return "NONE";
    case sun_tracker_transition_t::START:
        return "START";
    case sun_tracker_transition_t::STOP:
        return "STOP";
    case sun_tracker_transition_t::MOTORS_STOPPED:
        return "MOTORS_STOPPED";
    case sun_tracker_transition_t::RESET:
        return "RESET";
    default:
        return "(multiple values)";
    }
}

// This function is not thread safe, the caller has the responsibility to :
// - never call it concurrently
// - cache sun_tracker state to give it (optionaly asynchronously) to external components
// - calling it each time a transition is triggered
// This function updates the state depending on current_state and transition :
// - start meaningful actions (non blocking calls to start actions)
// - return the new state
// This function can execute long-time processing functions,
// the caller has the responsibility to run it in a separated task
// and listen to external events and cache them asynchronously
sun_tracker_state_t sun_tracker_state_machine_update(sun_tracker_state_t current_state,
                                                     sun_tracker_transition_t transition,
                                                     sun_tracker_image_callback publish_full_image,
                                                     sun_tracker_image_callback publish_target_image);
