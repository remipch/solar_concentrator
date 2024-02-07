#pragma once

#include "camera.hpp"
#include "panel.hpp"
#include "sun_tracker_callbacks.hpp"
#include "sun_tracker_detection_result.hpp"

#include <assert.h>

enum class sun_tracker_state_t : signed char {
    UNINITIALIZED,
    IDLE,
    TRACKING,
    STOPPING,
};

inline const char *str(sun_tracker_state_t state)
{
    switch (state) {
    case sun_tracker_state_t::UNINITIALIZED:
        return "UNINITIALIZED";
    case sun_tracker_state_t::IDLE:
        return "IDLE";
    case sun_tracker_state_t::TRACKING:
        return "TRACKING";
    case sun_tracker_state_t::STOPPING:
        return "STOPPING";
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
};

inline const char *str(sun_tracker_transition_t transition)
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
    default:
        return "(multiple values)";
    }
}

// for debug purpose
sun_tracker_detection_result_t sun_tracker_state_machine_get_detection_result();

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
// 'logic_result' output param is only for display purpose
sun_tracker_state_t sun_tracker_state_machine_update(sun_tracker_state_t current_state,
                                                     sun_tracker_transition_t transition,
                                                     panel_t panel,
                                                     sun_tracker_image_callback publish_full_image,
                                                     sun_tracker_result_t &result);
