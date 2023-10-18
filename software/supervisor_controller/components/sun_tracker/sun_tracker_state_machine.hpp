#pragma once

#include "image.hpp"
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

enum class sun_tracker_transition_t : signed char {
    NONE = 0,
    START,
    RESET,
};

inline const char* str(sun_tracker_transition_t transition)
{
    switch (transition) {
    case sun_tracker_transition_t::NONE:
        return "NONE";
    case sun_tracker_transition_t::RESET:
        return "RESET";
    case sun_tracker_transition_t::START:
        return "START";
    default:
        assert(false);
    }
}

// This function is not thread safe, the caller has the responsibility to :
// - never call it concurrently
// - cache sun_tracker state to give it (optionaly asynchronously) to external components
// - calling it each time a transition is triggered
// This function updates the state depending on current_state and transition :
// - start meaningful actions (non blocking calls to start actions)
// - return the new state
// This function can start long-time processing functions but must returns quickly
// so important future transitions can be treated quickly (motors manual move can
// start quickly, even if a full image is being captured asynchronously)
//TODO : add must_stop_callback to early quit current state
sun_tracker_state_t sun_tracker_state_machine_update(
    sun_tracker_state_t current_state,
    sun_tracker_transition_t transition,
    sun_tracker_result_callback publish_result,
    sun_tracker_image_callback publish_full_image,
    sun_tracker_image_callback publish_target_image);

